/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- IRenderEngine.hpp

Abstract:
- This serves as the entry point for a specific graphics engine specific renderer.

Author(s):
- Michael Niksa (MiNiksa) 17-Nov-2015
--*/

#pragma once

#include <d2d1.h>

#include "CursorOptions.h"
#include "Cluster.hpp"
#include "FontInfoDesired.hpp"
#include "IRenderData.hpp"
#include "RenderSettings.hpp"
#include "../../buffer/out/LineRendition.hpp"

#pragma warning(push)
#pragma warning(disable : 4100) // '...': unreferenced formal parameter
namespace Microsoft::Console::Render
{
    struct RenderFrameInfo
    {
        std::optional<CursorOptions> cursorInfo;
    };

    class __declspec(novtable) IRenderEngine
    {
    public:
        enum class GridLines
        {
            None,
            Top,
            Bottom,
            Left,
            Right,
            Underline,
            DoubleUnderline,
            Strikethrough,
            HyperlinkUnderline
        };
        using GridLineSet = til::enumset<GridLines>;

#pragma warning(suppress : 26432) // If you define or delete any default operation in the type '...', define or delete them all (c.21).
        virtual ~IRenderEngine()
        {
        }

        [[nodiscard]] virtual HRESULT StartPaint() noexcept = 0;
        [[nodiscard]] virtual HRESULT EndPaint() noexcept = 0;
        [[nodiscard]] virtual bool RequiresContinuousRedraw() noexcept = 0;
        virtual void WaitUntilCanRender() noexcept = 0;
        [[nodiscard]] virtual HRESULT Present() noexcept = 0;
        [[nodiscard]] virtual HRESULT PrepareForTeardown(_Out_ bool* pForcePaint) noexcept = 0;
        [[nodiscard]] virtual HRESULT ScrollFrame() noexcept = 0;
        [[nodiscard]] virtual HRESULT Invalidate(const SMALL_RECT* psrRegion) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateCursor(const SMALL_RECT* psrRegion) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateSystem(const RECT* prcDirtyClient) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateSelection(const std::vector<SMALL_RECT>& rectangles) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateScroll(const COORD* pcoordDelta) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateAll() noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateFlush(_In_ const bool circled, _Out_ bool* const pForcePaint) noexcept = 0;
        [[nodiscard]] virtual HRESULT InvalidateTitle(std::wstring_view proposedTitle) noexcept = 0;
        [[nodiscard]] virtual HRESULT NotifyNewText(const std::wstring_view newText) noexcept = 0;
        [[nodiscard]] virtual HRESULT PrepareRenderInfo(const RenderFrameInfo& info) noexcept = 0;
        [[nodiscard]] virtual HRESULT ResetLineTransform() noexcept = 0;
        [[nodiscard]] virtual HRESULT PrepareLineTransform(LineRendition lineRendition, size_t targetRow, size_t viewportLeft) noexcept = 0;
        [[nodiscard]] virtual HRESULT PaintBackground() noexcept = 0;
        [[nodiscard]] virtual HRESULT PaintBufferLine(gsl::span<const Cluster> clusters, COORD coord, bool fTrimLeft, bool lineWrapped) noexcept = 0;
        [[nodiscard]] virtual HRESULT PaintBufferGridLines(GridLineSet lines, COLORREF color, size_t cchLine, COORD coordTarget) noexcept = 0;
        [[nodiscard]] virtual HRESULT PaintSelection(SMALL_RECT rect) noexcept = 0;
        [[nodiscard]] virtual HRESULT PaintCursor(const CursorOptions& options) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateDrawingBrushes(const TextAttribute& textAttributes, const RenderSettings& renderSettings, gsl::not_null<IRenderData*> pData, bool usingSoftFont, bool isSettingDefaultBrushes) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateFont(const FontInfoDesired& FontInfoDesired, _Out_ FontInfo& FontInfo) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateSoftFont(gsl::span<const uint16_t> bitPattern, SIZE cellSize, size_t centeringHint) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateDpi(int iDpi) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateViewport(SMALL_RECT srNewViewport) noexcept = 0;
        [[nodiscard]] virtual HRESULT GetProposedFont(const FontInfoDesired& FontInfoDesired, _Out_ FontInfo& FontInfo, int iDpi) noexcept = 0;
        [[nodiscard]] virtual HRESULT GetDirtyArea(gsl::span<const til::rect>& area) noexcept = 0;
        [[nodiscard]] virtual HRESULT GetFontSize(_Out_ COORD* pFontSize) noexcept = 0;
        [[nodiscard]] virtual HRESULT IsGlyphWideByFont(std::wstring_view glyph, _Out_ bool* pResult) noexcept = 0;
        [[nodiscard]] virtual HRESULT UpdateTitle(std::wstring_view newTitle) noexcept = 0;

        // The following functions used to be specific to the DxRenderer and they should
        // be abstracted away and integrated into the above or simply get removed.

        // DxRenderer - getter
        virtual HRESULT Enable() noexcept { return S_OK; }
        virtual [[nodiscard]] bool GetRetroTerminalEffect() const noexcept { return false; }
        virtual [[nodiscard]] float GetScaling() const noexcept { return 1; }
#pragma warning(suppress : 26440) // Function '...' can be declared 'noexcept' (f.6).
        virtual [[nodiscard]] HANDLE GetSwapChainHandle()
        {
            return nullptr;
        }
        virtual [[nodiscard]] Types::Viewport GetViewportInCharacters(const Types::Viewport& viewInPixels) const noexcept { return Types::Viewport::Empty(); }
        virtual [[nodiscard]] Types::Viewport GetViewportInPixels(const Types::Viewport& viewInCharacters) const noexcept { return Types::Viewport::Empty(); }
        // DxRenderer - setter
        virtual void SetAntialiasingMode(const D2D1_TEXT_ANTIALIAS_MODE antialiasingMode) noexcept {}
        virtual void SetCallback(std::function<void()> pfn) noexcept {}
        virtual void EnableTransparentBackground(const bool isTransparent) noexcept {}
        virtual void SetForceFullRepaintRendering(bool enable) noexcept {}
        virtual [[nodiscard]] HRESULT SetHwnd(const HWND hwnd) noexcept { return E_NOTIMPL; }
        virtual void SetPixelShaderPath(std::wstring_view value) noexcept {}
        virtual void SetRetroTerminalEffect(bool enable) noexcept {}
        virtual void SetSelectionBackground(const COLORREF color, const float alpha = 0.5f) noexcept {}
        virtual void SetSoftwareRendering(bool enable) noexcept {}
        virtual void SetWarningCallback(std::function<void(HRESULT)> pfn) noexcept {}
        virtual [[nodiscard]] HRESULT SetWindowSize(const SIZE pixels) noexcept { return E_NOTIMPL; }
        virtual void ToggleShaderEffects() noexcept {}
        virtual [[nodiscard]] HRESULT UpdateFont(const FontInfoDesired& pfiFontInfoDesired, FontInfo& fiFontInfo, const std::unordered_map<std::wstring_view, uint32_t>& features, const std::unordered_map<std::wstring_view, float>& axes) noexcept { return E_NOTIMPL; }
        virtual void UpdateHyperlinkHoveredId(const uint16_t hoveredId) noexcept {}
    };
}
#pragma warning(pop)
