#pragma once
#include "App.g.h"
#include "Event.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace Magpie {
class MainWindow;
}

namespace winrt::Magpie::implementation {

struct RootPage;

class App : public App_base<App, Markup::IXamlMetadataProvider> {
public:
static App& Get();

App();
App(const App&) = delete;
App(App&&) = delete;

bool Initialize(const wchar_t* arguments);

int Run();

const DispatcherQueue& Dispatcher() const noexcept {
return _dispatcher;
}

void ShowMainWindow() noexcept;

void Quit();

const com_ptr<RootPage>& RootPage() const noexcept;

const ::Magpie::MainWindow& MainWindow() const noexcept {
return *_mainWindow;
}

::Magpie::MainWindow& MainWindow() noexcept {
return *_mainWindow;
}

bool IsLightTheme() const noexcept {
return _isLightTheme;
}

::Magpie::MultithreadEvent<bool> ThemeChanged;

private:
void _Uninitialize();

void _UpdateTheme();

Hosting::WindowsXamlManager _windowsXamlManager{ nullptr };

std::unique_ptr<::Magpie::MainWindow> _mainWindow;

DispatcherQueue _dispatcher{ nullptr };

bool _isLightTheme = true;

////////////////////////////////////////////////////
// 
// IXamlMetadataProvider related
// 
/////////////////////////////////////////////////////
public:
Markup::IXamlType GetXamlType(Interop::TypeName const& type) {
return _AppProvider()->GetXamlType(type);
}

Markup::IXamlType GetXamlType(hstring const& fullName) {
return _AppProvider()->GetXamlType(fullName);
}

com_array<Markup::XmlnsDefinition> GetXmlnsDefinitions() {
return _AppProvider()->GetXmlnsDefinitions();
}

private:
com_ptr<XamlMetaDataProvider> _AppProvider() {
if (!_appProvider) {
_appProvider = make_self<XamlMetaDataProvider>();
}
return _appProvider;
}

com_ptr<XamlMetaDataProvider> _appProvider;
};

}

BASIC_FACTORY(App)
