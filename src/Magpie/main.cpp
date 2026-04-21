#include "pch.h"
#include "App.h"
#include "Win32Helper.h"
#include "CommonSharedConstants.h"
#include "Logger.h"

using namespace Magpie;
using namespace winrt::Magpie::implementation;

// Set working directory to the program directory
static void SetWorkingDir() noexcept {
FAIL_FAST_IF_WIN32_BOOL_FALSE(SetCurrentDirectory(
Win32Helper::GetExePath().parent_path().c_str()));
}

static void InitializeLogger(const wchar_t* logFilePath) noexcept {
// Max 2 log files, each max 500KB
Logger::Get().Initialize(
spdlog::level::info,
logFilePath,
CommonSharedConstants::LOG_MAX_SIZE,
1
);
}

int APIENTRY wWinMain(
_In_ HINSTANCE /*hInstance*/,
_In_opt_ HINSTANCE /*hPrevInstance*/,
_In_ wchar_t* /*lpCmdLine*/,
_In_ int /*nCmdShow*/
) {
#ifdef _DEBUG
SetThreadDescription(GetCurrentThread(), L"Magpie-MainThread");
#endif

// Terminate process on heap corruption
HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, nullptr, 0);

SetWorkingDir();
InitializeLogger(CommonSharedConstants::LOG_PATH);

Logger::Get().Info("Application starting");

// Do not call uninit_apartment on exit
// See https://kennykerr.ca/2018/03/24/cppwinrt-hosting-the-windows-runtime/
winrt::init_apartment(winrt::apartment_type::single_threaded);

auto& app = App::Get();
if (!app.Initialize(nullptr)) {
return 0;
}

return app.Run();
}