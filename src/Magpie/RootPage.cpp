#include "pch.h"
#include "RootPage.h"
#if __has_include("RootPage.g.cpp")
#include "RootPage.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::Magpie::implementation {

RootPage::RootPage() {
}

RootPage::~RootPage() {
}

void RootPage::Button_Click(IInspectable const&, RoutedEventArgs const&) {
// Button clicked!
}

}
