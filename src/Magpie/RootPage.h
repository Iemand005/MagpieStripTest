#pragma once
#include "RootPage.g.h"

namespace winrt::Magpie::implementation {

struct RootPage : RootPageT<RootPage> {
RootPage();
~RootPage();

void InitializeComponent();

void Button_Click(IInspectable const&, RoutedEventArgs const&);
};

}

namespace winrt::Magpie::factory_implementation {

struct RootPage : RootPageT<RootPage, implementation::RootPage> {
};

}
