#include "pch.h"
#include "WindowHelper.h"
#include "StrHelper.h"
#include "Win32Helper.h"
 #include <parallel_hashmap/phmap.h>

namespace Magpie {

static const wchar_t* CoreWindowClassName = L"Windows.UI.Core.CoreWindow";

bool WindowHelper::IsStartMenu(HWND hWnd) noexcept {
	// 作为优化，首先检查窗口类
	std::wstring className = Win32Helper::GetWindowClassName(hWnd);

	if (className != CoreWindowClassName) {
		return false;
	}

	// 检查可执行文件名称
	std::wstring exeName = Win32Helper::GetWindowExeName(hWnd);
	StrHelper::ToLowerCase(exeName);
	// win10: searchapp.exe 和 startmenuexperiencehost.exe
	// win11: searchhost.exe 和 startmenuexperiencehost.exe
	return exeName == L"searchapp.exe" || exeName == L"searchhost.exe" ||
		exeName == L"startmenuexperiencehost.exe";
}

bool WindowHelper::IsForbiddenSystemWindow(HWND hwndSrc) noexcept {
	// (可执行文件名, 类名)
	return false;
}

}
