// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "winrt/Microsoft.UI.Xaml.Controls.h"

#include "HighlightedTextSegment.g.h"
#include "HighlightedText.g.h"

namespace winrt::TerminalApp::implementation
{
    struct HighlightedTextSegment : HighlightedTextSegmentT<HighlightedTextSegment>
    {
        HighlightedTextSegment() = default;
        HighlightedTextSegment(winrt::hstring const& text, bool isHighlighted);

        WINRT_CALLBACK(PropertyChanged, Windows::UI::Xaml::Data::PropertyChangedEventHandler);
        WINRT_OBSERVABLE_PROPERTY(winrt::hstring, TextSegment, _PropertyChangedHandlers);
        WINRT_OBSERVABLE_PROPERTY(bool, IsHighlighted, _PropertyChangedHandlers);
    };

    struct HighlightedText : HighlightedTextT<HighlightedText>
    {
        HighlightedText() = default;
        HighlightedText(Windows::Foundation::Collections::IObservableVector<winrt::TerminalApp::HighlightedTextSegment> const& segments);

        WINRT_CALLBACK(PropertyChanged, Windows::UI::Xaml::Data::PropertyChangedEventHandler);
        WINRT_OBSERVABLE_PROPERTY(Windows::Foundation::Collections::IObservableVector<winrt::TerminalApp::HighlightedTextSegment>, Segments, _PropertyChangedHandlers);
    };
}

namespace winrt::TerminalApp::factory_implementation
{
    BASIC_FACTORY(HighlightedTextSegment);
    BASIC_FACTORY(HighlightedText);
}
