// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
//
// Implementation of the TitlebarControl class
//

#include "pch.h"

#include "TitlebarControl.h"

#include "TitlebarControl.g.cpp"

namespace winrt::TerminalApp::implementation
{
    TitlebarControl::TitlebarControl(uint64_t handle) :
        _window{ reinterpret_cast<HWND>(handle) }
    {
        InitializeComponent();

        // Register our event handlers on the MMC buttons.
        MinMaxCloseControl().MinimizeClick({ this, &TitlebarControl::Minimize_Click });
        MinMaxCloseControl().MaximizeClick({ this, &TitlebarControl::Maximize_Click });
        MinMaxCloseControl().CloseClick({ this, &TitlebarControl::Close_Click });
    }

    double TitlebarControl::CaptionButtonWidth()
    {
        // Divide by three, since we know there are only three buttons. When
        // Windows 12 comes along and adds another, we can update this /s
        static double width{ MinMaxCloseControl().ActualWidth() / 3.0 };
        return width;
    }

    IInspectable TitlebarControl::Content()
    {
        return ContentRoot().Content();
    }

    void TitlebarControl::Content(IInspectable content)
    {
        ContentRoot().Content(content);
    }

    void TitlebarControl::Root_SizeChanged(const IInspectable& /*sender*/,
                                           const Windows::UI::Xaml::SizeChangedEventArgs& /*e*/)
    {
        const auto windowWidth = ActualWidth();
        const auto minMaxCloseWidth = MinMaxCloseControl().ActualWidth();
        const auto dragBarMinWidth = DragBar().MinWidth();
        const auto maxWidth = windowWidth - minMaxCloseWidth - dragBarMinWidth;
        // Only set our MaxWidth if it's greater than 0. Setting it to a
        // negative value will cause a crash.
        if (maxWidth >= 0)
        {
            ContentRoot().MaxWidth(maxWidth);
        }
    }

    void TitlebarControl::_OnMaximizeOrRestore(byte flag)
    {
        POINT point1 = {};
        ::GetCursorPos(&point1);
        const LPARAM lParam = MAKELPARAM(point1.x, point1.y);
        WINDOWPLACEMENT placement = { sizeof(placement) };
        ::GetWindowPlacement(_window, &placement);
        if (placement.showCmd == SW_SHOWNORMAL)
        {
            ::PostMessage(_window, WM_SYSCOMMAND, SC_MAXIMIZE | flag, lParam);
        }
        else if (placement.showCmd == SW_SHOWMAXIMIZED)
        {
            ::PostMessage(_window, WM_SYSCOMMAND, SC_RESTORE | flag, lParam);
        }
    }

    void TitlebarControl::Maximize_Click(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        _OnMaximizeOrRestore(HTMAXBUTTON);
    }

    void TitlebarControl::DragBar_DoubleTapped(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& /*e*/)
    {
        _OnMaximizeOrRestore(HTCAPTION);
    }

    void TitlebarControl::Minimize_Click(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        if (_window)
        {
            ::PostMessage(_window, WM_SYSCOMMAND, SC_MINIMIZE | HTMINBUTTON, 0);
        }
    }

    void TitlebarControl::Close_Click(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        ::PostMessage(_window, WM_SYSCOMMAND, SC_CLOSE, 0);
    }

    void TitlebarControl::SetWindowVisualState(WindowVisualState visualState)
    {
        MinMaxCloseControl().SetWindowVisualState(visualState);
    }

    // GH#9443: HoverButton, PressButton, ClickButton and ReleaseButtons are all
    // used to manually interact with the buttons, in the same way that XAML
    // would normally send events.

    void TitlebarControl::HoverButton(CaptionButton button)
    {
        MinMaxCloseControl().HoverButton(button);
    }
    void TitlebarControl::PressButton(CaptionButton button)
    {
        MinMaxCloseControl().PressButton(button);
    }
    winrt::fire_and_forget TitlebarControl::ClickButton(CaptionButton button)
    {
        // GH#8587: Handle this on the _next_ pass of the UI thread. If we
        // handle this immediately, then we'll accidentally leave the button in
        // the "Hovered" state when we minimize. This will leave the button
        // visibly hovered in the taskbar preview for our window.
        auto weakThis{ get_weak() };
        co_await MinMaxCloseControl().Dispatcher();
        if (auto self{ weakThis.get() })
        {
            // Just handle this in the same way we would if the button were
            // clicked normally.
            switch (button)
            {
            case CaptionButton::Minimize:
                Minimize_Click(nullptr, nullptr);
                break;
            case CaptionButton::Maximize:
                Maximize_Click(nullptr, nullptr);
                break;
            case CaptionButton::Close:
                Close_Click(nullptr, nullptr);
                break;
            }
        }
    }
    void TitlebarControl::ReleaseButtons()
    {
        MinMaxCloseControl().ReleaseButtons();
    }

}
