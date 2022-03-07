// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"

#include "DelegationConfig.hpp"

#include "RegistrySerialization.hpp"

#include <wil/resource.h>
#include <wil/winrt.h>
#include <windows.foundation.collections.h>
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.AppExtensions.h>

#include "../inc/conint.h"

#include <initguid.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::ApplicationModel;
using namespace ABI::Windows::ApplicationModel::AppExtensions;

#pragma hdrstop

#define DELEGATION_CONSOLE_KEY_NAME L"DelegationConsole"
#define DELEGATION_TERMINAL_KEY_NAME L"DelegationTerminal"

DEFINE_GUID(CLSID_SystemDelegationConsole, 0x2eaca947, 0x7f5f, 0x4cfa, 0xba, 0x87, 0x8f, 0x7f, 0xbe, 0xef, 0xbe, 0x69);
DEFINE_GUID(CLSID_SystemDelegationTerminal, 0xe12cff52, 0xa866, 0x4c77, 0x9a, 0x90, 0xf5, 0x70, 0xa7, 0xaa, 0x2c, 0x6b);

#define DELEGATION_CONSOLE_EXTENSION_NAME L"com.microsoft.windows.console.host"
#define DELEGATION_TERMINAL_EXTENSION_NAME L"com.microsoft.windows.terminal.host"

