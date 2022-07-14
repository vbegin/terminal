// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
Module Name:
- ITermDispatch.hpp

Abstract:
- This is the interface for all output state machine callbacks. When actions
    occur, they will be dispatched to the methods on this interface which must
    be implemented by a child class and passed into the state machine on
    creation.
*/
#pragma once
#include "DispatchTypes.hpp"
#include "../buffer/out/LineRendition.hpp"

namespace Microsoft::Console::VirtualTerminal
{
    class ITermDispatch;
};

class Microsoft::Console::VirtualTerminal::ITermDispatch
{
public:
    using StringHandler = std::function<bool(const wchar_t)>;

#pragma warning(push)
#pragma warning(disable : 26432) // suppress rule of 5 violation on interface because tampering with this is fraught with peril
    virtual ~ITermDispatch() = 0;

    virtual void Print(const wchar_t wchPrintable) = 0;
    virtual void PrintString(const std::wstring_view string) = 0;

    virtual bool CursorUp(const VTInt distance) = 0; // CUU
    virtual bool CursorDown(const VTInt distance) = 0; // CUD
    virtual bool CursorForward(const VTInt distance) = 0; // CUF
    virtual bool CursorBackward(const VTInt distance) = 0; // CUB, BS
    virtual bool CursorNextLine(const VTInt distance) = 0; // CNL
    virtual bool CursorPrevLine(const VTInt distance) = 0; // CPL
    virtual bool CursorHorizontalPositionAbsolute(const VTInt column) = 0; // HPA, CHA
    virtual bool VerticalLinePositionAbsolute(const VTInt line) = 0; // VPA
    virtual bool HorizontalPositionRelative(const VTInt distance) = 0; // HPR
    virtual bool VerticalPositionRelative(const VTInt distance) = 0; // VPR
    virtual bool CursorPosition(const VTInt line, const VTInt column) = 0; // CUP, HVP
    virtual bool CursorSaveState() = 0; // DECSC
    virtual bool CursorRestoreState() = 0; // DECRC
    virtual bool CursorVisibility(const bool isVisible) = 0; // DECTCEM
    virtual bool InsertCharacter(const VTInt count) = 0; // ICH
    virtual bool DeleteCharacter(const VTInt count) = 0; // DCH
    virtual bool ScrollUp(const VTInt distance) = 0; // SU
    virtual bool ScrollDown(const VTInt distance) = 0; // SD
    virtual bool InsertLine(const VTInt distance) = 0; // IL
    virtual bool DeleteLine(const VTInt distance) = 0; // DL
    virtual bool SetColumns(const VTInt columns) = 0; // DECCOLM
    virtual bool SetCursorKeysMode(const bool applicationMode) = 0; // DECCKM
    virtual bool SetKeypadMode(const bool applicationMode) = 0; // DECKPAM, DECKPNM
    virtual bool EnableWin32InputMode(const bool win32InputMode) = 0; // win32-input-mode
    virtual bool EnableCursorBlinking(const bool enable) = 0; // ATT610
    virtual bool SetAnsiMode(const bool ansiMode) = 0; // DECANM
    virtual bool SetScreenMode(const bool reverseMode) = 0; // DECSCNM
    virtual bool SetOriginMode(const bool relativeMode) = 0; // DECOM
    virtual bool SetAutoWrapMode(const bool wrapAtEOL) = 0; // DECAWM
    virtual bool SetTopBottomScrollingMargins(const VTInt topMargin, const VTInt bottomMargin) = 0; // DECSTBM
    virtual bool WarningBell() = 0; // BEL
    virtual bool CarriageReturn() = 0; // CR
    virtual bool LineFeed(const DispatchTypes::LineFeedType lineFeedType) = 0; // IND, NEL, LF, FF, VT
    virtual bool ReverseLineFeed() = 0; // RI
    virtual bool SetWindowTitle(std::wstring_view title) = 0; // OscWindowTitle
    virtual bool UseAlternateScreenBuffer() = 0; // ASBSET
    virtual bool UseMainScreenBuffer() = 0; // ASBRST
    virtual bool HorizontalTabSet() = 0; // HTS
    virtual bool ForwardTab(const VTInt numTabs) = 0; // CHT, HT
    virtual bool BackwardsTab(const VTInt numTabs) = 0; // CBT
    virtual bool TabClear(const DispatchTypes::TabClearType clearType) = 0; // TBC
    virtual bool EnableDECCOLMSupport(const bool enabled) = 0; // ?40
    virtual bool EnableVT200MouseMode(const bool enabled) = 0; // ?1000
    virtual bool EnableUTF8ExtendedMouseMode(const bool enabled) = 0; // ?1005
    virtual bool EnableSGRExtendedMouseMode(const bool enabled) = 0; // ?1006
    virtual bool EnableButtonEventMouseMode(const bool enabled) = 0; // ?1002
    virtual bool EnableAnyEventMouseMode(const bool enabled) = 0; // ?1003
    virtual bool EnableFocusEventMode(const bool enabled) = 0; // ?1004
    virtual bool EnableAlternateScroll(const bool enabled) = 0; // ?1007
    virtual bool EnableXtermBracketedPasteMode(const bool enabled) = 0; // ?2004
    virtual bool SetColorTableEntry(const size_t tableIndex, const DWORD color) = 0; // OSCColorTable
    virtual bool SetDefaultForeground(const DWORD color) = 0; // OSCDefaultForeground
    virtual bool SetDefaultBackground(const DWORD color) = 0; // OSCDefaultBackground
    virtual bool AssignColor(const DispatchTypes::ColorItem item, const VTInt fgIndex, const VTInt bgIndex) = 0; // DECAC

