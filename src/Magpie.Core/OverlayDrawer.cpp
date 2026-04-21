#include "pch.h"
#include "OverlayDrawer.h"
#include "CursorManager.h"
#include "DeviceResources.h"
#include "EffectDesc.h"
#include "FrameSourceBase.h"
#include "ImGuiFontsCacheManager.h"
#include "Logger.h"
#include "OverlayHelper.h"
#include "Renderer.h"
#include "ScalingWindow.h"
#include "StrHelper.h"
#include "Win32Helper.h"
#include <ShlObj.h>

using namespace std::chrono;

namespace Magpie {

static const char* COLOR_INDICATOR = "■";
static const wchar_t COLOR_INDICATOR_W = L'■';

static const float CORNER_ROUNDING = 6;

static const char* TOOLBAR_WINDOW_ID = "toolbar";
static const char* PROFILER_WINDOW_ID = "profiler";

static void SetDefaultWindowOptions(
	// phmap::flat_hash_map<std::string, OverlayWindowOption>& windowOptions
) noexcept {
}

bool OverlayDrawer::Initialize(DeviceResources& deviceResources, OverlayOptions& overlayOptions) noexcept {
	return true;
}

void OverlayDrawer::Draw(
	uint32_t count,
	uint32_t fps,
	const SmallVector<float>& effectTimings,
	POINT drawOffset
) noexcept {
}

ToolbarState OverlayDrawer::ToolbarState() const noexcept {
	if (!_isToolbarVisible) {
		return ToolbarState::Off;
	} else {
		return _isToolbarPinned ? ToolbarState::AlwaysShow : ToolbarState::AutoHide;
	}
}

void OverlayDrawer::ToolbarState(Magpie::ToolbarState value) noexcept {
	if (ToolbarState() == value) {
		return;
	}

	if (value == ToolbarState::Off) {
		_isToolbarVisible = false;
		_ClearStatesIfNoVisibleWindow();
	} else if (value == ToolbarState::AlwaysShow) {
		_isToolbarVisible = true;
		_isToolbarPinned = true;
	} else {
		_isToolbarVisible = true;
		_isToolbarPinned = false;
	}
}

bool OverlayDrawer::AnyVisibleWindow() const noexcept {
	bool result = _isToolbarVisible || _isProfilerVisible;
#ifdef _DEBUG
	result = result || _isDemoWindowVisible;
#endif
	return result;
}

void OverlayDrawer::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	if (AnyVisibleWindow()) {
		_imguiImpl.MessageHandler(msg, wParam, lParam);
	}
}

bool OverlayDrawer::NeedRedraw(uint32_t fps) const noexcept {
	if (!AnyVisibleWindow()) {
		return false;
	}

	if (_CalcToolbarAlpha() != _lastToolbarAlpha) {
		return true;
	}

	return _lastFPS != fps && (_lastToolbarAlpha > FLOAT_EPSILON<float> || _isProfilerVisible);
}

void OverlayDrawer::UpdateAfterActiveEffectsChanged() noexcept {
	const std::vector<const EffectDesc*>& effectDescs =
		ScalingWindow::Get().Renderer().ActiveEffectDescs();
	_timelineColors = OverlayHelper::GenerateTimelineColors(effectDescs);

	uint32_t passCount = 0;
	for (const EffectDesc* info : effectDescs) {
		passCount += (uint32_t)info->passes.size();
	}

	// 清空旧时间数据
	_effectTimingsStatistics.clear();
	_effectTimingsStatistics.resize(passCount);
	_lastestAvgEffectTimings.clear();
	_lastestAvgEffectTimings.resize(passCount);
	_lastUpdateTime = {};
}

static const std::wstring& GetAppLanguage() noexcept {
	static std::wstring language;
	if (language.empty()) {
		winrt::ResourceContext resourceContext = winrt::ResourceContext::GetForViewIndependentUse();
		language = resourceContext.QualifierValues().Lookup(L"Language");
		StrHelper::ToLowerCase(language);
	}
	return language;
}

static const std::wstring& GetSystemFontsFolder() noexcept {
	static std::wstring result;

	if (result.empty()) {
		wil::unique_cotaskmem_string fontsFolder;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Fonts, 0, NULL, fontsFolder.put());
		if (FAILED(hr)) {
			Logger::Get().ComError("SHGetKnownFolderPath 失败", hr);
			return result;
		}

		result = fontsFolder.get();
	}

	return result;
}

