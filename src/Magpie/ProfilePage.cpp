#include "pch.h"
#include "ProfilePage.h"
#if __has_include("ProfilePage.g.cpp")
#include "ProfilePage.g.cpp"
#endif
#include "App.h"
#include "ControlHelper.h"
#include "Profile.h"

using namespace ::Magpie;
using namespace winrt;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Input;

namespace winrt::Magpie::implementation {

void ProfilePage::OnNavigatedTo(Navigation::NavigationEventArgs const& args) {
	
}

void ProfilePage::ComboBox_DropDownOpened(IInspectable const& sender, IInspectable const&) {
}

void ProfilePage::NumberBox_Loaded(IInspectable const& sender, RoutedEventArgs const&) {
}

void ProfilePage::InitialWindowedScaleFactorComboBox_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&) {
	
}

void ProfilePage::CursorScalingComboBox_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&) {
	
}

void ProfilePage::RenameMenuItem_Click(IInspectable const&, RoutedEventArgs const&) {
}

void ProfilePage::RenameFlyout_Opening(IInspectable const&, IInspectable const&) {
	
}

void ProfilePage::RenameConfirmButton_Click(IInspectable const&, RoutedEventArgs const&) {
	
}

void ProfilePage::RenameTextBox_KeyDown(IInspectable const&, Input::KeyRoutedEventArgs const& args) {
	
}

void ProfilePage::ReorderMenuItem_Click(IInspectable const&, RoutedEventArgs const&) {
}

void ProfilePage::DeleteMenuItem_Click(IInspectable const&, RoutedEventArgs const&) {
}

void ProfilePage::DeleteButton_Click(IInspectable const&, RoutedEventArgs const&) {
	
}

void ProfilePage::LaunchParametersTextBox_KeyDown(IInspectable const&, Input::KeyRoutedEventArgs const& args) {
	
}

}