template<typename T, typename std::enable_if<std::is_base_of<DelegationConfig::DelegationBase, T>::value>::type* = nullptr>
HRESULT _lookupCatalog(PCWSTR extensionName, std::vector<T>& vec) noexcept
{
    vec.clear();

    T useInbox = { 0 };
    useInbox.clsid = { 0 };
    // CLSID of 0 will be sentinel to say "inbox console" or something.
    // The UI displaying this information will have to go look up appropriate strings
    // to convey that message.

    vec.push_back(useInbox);

    auto coinit = wil::CoInitializeEx(COINIT_APARTMENTTHREADED);

    ComPtr<IAppExtensionCatalogStatics> catalogStatics;
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_AppExtensions_AppExtensionCatalog).Get(), &catalogStatics));

    ComPtr<IAppExtensionCatalog> catalog;
    RETURN_IF_FAILED(catalogStatics->Open(HStringReference(extensionName).Get(), &catalog));

    ComPtr<IAsyncOperation<IVectorView<AppExtension*>*>> findOperation;
    RETURN_IF_FAILED(catalog->FindAllAsync(&findOperation));

    ComPtr<IVectorView<AppExtension*>> extensionList;
    RETURN_IF_FAILED(wil::wait_for_completion_nothrow(findOperation.Get(), &extensionList));

    UINT extensionCount;
    RETURN_IF_FAILED(extensionList->get_Size(&extensionCount));
    for (UINT index = 0; index < extensionCount; index++)
    {
        T extensionMetadata;

        ComPtr<IAppExtension> extension;
        RETURN_IF_FAILED(extensionList->GetAt(index, &extension));

        ComPtr<IPackage> extensionPackage;
        RETURN_IF_FAILED(extension->get_Package(&extensionPackage));

        ComPtr<IPackage2> extensionPackage2;
        RETURN_IF_FAILED(extensionPackage.As(&extensionPackage2));

        ComPtr<IPackageId> extensionPackageId;
        RETURN_IF_FAILED(extensionPackage->get_Id(&extensionPackageId));

        HString publisherId;
        RETURN_IF_FAILED(extensionPackageId->get_PublisherId(publisherId.GetAddressOf()));

        HString name;
        RETURN_IF_FAILED(extensionPackage2->get_DisplayName(name.GetAddressOf()));
        extensionMetadata.name = std::wstring{ name.GetRawBuffer(nullptr) };

        HString publisher;
        RETURN_IF_FAILED(extensionPackage2->get_PublisherDisplayName(publisher.GetAddressOf()));
        extensionMetadata.author = std::wstring{ publisher.GetRawBuffer(nullptr) };

        // Try to get the logo. Don't completely bail if we fail to get it. It's non-critical.
        ComPtr<IUriRuntimeClass> logoUri;
        LOG_IF_FAILED(extensionPackage2->get_Logo(logoUri.GetAddressOf()));

        // If we did manage to get one, extract the string and store in the structure
        if (logoUri)
        {
            HString logo;

            RETURN_IF_FAILED(logoUri->get_AbsoluteUri(logo.GetAddressOf()));
            extensionMetadata.logo = std::wstring{ logo.GetRawBuffer(nullptr) };
        }

        HString pfn;
        RETURN_IF_FAILED(extensionPackageId->get_FamilyName(pfn.GetAddressOf()));
        extensionMetadata.pfn = std::wstring{ pfn.GetRawBuffer(nullptr) };

        PackageVersion version;
        RETURN_IF_FAILED(extensionPackageId->get_Version(&version));
        extensionMetadata.version.major = version.Major;
        extensionMetadata.version.minor = version.Minor;
        extensionMetadata.version.build = version.Build;
        extensionMetadata.version.revision = version.Revision;

        // Fetch the custom properties XML out of the extension information
        ComPtr<IAsyncOperation<IPropertySet*>> propertiesOperation;
        RETURN_IF_FAILED(extension->GetExtensionPropertiesAsync(&propertiesOperation));

        // Wait for async to complete and return the property set.
        ComPtr<IPropertySet> properties;
        RETURN_IF_FAILED(wil::wait_for_completion_nothrow(propertiesOperation.Get(), &properties));

        // We can't do anything on a set, but it must also be convertible to this type of map per the Windows.Foundation specs for this
        ComPtr<IMap<HSTRING, IInspectable*>> map;
        RETURN_IF_FAILED(properties.As(&map));

        // Looking it up is going to get us an inspectable
        ComPtr<IInspectable> inspectable;
        RETURN_IF_FAILED(map->Lookup(HStringReference(L"Clsid").Get(), &inspectable));

        // Unfortunately that inspectable is another set because we're dealing with XML data payload that we put in the manifest.
        ComPtr<IPropertySet> anotherSet;
        RETURN_IF_FAILED(inspectable.As(&anotherSet));

        // And we can't look at sets directly, so move it to map.
        ComPtr<IMap<HSTRING, IInspectable*>> anotherMap;
        RETURN_IF_FAILED(anotherSet.As(&anotherMap));

        // Use the magic value of #text to get the body between the XML tags. And of course it's an obtuse Inspectable again.
        ComPtr<IInspectable> anotherInspectable;
        RETURN_IF_FAILED(anotherMap->Lookup(HStringReference(L"#text").Get(), &anotherInspectable));

        // But this time that Inspectable is an IPropertyValue, which is a PROPVARIANT in a trench coat.
        ComPtr<IPropertyValue> propValue;
        RETURN_IF_FAILED(anotherInspectable.As(&propValue));

        // Check the type of the variant
        PropertyType propType;
        RETURN_IF_FAILED(propValue->get_Type(&propType));

        // If we're not a string, bail because I don't know what's going on.
        if (propType != PropertyType::PropertyType_String)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }

        // Get that string out.
        HString value;
        RETURN_IF_FAILED(propValue->GetString(value.GetAddressOf()));

        // Holy cow. It should be a GUID. Try to parse it.
        IID iid;
        RETURN_IF_FAILED(IIDFromString(value.GetRawBuffer(nullptr), &iid));

        extensionMetadata.clsid = iid;

        vec.emplace_back(std::move(extensionMetadata));
    }

    return S_OK;
}

[[nodiscard]] HRESULT DelegationConfig::s_GetAvailableConsoles(std::vector<DelegationConsole>& consoles) noexcept
try
{
    return _lookupCatalog(DELEGATION_CONSOLE_EXTENSION_NAME, consoles);
}
CATCH_RETURN()

[[nodiscard]] HRESULT DelegationConfig::s_GetAvailableTerminals(std::vector<DelegationTerminal>& terminals) noexcept
try
{
    return _lookupCatalog(DELEGATION_TERMINAL_EXTENSION_NAME, terminals);
}
CATCH_RETURN()

