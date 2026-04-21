#include "pch.h"
#include "EffectCompiler.h"
#include "CommonSharedConstants.h"
#include "DirectXHelper.h"
#include "EffectCacheManager.h"
#include "EffectDesc.h"
#include "EffectHelper.h"
#include "Logger.h"
#include "StrHelper.h"
#include "Win32Helper.h"
#include <bit>	// std::has_single_bit
#include <bitset>

namespace Magpie {

// 当前 MagpieFX 版本
static constexpr uint32_t MAGPIE_FX_VERSION = 4;

static const char* META_INDICATOR = "//!";

class PassInclude : public ID3DInclude {
public:
	PassInclude(std::wstring_view localDir) : _localDir(localDir) {}

	PassInclude(const PassInclude&) = default;
	PassInclude(PassInclude&&) = default;

	HRESULT CALLBACK Open(
		D3D_INCLUDE_TYPE /*IncludeType*/,
		LPCSTR pFileName,
		LPCVOID /*pParentData*/,
		LPCVOID* ppData,
		UINT* pBytes
	) noexcept override {
		std::wstring relativePath = StrHelper::Concat(_localDir, StrHelper::UTF8ToUTF16(pFileName));

		std::string file;
		if (!Win32Helper::ReadTextFile(relativePath.c_str(), file)) {
			return E_FAIL;
		}

		char* result = new char[file.size()];
		std::memcpy(result, file.data(), file.size());

		*ppData = result;
		*pBytes = (UINT)file.size();

		return S_OK;
	}

	HRESULT CALLBACK Close(LPCVOID pData) noexcept override {
		delete[](char*)pData;
		return S_OK;
	}

private:
	std::wstring _localDir;
};

static uint32_t RemoveComments(std::string& source) noexcept {
	// 确保以换行符结尾
	if (source.back() != '\n') {
		source.push_back('\n');
	}

	std::string result;
	result.reserve(source.size());

	int j = 0;
	// 单独处理最后两个字符
	for (size_t i = 0, end = source.size() - 2; i < end; ++i) {
		if (source[i] == '/') {
			if (source[i + 1] == '/' && source[i + 2] != '!') {
				// 行注释
				i += 2;

				// 无需处理越界，因为必定以换行符结尾
				while (source[i] != '\n') {
					++i;
				}

				// 保留换行符
				source[j++] = '\n';

				continue;
			} else if (source[i + 1] == '*') {
				// 块注释
				i += 2;

				while (true) {
					if (++i >= source.size()) {
						// 未闭合
						return 1;
					}

					if (source[i - 1] == '*' && source[i] == '/') {
						break;
					}
				}

				// 文件结尾
				if (i >= source.size() - 2) {
					source.resize(j);
					return 0;
				}

				continue;
			}
		}

		source[j++] = source[i];
	}

	// 无需复制最后的换行符
	source[j++] = source[source.size() - 2];
	source.resize(j);
	return 0;
}

template <bool IncludeNewLine>
static void RemoveLeadingBlanks(std::string_view& source) noexcept {
	size_t i = 0;
	for (; i < source.size(); ++i) {
		if constexpr (IncludeNewLine) {
			if (!StrHelper::isspace(source[i])) {
				break;
			}
		} else {
			char c = source[i];
			if (c != ' ' && c != '\t') {
				break;
			}
		}
	}

	source.remove_prefix(i);
}

template <bool AllowNewLine>
static bool CheckNextToken(std::string_view& source, std::string_view token) noexcept {
	RemoveLeadingBlanks<AllowNewLine>(source);

	if (!source.starts_with(token)) {
		return false;
	}

	source.remove_prefix(token.size());
	return true;
}

template <bool AllowNewLine>
static uint32_t GetNextToken(std::string_view& source, std::string_view& value) noexcept {
	RemoveLeadingBlanks<AllowNewLine>(source);

	if (source.empty()) {
		return 2;
	}

	char cur = source[0];

	if (StrHelper::isalpha(cur) || cur == '_') {
		size_t j = 1;
		for (; j < source.size(); ++j) {
			cur = source[j];

			if (!StrHelper::isalnum(cur) && cur != '_') {
				break;
			}
		}

		value = source.substr(0, j);
		source.remove_prefix(j);
		return 0;
	}

	if constexpr (AllowNewLine) {
		return 1;
	} else {
		return cur == '\n' ? 2 : 1;
	}
}

static bool CheckMagic(std::string_view& source) noexcept {

	return true;
}

static uint32_t GetNextString(std::string_view& source, std::string_view& value) noexcept {
	return 0;
}

template <typename T>
static uint32_t GetNextNumber(std::string_view& source, T& value) noexcept {
	return 0;
}

static uint32_t GetNextExpr(std::string_view& source, std::string& expr) noexcept {
	return 0;
}

static uint32_t ResolveUseFlags(std::string_view& block, uint32_t& effectFlags) noexcept {

	return 0;
}

static uint32_t ResolveCapabilityFlags(std::string_view& block, uint32_t& effectFlags, bool noFP16) noexcept {
	return 0;
}

static uint32_t ResolveHeader(
	std::string_view block,
	EffectDesc& desc,
	uint32_t& effectFlags,
	bool noCompile,
	bool noFP16
) noexcept {

	return 0;
}

static uint32_t ResolveParameter(std::string_view block, EffectDesc& desc) noexcept {

	return 0;
}


static uint32_t ResolveTexture(std::string_view block, EffectDesc& desc) noexcept {

	return 0;
}

static uint32_t ResolveSampler(std::string_view block, EffectDesc& desc) noexcept {

	return 0;
}

static uint32_t ResolveCommon(std::string_view& block) noexcept {
	return 0;
}

static uint32_t ResolvePasses(SmallVector<std::string_view>& blocks, EffectDesc& desc) noexcept {


	return 0;
}

static uint32_t GeneratePassSource(
	const EffectDesc& desc,
	uint32_t passIdx,
	std::string_view cbHlsl,
	const SmallVector<std::string_view>& commonBlocks,
	std::string_view passBlock,
	std::string& result,
	std::vector<std::pair<std::string, std::string>>& macros
) noexcept {

	return 0;
}

static uint32_t CompilePasses(
	EffectDesc& desc,
	uint32_t flags,
	const SmallVector<std::string_view>& commonBlocks,
	const SmallVector<std::string_view>& passBlocks,
	const // phmap::flat_hash_map<std::string, float>* inlineParams
) noexcept {
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 所有通道共用的常量缓冲区
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////////////


	return 0;
}

static std::string ReadEffectSource(const std::wstring& effectName) noexcept {
	std::wstring fileName = StrHelper::Concat(
		CommonSharedConstants::EFFECTS_DIR, L"\\", effectName, L".hlsl");

	std::string source;
	if (!Win32Helper::ReadTextFile(fileName.c_str(), source)) {
		Logger::Get().Error("读取源文件失败");
		return {};
	}
	return source;
}

uint32_t EffectCompiler::Compile(
	EffectDesc& desc,
	uint32_t flags,
	const void*
) noexcept {

	return 0;
}

}
