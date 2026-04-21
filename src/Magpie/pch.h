#pragma once

// Windows headers
#include <SDKDDKVer.h>
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

// Avoid C++/WinRT header warnings
#undef GetCurrentTime
#undef GetNextSibling

// C++ runtime
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <functional>
#include <span>
#include <filesystem>

// WIL
#define WIL_ENABLE_EXCEPTIONS
#include <wil/result_macros.h>
#undef WIL_ENABLE_EXCEPTIONS
#include <wil/resource.h>
#include <wil/win32_helpers.h>
#include <wil/filesystem.h>

// C++/WinRT
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>

#include <wil/cppwinrt_authoring.h>

namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Hosting;

namespace MUXC = Microsoft::UI::Xaml::Controls;
}

#include "CommonDefines.h"