[[nodiscard]] HRESULT DelegationConfig::s_GetAvailablePackages(std::vector<DelegationPackage>& packages, DelegationPackage& defPackage) noexcept
try
{
    packages.clear();

    // Get consoles and terminals.
    // If we fail to look up any, we should still have ONE come back to us as the hardcoded default console host.
    // The errors aren't really useful except for debugging, so log only.
    std::vector<DelegationConsole> consoles;
    LOG_IF_FAILED(s_GetAvailableConsoles(consoles));

    std::vector<DelegationTerminal> terminals;
    LOG_IF_FAILED(s_GetAvailableTerminals(terminals));

    // TODO: I hate this algorithm (it's bad performance), but I couldn't
    // find an AppModel interface that would let me look up all the extensions
    // in one package.
    for (auto& term : terminals)
    {
        for (auto& con : consoles)
        {
            if (term.IsFromSamePackage(con))
            {
                DelegationPackage pkg;
                pkg.terminal = term;
                pkg.console = con;
                packages.push_back(pkg);
                break;
            }
        }
    }

    // We should find at least one package.
    RETURN_HR_IF(E_FAIL, packages.empty());

    // We also find the default here while we have the list of available ones so
    // we can return the opaque structure instead of the raw IID.
    IID defCon;
    LOG_IF_FAILED(s_GetDefaultConsoleId(defCon));
    IID defTerm;
    LOG_IF_FAILED(s_GetDefaultTerminalId(defTerm));

    // The default one is the 0th one because that's supposed to be the inbox conhost one.
    DelegationPackage chosenPackage = packages.at(0);

    // Search through and find a package that matches. If we failed to match because
    // it's torn across multiple or something not in the catalog, we'll offer the inbox conhost one.
    for (auto& pkg : packages)
    {
        if (pkg.console.clsid == defCon && pkg.terminal.clsid == defTerm)
        {
            chosenPackage = pkg;
            break;
        }
    }

    defPackage = chosenPackage;

    return S_OK;
}
CATCH_RETURN()

[[nodiscard]] HRESULT DelegationConfig::s_SetDefaultConsoleById(const IID& iid, const bool useRegExe) noexcept
{
    return s_Set(DELEGATION_CONSOLE_KEY_NAME, iid, useRegExe);
}

[[nodiscard]] HRESULT DelegationConfig::s_SetDefaultTerminalById(const IID& iid, const bool useRegExe) noexcept
{
    return s_Set(DELEGATION_TERMINAL_KEY_NAME, iid, useRegExe);
}

[[nodiscard]] HRESULT DelegationConfig::s_SetDefaultByPackage(const DelegationPackage& package, const bool useRegExe) noexcept
{
    RETURN_IF_FAILED(s_SetDefaultConsoleById(package.console.clsid, useRegExe));
    RETURN_IF_FAILED(s_SetDefaultTerminalById(package.terminal.clsid, useRegExe));
    return S_OK;
}

[[nodiscard]] HRESULT DelegationConfig::s_GetDefaultConsoleId(IID& iid) noexcept
{
    iid = { 0 };

    auto hr = s_Get(DELEGATION_CONSOLE_KEY_NAME, iid);

    bool defApp = false;
    if (SUCCEEDED(Microsoft::Console::Internal::DefaultApp::CheckShouldTerminalBeDefault(defApp)) && defApp)
    {
        if (FAILED(hr))
        {
            // If we can't find a user-defined delegation console/terminal, use the hardcoded
            // delegation console/terminal instead.
            iid = CLSID_SystemDelegationConsole;
            hr = S_OK;
        }
    }

    return hr;
}

[[nodiscard]] HRESULT DelegationConfig::s_GetDefaultTerminalId(IID& iid) noexcept
{
    iid = { 0 };

    auto hr = s_Get(DELEGATION_TERMINAL_KEY_NAME, iid);

    bool defApp = false;
    if (SUCCEEDED(Microsoft::Console::Internal::DefaultApp::CheckShouldTerminalBeDefault(defApp)) && defApp)
    {
        if (FAILED(hr))
        {
            // If we can't find a user-defined delegation console/terminal, use the hardcoded
            // delegation console/terminal instead.
            iid = CLSID_SystemDelegationTerminal;
            hr = S_OK;
        }
    }

    return hr;
}

