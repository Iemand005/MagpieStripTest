#include "pch.h"
#include "MainWindow.h"
#include "App.h"
#include "CommonSharedConstants.h"
#include "resource.h"
#include "Win32Helper.h"
#include <ShellScalingApi.h>

using namespace winrt;
using namespace winrt::Magpie::implementation;

namespace Magpie {

bool MainWindow::Create() noexcept {
[[maybe_unused]] static Ignore _ = [] {
const HINSTANCE hInstance = wil::GetModuleInstanceHandle();

WNDCLASSEXW wcex{
.cbSize = sizeof(wcex),
.lpfnWndProc = _WndProc,
.hInstance = hInstance,
.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP)),
.hCursor = LoadCursor(nullptr, IDC_ARROW),
.lpszClassName = CommonSharedConstants::MAIN_WINDOW_CLASS_NAME
};
RegisterClassEx(&wcex);

return Ignore();
}();

const auto& [posToSet, sizeToSet] = _CreateWindow();

if (!Handle()) {
return false;
}

_Content(make_self<RootPage>());

_appThemeChangedRevoker = App::Get().ThemeChanged(auto_revoke, [this](bool) { _UpdateTheme(); });
_UpdateTheme();

// Set window position and size
SetWindowPos(Handle(), NULL, posToSet.x, posToSet.y, sizeToSet.cx, sizeToSet.cy,
(sizeToSet.cx == 0 ? (SWP_NOMOVE | SWP_NOSIZE) : 0) | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS);

// Show window after XAML is loaded
Content()->Loaded([this](winrt::IInspectable const&, RoutedEventArgs const&) {
ShowWindow(Handle(), SW_SHOWNORMAL);
SetForegroundWindow(Handle());
});

return true;
}

void MainWindow::Show() const noexcept {
if (IsIconic(Handle())) {
ShowWindow(Handle(), SW_RESTORE);
}

SetForegroundWindow(Handle());
}

LRESULT MainWindow::_MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
switch (msg) {
case WM_SIZE:
{
base_type::_MessageHandler(WM_SIZE, wParam, lParam);
return 0;
}
case WM_GETMINMAXINFO:
{
// Set minimum window size
MINMAXINFO* mmi = (MINMAXINFO*)lParam;
mmi->ptMinTrackSize = { 
std::lroundf(300 * CurrentDpi() / float(USER_DEFAULT_SCREEN_DPI)),
std::lroundf(200 * CurrentDpi() / float(USER_DEFAULT_SCREEN_DPI))
};
return 0;
}
}

return base_type::_MessageHandler(msg, wParam, lParam);
}

std::pair<POINT, SIZE> MainWindow::_CreateWindow() noexcept {
// Get monitor info for initial positioning
POINT posToSet = { CW_USEDEFAULT, CW_USEDEFAULT };
SIZE sizeToSet = { 800, 600 };

if (HMONITOR hMon = MonitorFromPoint(posToSet, MONITOR_DEFAULTTOPRIMARY)) {
MONITORINFO mi{ .cbSize = sizeof(mi) };
if (GetMonitorInfo(hMon, &mi)) {
posToSet = { mi.rcWork.left, mi.rcWork.top };
}
}

const HINSTANCE hInstance = wil::GetModuleInstanceHandle();
HWND hWnd = CreateWindowEx(
WS_EX_NOREDIRECTIONBITMAP,
CommonSharedConstants::MAIN_WINDOW_CLASS_NAME,
L"Magpie",
WS_OVERLAPPEDWINDOW,
posToSet.x,
posToSet.y,
sizeToSet.cx,
sizeToSet.cy,
nullptr,
nullptr,
hInstance,
this
);

if (!hWnd) {
return { posToSet, sizeToSet };
}

Handle(hWnd);
return { posToSet, sizeToSet };
}

void MainWindow::_UpdateTheme() noexcept {
const HWND hWnd = Handle();
const bool isLightTheme = App::Get().IsLightTheme();

// Set window theme
Win32Helper::OSVersion osVersion = Win32Helper::GetOSVersion();
if (osVersion.IsWin11()) {
// Set Mica background for Windows 11
DWM_SYSTEMBACKDROP_TYPE value = DWMSBT_MAINWINDOW;
DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
}
}

}
