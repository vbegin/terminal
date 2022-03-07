// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "EnumEntry.h"
#include "GlobalAppearance.h"
#include "GlobalAppearance.g.cpp"
#include "GlobalAppearancePageNavigationState.g.cpp"

#include <LibraryResources.h>
#include <WtExeUtils.h>

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Microsoft::Terminal::Settings::Model;
using namespace winrt::Windows::Foundation::Collections;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
    // For ComboBox an empty SelectedItem string denotes no selection.
    // What we want instead is for "Use system language" to be selected by default.
    // --> "und" is synonymous for "Use system language".
    constexpr std::wstring_view systemLanguageTag{ L"und" };

    static constexpr std::array appLanguageTags{
        L"en-US",
        L"de-DE",
        L"es-ES",
        L"fr-FR",
        L"it-IT",
        L"ja",
        L"ko",
        L"pt-BR",
        L"qps-PLOC",
        L"qps-PLOCA",
        L"qps-PLOCM",
        L"ru",
        L"zh-Hans-CN",
        L"zh-Hant-TW",
    };

    GlobalAppearance::GlobalAppearance()
    {
        InitializeComponent();

        INITIALIZE_BINDABLE_ENUM_SETTING(Theme, ElementTheme, winrt::Windows::UI::Xaml::ElementTheme, L"Globals_Theme", L"Content");
        INITIALIZE_BINDABLE_ENUM_SETTING(TabWidthMode, TabViewWidthMode, winrt::Microsoft::UI::Xaml::Controls::TabViewWidthMode, L"Globals_TabWidthMode", L"Content");
    }

    void GlobalAppearance::OnNavigatedTo(const NavigationEventArgs& e)
    {
        _State = e.Parameter().as<Editor::GlobalAppearancePageNavigationState>();
    }

    winrt::hstring GlobalAppearance::LanguageDisplayConverter(const winrt::hstring& tag)
    {
        if (tag == systemLanguageTag)
        {
            return RS_(L"Globals_LanguageDefault");
        }

        winrt::Windows::Globalization::Language language{ tag };
        return language.NativeName();
    }

    // Returns whether the language selector is available/shown.
    //
    // winrt::Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride()
    // doesn't work for unpackaged applications. The corresponding code in TerminalApp is disabled.
    // It would be confusing for our users if we presented a dysfunctional language selector.
    bool GlobalAppearance::LanguageSelectorAvailable()
    {
        return IsPackaged();
    }

    // Returns the list of languages the user may override the application language with.
    // The returned list are BCP 47 language tags like {"und", "en-US", "de-DE", "es-ES", ...}.
    // "und" is short for "undefined" and is synonymous for "Use system language" in this code.
    winrt::Windows::Foundation::Collections::IObservableVector<winrt::hstring> GlobalAppearance::LanguageList()
    {
        if (_languageList)
        {
            return _languageList;
        }

        if (!LanguageSelectorAvailable())
        {
            _languageList = {};
            return _languageList;
        }

        // In order to return the language list this code does the following:
        // [1] Get all possible languages we want to allow the user to choose.
        //     We have to acquire languages from multiple sources, creating duplicates. See below at [1].
        // [2] Sort languages by their ASCII tags, forcing the UI in a consistent/stable order.
        //     I wanted to sort the localized language names initially, but it turned out to be complex.
        // [3] Remove potential duplicates in our language list from [1].
        //     We don't want to have en-US twice in the list, do we?
        // [4] Optionally remove unwanted language tags (like pseudo-localizations).

        std::vector<winrt::hstring> tags;

        // [1]:
        {
            // ManifestLanguages contains languages the app ships with.
            // Unfortunately, we cannot use this source. Our manifest must contain the
            // ~100 languages that are localized for the shell extension and start menu
            // presentation so we align with Windows display languages for those surfaces.
            // However, the actual content of our application is limited to a much smaller
            // subset of approximately 14 languages. As such, we will code the limited
            // subset of languages that we support for selection within the Settings
            // dropdown to steer users towards the ones that we can display in the app.

            // As per the function definition, the first item
            // is always "Use system language" ("und").
            tags.emplace_back(systemLanguageTag);

            // Add our hardcoded languages after the system definition.
            for (const auto& v : appLanguageTags)
            {
                tags.push_back(v);
            }
        }

        // NOTE: The size of tags is always >0, due to tags[0] being hardcoded to "und".
        const auto tagsBegin = ++tags.begin();
        const auto tagsEnd = tags.end();

        // [2]:
        std::sort(tagsBegin, tagsEnd);

        // I'd love for both, std::unique and std::remove_if, to occur in a single loop,
        // but the code turned out to be complex and even less maintainable, so I gave up.
        {
            // [3] part 1:
            auto it = std::unique(tagsBegin, tagsEnd);

            // The qps- languages are useful for testing ("pseudo-localization").
            // --> Leave them in if debug features are enabled.
            if (!_State.Globals().DebugFeaturesEnabled())
            {
                // [4] part 1:
                it = std::remove_if(tagsBegin, it, [](const winrt::hstring& tag) -> bool {
                    return til::starts_with(tag, L"qps-");
                });
            }

            // [3], [4] part 2 (completing the so called "erase-remove idiom"):
            tags.erase(it, tagsEnd);
        }

        _languageList = winrt::single_threaded_observable_vector(std::move(tags));
        return _languageList;
    }

    winrt::Windows::Foundation::IInspectable GlobalAppearance::CurrentLanguage()
    {
        if (_currentLanguage)
        {
            return _currentLanguage;
        }

        if (!LanguageSelectorAvailable())
        {
            _currentLanguage = {};
            return _currentLanguage;
        }

        // NOTE: PrimaryLanguageOverride throws if this instance is unpackaged.
        auto currentLanguage = winrt::Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride();
        if (currentLanguage.empty())
        {
            currentLanguage = systemLanguageTag;
        }

        _currentLanguage = winrt::box_value(currentLanguage);
        return _currentLanguage;
    }

    void GlobalAppearance::CurrentLanguage(const winrt::Windows::Foundation::IInspectable& tag)
    {
        _currentLanguage = tag;

        const auto currentLanguage = winrt::unbox_value<winrt::hstring>(_currentLanguage);
        const auto globals = _State.Globals();
        if (currentLanguage == systemLanguageTag)
        {
            globals.ClearLanguage();
        }
        else
        {
            globals.Language(currentLanguage);
        }
    }

}
