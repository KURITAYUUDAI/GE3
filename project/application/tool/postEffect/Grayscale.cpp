#include "Grayscale.h"
#include "DirectXBase.h"
#include "PSOManager.h"
#include "SrvManager.h"
#include "Logger.h"
#include <cassert>

void Grayscale::Initialize(uint32_t width, uint32_t height)
{
    (void)width;
    (void)height;

    CreateConstantBuffer();
    RegisterPSOs();
}

void Grayscale::Finalize()
{

}

std::vector<PostEffect::PassFunc> Grayscale::GetPasses()
{
    return
    {
        [this](D3D12_CPU_DESCRIPTOR_HANDLE destRTV, uint32_t srcSRVIndex){
            Pass(srcSRVIndex, destRTV);
        }
    };
}

std::vector<std::vector<PostEffect::PassBarrier>> Grayscale::GetBarriers()
{
    return {};
}

void Grayscale::Pass(uint32_t srcSRVIndex, D3D12_CPU_DESCRIPTOR_HANDLE destRTV)
{
    auto* cmdList = DirectXBase::GetInstance()->GetCommandList();
    auto* srvManager = SrvManager::GetInstance();

    UpdateConstantBuffer();

    float clearColor[4] = {
        kClearColor_.x, kClearColor_.y, kClearColor_.z, kClearColor_.w };
    cmdList->ClearRenderTargetView(destRTV, clearColor, 0, nullptr);
    cmdList->OMSetRenderTargets(1, &destRTV, FALSE, nullptr);

    auto psoH = PSOManager::GetInstance()->GetPSOData(
        kPsoName_, PSOManager::BlendMode::None, PSOManager::FillMode::kSolid);
    cmdList->SetGraphicsRootSignature(psoH.rootSignature.Get());
    cmdList->SetPipelineState(psoH.pipelineState.Get());
    srvManager->PreDraw();
    cmdList->SetGraphicsRootDescriptorTable(
        0, srvManager->GetGPUDescriptorHandle(srcSRVIndex));
    cmdList->SetGraphicsRootConstantBufferView(
        1, constantBufferResource_->GetGPUVirtualAddress());
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void Grayscale::CreateConstantBuffer()
{
    constantBufferResource_ =
        DirectXBase::GetInstance()->CreateConstantBufferResource(sizeof(GrayscaleParam));

    HRESULT hr = constantBufferResource_->Map(
        0, nullptr, reinterpret_cast<void**>(&constantBufferMappedData_));
    assert(SUCCEEDED(hr));

    *constantBufferMappedData_ = param_;
}

void Grayscale::RegisterPSOs()
{
    auto makeRootSignature = []() -> ComPtr<ID3D12RootSignature>{
        D3D12_DESCRIPTOR_RANGE srvRange{};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0; // t0
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER params[2]{};
        // [0] t0 : 入力テクスチャ
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        params[0].DescriptorTable.pDescriptorRanges = &srvRange;
        params[0].DescriptorTable.NumDescriptorRanges = 1;
        // [1] b0 : time
        params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        params[1].Descriptor.ShaderRegister = 0; // b0

        // バイリニア + クランプ（線形サンプリング最適化に必須）
        D3D12_STATIC_SAMPLER_DESC sampler =
            PSOManager::GetInstance()->GetDefaultStaticSamplerDesc();
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

        D3D12_ROOT_SIGNATURE_DESC sigDesc{};
        sigDesc.NumParameters = _countof(params);
        sigDesc.pParameters = params;
        sigDesc.NumStaticSamplers = 1;
        sigDesc.pStaticSamplers = &sampler;
        sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> sigBlob, errBlob;
        HRESULT hr = D3D12SerializeRootSignature(
            &sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
        if (FAILED(hr)) {
            Logger::Log(reinterpret_cast<char*>(errBlob->GetBufferPointer()));
            assert(false);
        }

        ComPtr<ID3D12RootSignature> rootSig;
        hr = DirectXBase::GetInstance()->GetDevice()->CreateRootSignature(
            0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(),
            IID_PPV_ARGS(&rootSig));
        assert(SUCCEEDED(hr));
        return rootSig;
        };

    auto makeInputLayout = [](){
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{};
        };

    PSOManager::PSOConfig config{};
    config.vertexShaderPath = L"resources/shaders/PostEffect/Fullscreen.VS.hlsl";
    config.pixelShaderPath = L"resources/shaders/PostEffect/Grayscale.PS.hlsl";
    config.rootSignatureGenerator = makeRootSignature;
    config.inputLayoutGenerator = makeInputLayout;
    config.depthEnable = false;
    PSOManager::GetInstance()->RegisterPSOConfig(kPsoName_, config);
}

void Grayscale::UpdateConstantBuffer()
{
    *constantBufferMappedData_ = param_;
}