bool OverlayDrawer::_BuildFonts() noexcept {

	return true;
}

template <size_t SIZE>
static void SetGlyphRanges(SmallVector<ImWchar>& uiRanges, const ImWchar (&ranges)[SIZE]) noexcept {
	uiRanges.assign(std::begin(ranges), std::end(ranges));
}

// 指针重载，但不能直接使用指针
template <typename T, typename = std::enable_if_t<std::is_same_v<T, const ImWchar*>>>
static void SetGlyphRanges(SmallVector<ImWchar>& uiRanges, T ranges) noexcept {
	// 删除末尾的 0
	for (const ImWchar* range = ranges; *range; ++range) {
		uiRanges.push_back(*range);
	}
}

SmallVector<ImWchar> OverlayDrawer::_BuildFontUI(
	std::wstring_view language,
	const std::vector<uint8_t>& fontData
) noexcept {
	SmallVector<ImWchar> ranges;
	return ranges;
}

void OverlayDrawer::_BuildFontIcons(const char* fontPath) noexcept {
	ImFontConfig config;
#ifdef _DEBUG
	std::char_traits<char>::copy(config.Name, "_fontIcons", std::size(config.Name));
#endif

	const float fontSize = 16 * _dpiScale;
	_fontIcons = ImGui::GetIO().Fonts->AddFontFromFileTTF(
		fontPath, fontSize, &config, OverlayHelper::ICON_RANGES);
}

static std::string_view GetEffectDisplayName(const EffectDesc& effectDesc) noexcept {
	auto delimPos = effectDesc.name.find_last_of('\\');
	if (delimPos == std::string::npos) {
		return effectDesc.name;
	} else {
		return std::string_view(effectDesc.name.begin() + delimPos + 1, effectDesc.name.end());
	}
}

bool OverlayDrawer::_DrawTimingItem(
	int& itemId,
	const char* text,
	const ImColor* color,
	float time,
	bool isExpanded
) const noexcept {
	return true;
}

// 返回鼠标悬停的项的序号，未悬停于任何项返回 -1
int OverlayDrawer::_DrawEffectTimings(
	int& itemId,
	const _EffectDrawInfo& drawInfo,
	bool showPasses,
	std::span<const ImColor> colors,
	bool singleEffect
) const noexcept {
	int result = -1;

	showPasses &= drawInfo.passTimings.size() > 1;

	std::string effectName;
	if (ScalingWindow::Get().Options().IsDeveloperMode() && drawInfo.desc->flags & EffectFlags::FP16) {
		// 开发者选项开启时显示效果是否使用 FP16
		effectName = StrHelper::Concat(GetEffectDisplayName(*drawInfo.desc), " (FP16)");
	} else {
		effectName = std::string(GetEffectDisplayName(*drawInfo.desc));
	}
	
	if (_DrawTimingItem(
		itemId,
		effectName.c_str(),
		(!singleEffect && !showPasses) ? &colors[0] : nullptr,
		drawInfo.totalTime,
		showPasses
	)) {
		result = 0;
	}

	if (showPasses) {
		for (size_t j = 0; j < drawInfo.passTimings.size(); ++j) {
			ImGui::Indent(16);

			if (_DrawTimingItem(
				itemId,
				drawInfo.desc->passes[j].desc.c_str(),
				&colors[j],
				drawInfo.passTimings[j]
			)) {
				result = (int)j;
			}

			ImGui::Unindent(16);
		}
	}

	return result;
}

void OverlayDrawer::_DrawTimelineItem(
	int& itemId,
	ImU32 color,
	float dpiScale,
	std::string_view name,
	float time,
	float effectsTotalTime,
	bool selected
) {
	ImGui::PushID(itemId++);

	ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, color);
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, color);
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, color);
	ImGui::PushStyleColor(ImGuiCol_Header, color);
	ImGui::Selectable("", selected);
	ImGui::PopStyleColor(3);

	if (ImGui::IsItemHovered() || ImGui::IsItemClicked()) {
		std::string content = "";
		ImGui::PushFont(_fontMonoNumbers);
		_imguiImpl.Tooltip(content.c_str(), _dpiScale, nullptr, 500 * dpiScale);
		ImGui::PopFont();
	}

	// 空间足够时显示文字
	std::string text;
	if (selected) {
		text = "" ;
	} else {
		text.assign(name);
	}

	float textWidth = ImGui::CalcTextSize(text.c_str()).x;
	float itemWidth = ImGui::GetItemRectSize().x;
	float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
	if (itemWidth - (selected ? 0 : itemSpacing) > textWidth + 4 * _dpiScale) {
		ImGui::SameLine(0, 0);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (itemWidth - textWidth - itemSpacing) / 2);
		// 竖直方向居中
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 0.5f * _dpiScale);
		ImGui::TextUnformatted(text.c_str());
	}

	ImGui::PopID();
}

