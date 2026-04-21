#include "pch.h"
#include "EffectDrawer.h"
#include "BackendDescriptorStore.h"
#include "DeviceResources.h"
#include "DirectXHelper.h"
#include "EffectHelper.h"
#include "EffectsProfiler.h"
#include "Logger.h"
#include "ScalingOptions.h"
#include "ScalingWindow.h"
#include "StrHelper.h"
#include "TextureHelper.h"
#include "Win32Helper.h"

namespace Magpie {

EffectDrawer::~EffectDrawer() {
	// [0] 为输入，由前一个 EffectDrawer 管理
	const uint32_t textureCount = (uint32_t)_textures.size();
	for (uint32_t i = 1; i < textureCount; ++i) {
		_descriptorStore->RemoveCache(_textures[i].get());
	}
}

bool EffectDrawer::Initialize(
	const EffectDesc& desc,
	const EffectOption& option,
	DeviceResources& deviceResources,
	BackendDescriptorStore& descriptorStore,
	ID3D11Texture2D** inOutTexture
) noexcept {


	return true;
}

void EffectDrawer::Draw(EffectsProfiler& profiler) const noexcept {
	
}

void EffectDrawer::DrawForExport(const EffectDesc& desc, uint32_t passIdx) const noexcept {
	
}

bool EffectDrawer::ResizeTextures(
	const EffectDesc& desc,
	const EffectOption& option,
	DeviceResources& deviceResources,
	ID3D11Texture2D** inOutTexture
) noexcept {

	return true;
}

SIZE EffectDrawer::_CalcOutputSize(
	const EffectDesc& desc,
	const EffectOption& option,
	SIZE inputSize
) const noexcept {
	SIZE outputSize{};
	return outputSize;
}

bool EffectDrawer::_UpdatePassResources(const EffectDesc& desc) noexcept {
	const uint32_t passCount = (uint32_t)desc.passes.size();
	for (uint32_t i = 0; i < passCount; ++i) {
		const SmallVector<uint32_t>& inputs = desc.passes[i].inputs;
		const SmallVector<uint32_t>& outputs = desc.passes[i].outputs;
		const std::pair<uint32_t, uint32_t>& blockSize = desc.passes[i].blockSize;

		for (uint32_t j = 0; j < inputs.size(); ++j) {
			auto srv = _srvs[i][j] = _descriptorStore->GetShaderResourceView(_textures[inputs[j]].get());
			if (!srv) {
				Logger::Get().Error("GetShaderResourceView 失败");
				return false;
			}
		}

		for (uint32_t j = 0; j < outputs.size(); ++j) {
			auto uav = _uavs[i][j] = _descriptorStore->GetUnorderedAccessView(_textures[outputs[j]].get());
			if (!uav) {
				Logger::Get().Error("GetUnorderedAccessView 失败");
				return false;
			}
		}

		D3D11_TEXTURE2D_DESC outputDesc;
		_textures[outputs[0]]->GetDesc(&outputDesc);
		_dispatches[i] = {
			(outputDesc.Width + blockSize.first - 1) / blockSize.first,
			(outputDesc.Height + blockSize.second - 1) / blockSize.second
		};
	}

	return true;
}

bool EffectDrawer::_UpdateConstants(
	const EffectDesc& desc,
	const EffectOption& option,
	DeviceResources& deviceResources,
	SIZE inputSize,
	SIZE outputSize
) noexcept {
	const bool isInlineParams = desc.flags & EffectFlags::InlineParams;

	SmallVector<EffectHelper::Constant32, 32> constants;
	
	// 大小必须为 4 的倍数
	const size_t builtinConstantCount = 10;
	size_t psStylePassParams = 0;
	for (UINT i = 0, end = (UINT)desc.passes.size() - 1; i < end; ++i) {
		if (desc.passes[i].flags & EffectPassFlags::PSStyle) {
			psStylePassParams += 4;
		}
	}
	constants.resize((builtinConstantCount + psStylePassParams + (isInlineParams ? 0 : desc.params.size()) + 3) / 4 * 4);
	// cbuffer __CB1 : register(b0) {
	//     uint2 __inputSize;
	//     uint2 __outputSize;
	//     float2 __inputPt;
	//     float2 __outputPt;
	//     float2 __scale;
	//     [PARAMETERS...]
	// );
	constants[0].uintVal = inputSize.cx;
	constants[1].uintVal = inputSize.cy;
	constants[2].uintVal = outputSize.cx;
	constants[3].uintVal = outputSize.cy;
	constants[4].floatVal = 1.0f / inputSize.cx;
	constants[5].floatVal = 1.0f / inputSize.cy;
	constants[6].floatVal = 1.0f / outputSize.cx;
	constants[7].floatVal = 1.0f / outputSize.cy;
	constants[8].floatVal = outputSize.cx / (FLOAT)inputSize.cx;
	constants[9].floatVal = outputSize.cy / (FLOAT)inputSize.cy;

	// PS 样式的通道需要的参数
	EffectHelper::Constant32* pCurParam = constants.data() + builtinConstantCount;
	if (psStylePassParams > 0) {
		for (UINT i = 0, end = (UINT)desc.passes.size() - 1; i < end; ++i) {
			if (desc.passes[i].flags & EffectPassFlags::PSStyle) {
				D3D11_TEXTURE2D_DESC outputDesc;
				_textures[desc.passes[i].outputs[0]]->GetDesc(&outputDesc);
				pCurParam->uintVal = outputDesc.Width;
				++pCurParam;
				pCurParam->uintVal = outputDesc.Height;
				++pCurParam;
				pCurParam->floatVal = 1.0f / outputDesc.Width;
				++pCurParam;
				pCurParam->floatVal = 1.0f / outputDesc.Height;
				++pCurParam;
			}
		}
	}

	if (!isInlineParams) {
		for (UINT i = 0; i < desc.params.size(); ++i) {
			const auto& paramDesc = desc.params[i];
			auto it = option.parameters.find(paramDesc.name);

			if (paramDesc.constant.index() == 0) {
				const EffectConstant<float>& constant = std::get<0>(paramDesc.constant);
				float value = constant.defaultValue;

				if (it != option.parameters.end()) {
					value = it->second;

					if (value < constant.minValue || value > constant.maxValue) {
						return false;
					}
				}

				pCurParam->floatVal = value;
			} else {
				const EffectConstant<int>& constant = std::get<1>(paramDesc.constant);
				int value = constant.defaultValue;

				if (it != option.parameters.end()) {
					value = (int)std::lroundf(it->second);

					if ((value < constant.minValue) || (value > constant.maxValue)) {
						Logger::Get().Error(StrHelper::Concat("参数 ", paramDesc.name, " 的值非法"));
						return false;
					}
				}

				pCurParam->intVal = value;
			}

			++pCurParam;
		}
	}

	if (_constantBuffer) {
		// 更新缓冲区
		deviceResources.GetD3DDC()->UpdateSubresource1(
			_constantBuffer.get(), 0, nullptr, constants.data(), 0, 0, D3D11_COPY_DISCARD);
	} else {
		// 创建缓冲区
		D3D11_BUFFER_DESC bd{
			.ByteWidth = 4 * (UINT)constants.size(),
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER
		};

		D3D11_SUBRESOURCE_DATA initData{ .pSysMem = constants.data() };

		HRESULT hr = deviceResources.GetD3DDevice()->CreateBuffer(&bd, &initData, _constantBuffer.put());
		if (FAILED(hr)) {
			Logger::Get().ComError("CreateBuffer 失败", hr);
			return false;
		}
	}

	return true;
}

void EffectDrawer::_DrawPass(uint32_t i) const noexcept {
	_d3dDC->CSSetShader(_shaders[i].get(), nullptr, 0);

	_d3dDC->CSSetShaderResources(0, (UINT)_srvs[i].size(), _srvs[i].data());
	UINT uavCount = (UINT)_uavs[i].size() / 2;
	_d3dDC->CSSetUnorderedAccessViews(0, uavCount, _uavs[i].data(), nullptr);

	_d3dDC->Dispatch(_dispatches[i].first, _dispatches[i].second, 1);

	_d3dDC->CSSetUnorderedAccessViews(0, uavCount, _uavs[i].data() + uavCount, nullptr);
}

static bool IsReadonlyTexture(const EffectDesc& desc, uint32_t texture) noexcept {
	return texture == 0 || !desc.textures[texture].source.empty();
}

// 计算导出某个通道的输出时需要重新渲染的通道
SmallVector<uint32_t> EffectDrawer::_CalcPassesToDrawForExport(
	const EffectDesc& desc,
	uint32_t passIdx
) const noexcept {
	SmallVector<uint32_t> passesToDraw;
	passesToDraw.push_back(passIdx);

	if (passIdx == 0) {
		return passesToDraw;
	}

	const std::vector<EffectPassDesc>& passes = desc.passes;
	const uint32_t end = (uint32_t)passes.size() - 1;

	// 用于记录该通道依赖的输入纹理，格式为 (passIdx, texture)
	SmallVector<std::pair<uint32_t, uint32_t>, 0> depTextures;

	for (uint32_t input : passes[passIdx].inputs) {
		if (!IsReadonlyTexture(desc, input)) {
			depTextures.emplace_back(passIdx, input);
		}
	}

	while (!depTextures.empty()) {
		const auto [curPass, curTexture] = depTextures.pop_back_val();

		// 检查 curTexture 是否会被后面的通道修改
		{
			bool isOverwritten = false;
			for (uint32_t i = curPass + 1; i < end; ++i) {
				const SmallVector<uint32_t>& curOutputs = passes[i].outputs;
				if (std::find(curOutputs.begin(), curOutputs.end(), curTexture) != curOutputs.end()) {
					isOverwritten = true;
					break;
				}
			}
			if (!isOverwritten) {
				continue;
			}
		}

		// 需要重新渲染前一个输出 curTexture 的通道，并带来新的依赖
		for (int i = (int)curPass - 1; i >= 0; --i) {
			const SmallVector<uint32_t>& curOutputs = passes[i].outputs;
			if (std::find(curOutputs.begin(), curOutputs.end(), curTexture) != curOutputs.end()) {
				const uint32_t ui = (uint32_t)i;

				if (std::find(passesToDraw.begin(), passesToDraw.end(), ui) == passesToDraw.end()) {
					passesToDraw.push_back(ui);

					// 作为优化，如果之前的所有通道都需要重新渲染则提前返回
					if ((uint32_t)passesToDraw.size() == passIdx + 1) {
						for (uint32_t j = 0; j <= passIdx; ++j) {
							passesToDraw[j] = j;
						}
						return passesToDraw;
					}

					for (uint32_t input : passes[ui].inputs) {
						if (!IsReadonlyTexture(desc, input)) {
							depTextures.emplace_back(ui, input);
						}
					}
				}

				break;
			}
		}
	}

	std::sort(passesToDraw.begin(), passesToDraw.end());
	return passesToDraw;
}

void EffectDrawer::_PrepareForDraw() const noexcept {
	{
		ID3D11Buffer* t = _constantBuffer.get();
		_d3dDC->CSSetConstantBuffers(0, 1, &t);
	}

	_d3dDC->CSSetSamplers(0, (UINT)_samplers.size(), _samplers.data());
}

}
