#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif
#include "CommonSharedConstants.h"
#include "Logger.h"
#include "MainWindow.h"
#include "Win32Helper.h"

using namespace ::Magpie;
using namespace winrt;
using namespace Windows::UI::ViewManagement;

namespace winrt::Magpie::implementation {

static void FixThreadPoolCrash() noexcept {
assert(!GetModuleHandle(L"Windows.UI.Xaml.dll"));
LoadLibraryEx(L"twinapi.appcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
}

App& App::Get() {
static com_ptr<App> instance = [] {
FixThreadPoolCrash();
return make_self<App>();
}();

return *instance;
}

App::App() {
UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e) {
Logger::Get().ComCritical("Unhandled exception", e.Exception().value);

if (IsDebuggerPresent()) {
__debugbreak();
}
});
}

bool App::Initialize(const wchar_t* /*arguments*/) {
_mainWindow = std::make_unique<class MainWindow>();

// Initialize XAML framework
_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();

// Win10 中隐藏 DesktopWindowXamlSource 窗口
if (Win32Helper::GetOSVersion().IsWin10()) {
if (CoreWindow coreWindow = CoreWindow::GetForCurrentThread()) {
HWND hwndDWXS;
coreWindow.try_as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
ShowWindow(hwndDWXS, SW_HIDE);
}
}

// Get DispatcherQueue after WindowsXamlManager initialization
_dispatcher = winrt::DispatcherQueue::GetForCurrentThread();

_UpdateTheme();

if (!_mainWindow->Create()) {
_Uninitialize();
return false;
}

return true;
}

int App::Run() {
MSG msg;
while (GetMessage(&msg, nullptr, 0, 0)) {
_mainWindow->HandleMessage(msg);
}

_Uninitialize();

Logger::Get().Info("Application exiting");
Logger::Get().Flush();

return (int)msg.wParam;
}

void App::ShowMainWindow() noexcept {
if (*_mainWindow) {
_mainWindow->Show();
} else {
_mainWindow->Create();
}
}

void App::Quit() {
_mainWindow->Destroy();
PostQuitMessage(0);
}

const com_ptr<RootPage>& App::RootPage() const noexcept {
assert(_mainWindow && *_mainWindow);
return _mainWindow->Content();
}

void App::_Uninitialize() {
Logger::Get().Info("App uninitialized");
}

void App::_UpdateTheme() {
bool isLightTheme = true;
UISettings uiSettings;
auto color = uiSettings.GetColorValue(UIColorType::Foreground);
// If foreground is light, the theme is dark
isLightTheme = !(5 * color.G + 2 * color.R + color.B > 8 * 128);

if (_isLightTheme != isLightTheme) {
_isLightTheme = isLightTheme;
ThemeChanged.Invoke(isLightTheme);
}
}

}