static std::string IconLabel(ImWchar iconChar) noexcept {
	const wchar_t text[] = { iconChar, L'\0' };
	return StrHelper::UTF16ToUTF8(text);
}

bool OverlayDrawer::_DrawToolbar(uint32_t fps, int& itemId) noexcept {
	bool needRedraw = false;

	const float windowWidth = 360 * _dpiScale;
	ImGui::SetNextWindowSize({ windowWidth, (CORNER_ROUNDING + 31) * _dpiScale });
	ImGui::SetNextWindowPos(
		ImVec2((ImGui::GetIO().DisplaySize.x - windowWidth) / 2, -CORNER_ROUNDING * _dpiScale));

	_lastToolbarAlpha = _CalcToolbarAlpha();
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, _lastToolbarAlpha);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImU32)ImColor(15, 15, 15, 180));
	const ImVec2 originalWindowPadding = ImGui::GetStyle().WindowPadding;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 6 * _dpiScale,0.0f });

	_isToolbarItemActive = false;

	if (ImGui::Begin(StrHelper::Concat("##", TOOLBAR_WINDOW_ID).c_str(), nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		// 通过工具栏拖拽缩放窗口时不要更新 _isCursorOnCaptionArea
		if (!ScalingWindow::Get().IsResizingOrMoving()) {
			// 鼠标被 ImGui 捕获时禁止拖拽缩放窗口
			_isCursorOnCaptionArea = !ImGui::IsAnyMouseDown();
			if (_isCursorOnCaptionArea) {
				// 检查鼠标是否被其他窗口遮挡
				const char* hoveredWindowId = _imguiImpl.GetHoveredWindowId();
				_isCursorOnCaptionArea = hoveredWindowId &&
					hoveredWindowId == std::string_view(TOOLBAR_WINDOW_ID);
			}
		}

		ImGui::SetCursorPosY((CORNER_ROUNDING + 3) * _dpiScale);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, originalWindowPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4 * _dpiScale,4 * _dpiScale });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4 * _dpiScale);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4 * _dpiScale, 0.0f });
		// 禁用仅为阻止交互，不应有视觉改变
		ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.118f, 0.533f, 0.894f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.118f, 0.533f, 0.894f, 0.8f });

		auto drawToggleButton = [&](bool& value, ImWchar icon, const char* tooltip) {
			bool stylePushed = value;
			if (stylePushed) {
				ImGui::PushStyleColor(ImGuiCol_Button, { 0.118f, 0.533f, 0.894f, 0.8f });
			}

			ImGui::PushFont(_fontIcons);
			if (ImGui::Button(IconLabel(icon).c_str())) {
				value = !value;
				needRedraw = true;
			}
			if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
				_isCursorOnCaptionArea = false;
				_isToolbarItemActive = true;
			}
			ImGui::PopFont();
			if (ImGui::IsItemHovered() || ImGui::IsItemClicked()) {
				_imguiImpl.Tooltip(tooltip, _dpiScale);
			}

			if (stylePushed) {
				ImGui::PopStyleColor();
			}
		};

		auto drawButton = [&](ImWchar icon, const char* tooltip, const char* description = nullptr) {
			ImGui::PushFont(_fontIcons);
			const bool clicked = ImGui::Button(IconLabel(icon).c_str());
			if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
				_isCursorOnCaptionArea = false;
				_isToolbarItemActive = true;
			}
			ImGui::PopFont();
			if (ImGui::IsItemHovered()) {
				_imguiImpl.Tooltip(tooltip, _dpiScale, description);
			}
			return clicked;
		};

		// 光标不在缩放窗口上时阻止交互
		ImGui::BeginDisabled(!ScalingWindow::Get().CursorManager().CursorHandle());

		const std::string& pinStr = _GetResourceString(L"Overlay_Toolbar_Pin");
		drawToggleButton(_isToolbarPinned, OverlayHelper::SegoeIcons::Pinned, pinStr.c_str());
		ImGui::SameLine();
		const std::string& profilerStr = _GetResourceString(L"Overlay_Toolbar_Profiler");
		drawToggleButton(_isProfilerVisible, OverlayHelper::SegoeIcons::Diagnostic, profilerStr.c_str());
