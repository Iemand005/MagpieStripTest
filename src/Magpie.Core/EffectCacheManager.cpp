#include "pch.h"
#include "EffectCacheManager.h"
#include "CommonSharedConstants.h"
#include "Logger.h"
#include "StrHelper.h"
#include "Win32Helper.h"
#include "YasHelper.h"
#include <d3dcompiler.h>
#include <rapidhash.h>

namespace yas::detail {

// winrt::com_ptr<ID3DBlob>
template <std::size_t F>
struct serializer<
	type_prop::not_a_fundamental,
	ser_case::use_internal_serializer,
	F,
	winrt::com_ptr<ID3DBlob>
> {
	template <typename Archive>
	static Archive& save(Archive& ar, const winrt::com_ptr<ID3DBlob>& blob) {
		uint32_t size = (uint32_t)blob->GetBufferSize();
		ar& size;

		ar.write(blob->GetBufferPointer(), size);

		return ar;
	}

	template <typename Archive>
	static Archive& load(Archive& ar, winrt::com_ptr<ID3DBlob>& blob) {
		uint32_t size = 0;
		ar& size;
		HRESULT hr = D3DCreateBlob(size, blob.put());
		if (FAILED(hr)) {
			Logger::Get().ComError("D3DCreateBlob 失败", hr);
			throw new std::exception();
		}

		ar.read(blob->GetBufferPointer(), size);

		return ar;
	}
};

}

namespace Magpie {

template <typename Archive>
void serialize(Archive& ar, EffectParameterDesc& o) {
	ar& o.name& o.label& o.constant;
}

template <typename Archive>
void serialize(Archive& ar, EffectIntermediateTextureDesc& o) {
	ar& o.format& o.name& o.source& o.sizeExpr;
}

template <typename Archive>
void serialize(Archive& ar, EffectSamplerDesc& o) {
	ar& o.filterType& o.addressType& o.name;
}

template <typename Archive>
void serialize(Archive& ar, EffectPassDesc& o) {
	ar& o.cso& o.inputs& o.outputs& o.numThreads[0] & o.numThreads[1] & o.numThreads[2] & o.blockSize& o.desc& o.flags;
}

template <typename Archive>
void serialize(Archive& ar, EffectDesc& o) {
	ar& o.name& o.params& o.textures& o.samplers& o.passes& o.flags;
}

static constexpr uint32_t MAX_CACHE_COUNT = 127;

// 缓存版本
// 当缓存文件结构有更改时更新它，使旧缓存失效
static constexpr uint32_t EFFECT_CACHE_VERSION = 15;


static std::wstring GetLinearEffectName(std::wstring_view effectName) {
	std::wstring result(effectName);
	for (wchar_t& c : result) {
		if (c == L'\\') {
			c = L'#';
		}
	}
	return result;
}

static std::wstring GetCacheFileName(std::wstring_view linearEffectName, uint32_t flags, uint64_t hash) {
	assert(flags <= 0xFFFF);
	// 缓存文件的命名: {效果名}_{标志位(4)}_{哈希(16)）}
	return L"";
}

void EffectCacheManager::_AddToMemCache(const std::wstring& cacheFileName, std::string& key, const EffectDesc& desc) {
	
}

bool EffectCacheManager::_LoadFromMemCache(const std::wstring& cacheFileName, std::string_view key, EffectDesc& desc) {
	
	return false;
}

bool EffectCacheManager::Load(
	std::wstring_view effectName,
	uint32_t flags,
	uint64_t hash,
	std::string_view key,
	EffectDesc& desc
) {
	
	return true;
}

void EffectCacheManager::Save(
	std::wstring_view effectName,
	uint32_t flags,
	uint64_t hash,
	std::string key,
	const EffectDesc& desc
) {
	
}

uint64_t EffectCacheManager::GetHash(std::string_view key) {
	return rapidhash(key.data(), key.size());
}

}
