#pragma once
#include "RootPage.h"
#include "XamlWindow.h"

namespace Magpie {

class MainWindow final : public XamlWindowT<MainWindow, winrt::com_ptr<winrt::Magpie::implementation::RootPage>> {
using base_type = XamlWindowT<MainWindow, winrt::com_ptr<winrt::Magpie::implementation::RootPage>>;
friend WindowBaseT<MainWindow>;
public:
bool Create() noexcept;

void Show() const noexcept;

protected:
LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
std::pair<POINT, SIZE> _CreateWindow() noexcept;

void _UpdateTheme() noexcept;

::Magpie::MultithreadEvent<bool>::EventRevoker _appThemeChangedRevoker;
};

}