[[nodiscard]] HRESULT DelegationConfig::s_Get(PCWSTR value, IID& iid) noexcept
{
    wil::unique_hkey currentUserKey;
    wil::unique_hkey consoleKey;

    RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_OpenConsoleKey(&currentUserKey, &consoleKey));

    wil::unique_hkey startupKey;
    RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_OpenKey(consoleKey.get(), L"%%Startup", &startupKey));

    DWORD bytesNeeded = 0;
    NTSTATUS result = RegistrySerialization::s_QueryValue(startupKey.get(),
                                                          value,
                                                          0,
                                                          REG_SZ,
                                                          nullptr,
                                                          &bytesNeeded);

    if (NTSTATUS_FROM_WIN32(ERROR_SUCCESS) != result)
    {
        RETURN_NTSTATUS(result);
    }

    auto buffer = std::make_unique<wchar_t[]>(bytesNeeded / sizeof(wchar_t) + 1);

    DWORD bytesUsed = 0;

    RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_QueryValue(startupKey.get(),
                                                                  value,
                                                                  bytesNeeded,
                                                                  REG_SZ,
                                                                  reinterpret_cast<BYTE*>(buffer.get()),
                                                                  &bytesUsed));

    RETURN_IF_FAILED(IIDFromString(buffer.get(), &iid));

    return S_OK;
}

[[nodiscard]] HRESULT DelegationConfig::s_Set(PCWSTR value, const CLSID clsid, const bool useRegExe) noexcept
try
{
    // BODGY
    // A Centennial application is not allowed to write the system registry and is redirected
    // to a per-package copy-on-write hive.
    // The restricted capability "unvirtualizedResources" can be combined with
    // desktop6:RegistryWriteVirtualization to opt-out... but...
    // - It will no longer be possible to double-click install through the App Installer
    // - It requires a special exception to submit to the store
    // - There MAY be some cleanup logic where the app catalog may try to undo
    //   whatever the package did.
    // This works around it by shelling out to reg.exe because somehow that's just peachy.
    if (useRegExe)
    {
        wil::unique_cotaskmem_string str;
        RETURN_IF_FAILED(StringFromCLSID(clsid, &str));

        auto regExePath = wil::ExpandEnvironmentStringsW<std::wstring>(L"%WINDIR%\\System32\\reg.exe");

        auto command = wil::str_printf<std::wstring>(L"%s ADD HKCU\\Console\\%%%%Startup /v %s /t REG_SZ /d %s /f", regExePath.c_str(), value, str.get());

        wil::unique_process_information pi;
        STARTUPINFOEX siEx{ 0 };
        siEx.StartupInfo.cb = sizeof(siEx);

        RETURN_IF_WIN32_BOOL_FALSE(CreateProcessW(
            nullptr,
            command.data(),
            nullptr, // lpProcessAttributes
            nullptr, // lpThreadAttributes
            false, // bInheritHandles
            EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW, // dwCreationFlags
            nullptr, // lpEnvironment
            nullptr,
            &siEx.StartupInfo, // lpStartupInfo
            &pi // lpProcessInformation
            ));

        return S_OK;
    }
    else
    {
        wil::unique_hkey currentUserKey;
        wil::unique_hkey consoleKey;

        RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_OpenConsoleKey(&currentUserKey, &consoleKey));

        // Create method for registry is a "create if not exists, otherwise open" function.
        wil::unique_hkey startupKey;
        RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_CreateKey(consoleKey.get(), L"%%Startup", &startupKey));

        wil::unique_cotaskmem_string str;
        RETURN_IF_FAILED(StringFromCLSID(clsid, &str));

        RETURN_IF_NTSTATUS_FAILED(RegistrySerialization::s_SetValue(startupKey.get(), value, REG_SZ, reinterpret_cast<BYTE*>(str.get()), gsl::narrow<DWORD>(wcslen(str.get()) * sizeof(wchar_t))));

        return S_OK;
    }
}
CATCH_RETURN()