    virtual bool EraseInDisplay(const DispatchTypes::EraseType eraseType) = 0; // ED
    virtual bool EraseInLine(const DispatchTypes::EraseType eraseType) = 0; // EL
    virtual bool EraseCharacters(const VTInt numChars) = 0; // ECH

    virtual bool SetGraphicsRendition(const VTParameters options) = 0; // SGR
    virtual bool SetLineRendition(const LineRendition rendition) = 0; // DECSWL, DECDWL, DECDHL

    virtual bool PushGraphicsRendition(const VTParameters options) = 0; // XTPUSHSGR
    virtual bool PopGraphicsRendition() = 0; // XTPOPSGR

    virtual bool SetMode(const DispatchTypes::ModeParams param) = 0; // DECSET

    virtual bool ResetMode(const DispatchTypes::ModeParams param) = 0; // DECRST

    virtual bool DeviceStatusReport(const DispatchTypes::AnsiStatusType statusType) = 0; // DSR, DSR-OS, DSR-CPR
    virtual bool DeviceAttributes() = 0; // DA1
    virtual bool SecondaryDeviceAttributes() = 0; // DA2
    virtual bool TertiaryDeviceAttributes() = 0; // DA3
    virtual bool Vt52DeviceAttributes() = 0; // VT52 Identify
    virtual bool RequestTerminalParameters(const DispatchTypes::ReportingPermission permission) = 0; // DECREQTPARM

    virtual bool DesignateCodingSystem(const VTID codingSystem) = 0; // DOCS
    virtual bool Designate94Charset(const VTInt gsetNumber, const VTID charset) = 0; // SCS
    virtual bool Designate96Charset(const VTInt gsetNumber, const VTID charset) = 0; // SCS
    virtual bool LockingShift(const VTInt gsetNumber) = 0; // LS0, LS1, LS2, LS3
    virtual bool LockingShiftRight(const VTInt gsetNumber) = 0; // LS1R, LS2R, LS3R
    virtual bool SingleShift(const VTInt gsetNumber) = 0; // SS2, SS3
    virtual bool AcceptC1Controls(const bool enabled) = 0; // DECAC1

    virtual bool SoftReset() = 0; // DECSTR
    virtual bool HardReset() = 0; // RIS
    virtual bool ScreenAlignmentPattern() = 0; // DECALN

    virtual bool SetCursorStyle(const DispatchTypes::CursorStyle cursorStyle) = 0; // DECSCUSR
    virtual bool SetCursorColor(const COLORREF color) = 0; // OSCSetCursorColor, OSCResetCursorColor

    virtual bool SetClipboard(std::wstring_view content) = 0; // OSCSetClipboard

    // DTTERM_WindowManipulation
    virtual bool WindowManipulation(const DispatchTypes::WindowManipulationType function,
                                    const VTParameter parameter1,
                                    const VTParameter parameter2) = 0;

    virtual bool AddHyperlink(const std::wstring_view uri, const std::wstring_view params) = 0;
    virtual bool EndHyperlink() = 0;

    virtual bool DoConEmuAction(const std::wstring_view string) = 0;

    virtual bool DoITerm2Action(const std::wstring_view string) = 0;

    virtual bool DoFinalTermAction(const std::wstring_view string) = 0;

    virtual StringHandler DownloadDRCS(const VTInt fontNumber,
                                       const VTParameter startChar,
                                       const DispatchTypes::DrcsEraseControl eraseControl,
                                       const DispatchTypes::DrcsCellMatrix cellMatrix,
                                       const DispatchTypes::DrcsFontSet fontSet,
                                       const DispatchTypes::DrcsFontUsage fontUsage,
                                       const VTParameter cellHeight,
                                       const DispatchTypes::DrcsCharsetSize charsetSize) = 0; // DECDLD

    virtual StringHandler RestoreTerminalState(const DispatchTypes::ReportFormat format) = 0; // DECRSTS

    virtual StringHandler RequestSetting() = 0; // DECRQSS

    virtual bool PlaySounds(const VTParameters parameters) = 0; // DECPS
};
inline Microsoft::Console::VirtualTerminal::ITermDispatch::~ITermDispatch() {}
#pragma warning(pop)
