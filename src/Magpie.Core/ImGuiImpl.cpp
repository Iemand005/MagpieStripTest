#include "pch.h"
#include "ImGuiImpl.h"
#include "CursorManager.h"
#include "DeviceResources.h"
#include "ImGuiBackend.h"
#include "Logger.h"
#include "Renderer.h"
#include "ScalingWindow.h"
#include "StrHelper.h"
#include "Win32Helper.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>

namespace Magpie {

static bool operator==(const ImVec2& l, const ImVec2& r) noexcept {
	return l.x == r.x && l.y == r.y;
}

static bool operator==(const ImVec4& l, const ImVec4& r) noexcept {
	return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
}

static const char* GetWindowIDFromName(const char* name) noexcept {
	size_t idPos = std::string_view(name).find("##");
	if (idPos == std::string_view::npos) {
		return name;
	} else {
		return name + idPos + 2;
	}
}

ImGuiImpl::~ImGuiImpl() noexcept {
	if (ImGui::GetCurrentContext()) {
		ImGui::DestroyContext();
	}
}

bool ImGuiImpl::Initialize(DeviceResources& deviceResources) noexcept {
	return true;
}

bool ImGuiImpl::BuildFonts() noexcept {
	return _backend.BuildFonts();
}

void ImGuiImpl::NewFrame(
	// phmap::flat_hash_map<std::string, OverlayWindowOption>& windowOptions,
	float fittsLawAdjustment,
	float dpiScale
) noexcept {
}

void ImGuiImpl::Draw(POINT drawOffset) noexcept {
}

void ImGuiImpl::Tooltip(
	const char* content,
	float dpiScale,
	const char* description,
	float maxWidth
) noexcept {
}

void ImGuiImpl::_UpdateMousePos(float fittsLawAdjustment) noexcept {
}

void ImGuiImpl::ClearStates() noexcept {
}

void ImGuiImpl::MessageHandler(UINT msg, WPARAM wParam, LPARAM /*lParam*/) noexcept {
}

std::optional<ImVec4> ImGuiImpl::GetWindowRect(const char* id) const noexcept {
	return std::nullopt;
}

const char* ImGuiImpl::GetHoveredWindowId() const noexcept {
	return nullptr;
}

}