#ifdef _DEBUG
		ImGui::SameLine();
		const std::string& demoStr = _GetResourceString(L"Overlay_Toolbar_Demo");
		drawToggleButton(_isDemoWindowVisible, OverlayHelper::SegoeIcons::Design, demoStr.c_str());
#endif
		ImGui::SameLine();
		const std::string& screenshotStr = _GetResourceString(L"Overlay_Toolbar_TakeScreenshot");
		const std::string& screenshotDescStr = _GetResourceString(L"Overlay_Toolbar_TakeScreenshot_Description");
		if (drawButton(OverlayHelper::SegoeIcons::Camera, screenshotStr.c_str(), screenshotDescStr.c_str())) {
			const std::vector<const EffectDesc*>& effectDescs =
				ScalingWindow::Get().Renderer().ActiveEffectDescs();
			ScalingWindow::Get().Renderer().TakeScreenshot((uint32_t)effectDescs.size() - 1);
		}
		// 截图按钮右键菜单
		if (ImGui::BeginPopupContextItem()) {
			_isCursorOnCaptionArea = false;
			_isToolbarItemActive = true;

			ImGui::SeparatorText(_GetResourceString(L"Overlay_Toolbar_TakeScreenshot_PopupTitle").c_str());
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f * _dpiScale);
			ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 6.0f * _dpiScale);

			const std::vector<const EffectDesc*>& effectDescs =
				ScalingWindow::Get().Renderer().ActiveEffectDescs();
			const uint32_t effectCount = (uint32_t)effectDescs.size();

			const bool isDeveloperMode = ScalingWindow::Get().Options().IsDeveloperMode();
			for (uint32_t i = 0; i < effectCount; ++i) {
				const EffectDesc& effectDesc = *effectDescs[i];
				std::string_view effectName = GetEffectDisplayName(effectDesc);
			
				if (isDeveloperMode && effectDesc.passes.size() > 1) {
					// 开发者模式允许保存任意通道的输出
					ImGui::PushID(itemId++);
					if (ImGui::BeginMenu(effectName.data())) {
						const uint32_t passCount = (uint32_t)effectDesc.passes.size();
						for (uint32_t j = 0; j < passCount; ++j) {
							const EffectPassDesc& passDesc = effectDesc.passes[j];
							const uint32_t outputCount = (uint32_t)passDesc.outputs.size();

							if (outputCount == 1) {
								ImGui::PushID(itemId++);
								if (ImGui::MenuItem(passDesc.desc.c_str())) {
									ScalingWindow::Get().Renderer().TakeScreenshot(i, j);
								}
								ImGui::PopID();
							} else {
								ImGui::PushID(itemId++);
								if (ImGui::BeginMenu(passDesc.desc.c_str())) {
									for (uint32_t k = 0; k < outputCount; ++k) {
										ImGui::PushID(itemId++);
										if (ImGui::MenuItem(effectDesc.textures[passDesc.outputs[k]].name.c_str())) {
											ScalingWindow::Get().Renderer().TakeScreenshot(i, j, k);
										}
										ImGui::PopID();
									}

									ImGui::EndMenu();
								}
								ImGui::PopID();
							}
						}

						ImGui::EndMenu();
					}
					ImGui::PopID();
				} else {
					ImGui::PushID(itemId++);
					if (ImGui::MenuItem(effectName.data())) {
						ScalingWindow::Get().Renderer().TakeScreenshot(i);
					}
					ImGui::PopID();
				}
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}

		// 居中绘制 FPS
		ImGui::SameLine();
		const std::string fpsText = "";
		ImGui::SetCursorPosX((ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(fpsText.c_str()).x) / 2);
		ImGui::SetCursorPosY((CORNER_ROUNDING + 1) * _dpiScale);
		ImGui::PushFont(_fontMonoNumbers);
		ImGui::TextUnformatted(fpsText.c_str());
		ImGui::PopFont();

		ImGui::SameLine();
		ImGui::SetCursorPosY((CORNER_ROUNDING + 3) * _dpiScale);

		// 源窗口支持最小化时才显示最小化按钮
		const HWND hwndSrc = ScalingWindow::Get().SrcTracker().Handle();
		const bool canSrcMinimized = GetWindowStyle(hwndSrc) & WS_MINIMIZEBOX;
		ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x -
			((canSrcMinimized ? 3 : 2) * 28 - 4) * _dpiScale);

		if (canSrcMinimized) {
			const std::string& minimizeStr = _GetResourceString(L"Overlay_Toolbar_Minimize");
			const ImWchar icon = Win32Helper::GetOSVersion().IsWin11() ?
				OverlayHelper::SegoeIcons::CheckboxIndeterminate : OverlayHelper::SegoeIcons::Remove;
			if (drawButton(icon, minimizeStr.c_str())) {
				// 模拟通过标题栏最小化，失败则回落到 ShowWindow
				if (!PostMessage(hwndSrc, WM_SYSCOMMAND, SC_MINIMIZE, 0)) {
					ShowWindowAsync(hwndSrc, SW_SHOWMINIMIZED);
				}
			}
			ImGui::SameLine();
		}

		{
			const bool isWindowedMode = ScalingWindow::Get().Options().IsWindowedMode();
			const ImWchar icon = isWindowedMode ?
				OverlayHelper::SegoeIcons::FullScreen : OverlayHelper::SegoeIcons::Favicon;
			const std::string& switchScalingStr = _GetResourceString(
				isWindowedMode ? L"Overlay_Toolbar_SwitchToFullscreen" : L"Overlay_Toolbar_SwitchToWindowed");
			if (drawButton(icon, switchScalingStr.c_str())) {
				ScalingWindow::Dispatcher().TryEnqueue([]() {
					ScalingWindow::Get().ToggleScaling(!ScalingWindow::Get().Options().IsWindowedMode());
				});
			}
		}
		ImGui::SameLine();

		// 和主窗口保持一致 (#C42B1C)
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.769f, 0.169f, 0.11f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.769f, 0.169f, 0.11f, 0.8f });

		const std::string& closeStr = _GetResourceString(L"Overlay_Toolbar_Close");
		const std::string& closeDescStr = _GetResourceString(L"Overlay_Toolbar_Close_Description");
		if (drawButton(OverlayHelper::SegoeIcons::Cancel, closeStr.c_str(), closeDescStr.c_str())) {
			ScalingWindow::Dispatcher().TryEnqueue([]() {
				ScalingWindow::Get().Stop();
			});
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			ScalingWindow::Dispatcher().TryEnqueue([this, runId(ScalingWindow::RunId())]() {
				if (runId == ScalingWindow::RunId()) {
					ToolbarState(ToolbarState::Off);
					ScalingWindow::Get().Renderer().Render(true);
				}
			});
		}

		ImGui::EndDisabled();

		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar(6);
	} else {
		_isCursorOnCaptionArea = false;
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
	
	return needRedraw;
}

#ifdef MP_DEBUG_INFO_ON_OVERLAY
static std::string RectToStr(const RECT& rect) noexcept {
	return fmt::format("{},{},{},{} ({}x{})",
		rect.left, rect.top, rect.right, rect.bottom,
		rect.right - rect.left, rect.bottom - rect.top);
}
#endif

// 返回 true 表示应再渲染一次
bool OverlayDrawer::_DrawProfiler(const SmallVector<float>& effectTimings, uint32_t fps, int& itemId) noexcept {
	return false;
}

const std::string& OverlayDrawer::_GetResourceString(const std::wstring_view& key) noexcept {
	
	return "";
}

float OverlayDrawer::_CalcToolbarAlpha() const noexcept {
	return 1.0f;
}

void OverlayDrawer::_ClearStatesIfNoVisibleWindow() noexcept {
	if (AnyVisibleWindow()) {
		return;
	}

	_imguiImpl.ClearStates();
	_isCursorOnCaptionArea = false;
	_isToolbarItemActive = false;
}

}
