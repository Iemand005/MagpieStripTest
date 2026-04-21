#include "pch.h"
#include "BackendDescriptorStore.h"
#include "Logger.h"

namespace Magpie {

ID3D11ShaderResourceView* BackendDescriptorStore::GetShaderResourceView(ID3D11Texture2D* texture) noexcept {

	return nullptr; //.emplace(texture, std::move(srv)).first->second.get();
}

ID3D11UnorderedAccessView* BackendDescriptorStore::GetUnorderedAccessView(ID3D11Texture2D* texture) noexcept {
	return nullptr; //.emplace(texture, std::move(uav)).first->second.get();
}

ID3D11UnorderedAccessView* BackendDescriptorStore::GetUnorderedAccessView(
	ID3D11Buffer* buffer,
	uint32_t numElements,
	DXGI_FORMAT format
) noexcept {

	return nullptr; //.emplace(buffer, std::move(uav)).first->second.get();
}

void BackendDescriptorStore::RemoveCache(ID3D11Texture2D* texture) noexcept {
	
}

}
