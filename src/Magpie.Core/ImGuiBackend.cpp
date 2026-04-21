#include "pch.h"
#include "ImGuiBackend.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"
#include "Logger.h"
#include "shaders/ImGuiImplPS.h"
#include "shaders/ImGuiImplVS.h"
#include <imgui.h>

namespace Magpie {

struct VERTEX_CONSTANT_BUFFER {
	float mvp[4][4];
};

bool ImGuiBackend::Initialize(DeviceResources& deviceResources) noexcept {
	_deviceResources = &deviceResources;

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "Magpie";
	// 支持 ImDrawCmd::VtxOffset
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	if (!_CreateDeviceObjects()) {
		Logger::Get().Error("_CreateDeviceObjects 失败");
		return false;
	}

	return true;
}

void ImGuiBackend::_SetupRenderState(const void * drawData, POINT viewportOffset) noexcept {
}

void ImGuiBackend::RenderDrawData(const void * drawData, POINT viewportOffset) noexcept {
}

bool ImGuiBackend::_CreateDeviceObjects() noexcept {

	return true;
}

bool ImGuiBackend::BuildFonts() noexcept {
	return true;
}

}
