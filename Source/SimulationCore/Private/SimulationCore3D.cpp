#include "SimulationCore.h"

#include "ShaderParameterStruct.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSimulationLBMShaderParameters, )
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray)
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray2)
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
END_SHADER_PARAMETER_STRUCT()

class FLBMInitialState3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMInitialState3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMInitialState3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMInitialState3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_InitialState3D"), SF_Compute)



class FLBMStreaming3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMStreaming3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMStreaming3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMStreaming3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_Streaming3D"), SF_Compute)


class FLBMCollision3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMCollision3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMCollision3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMCollision3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_Collision3D"), SF_Compute)

// 初始化单例指针
FSimulationShaderResource3D* FSimulationShaderResource3D::GInstance = nullptr;

FSimulationShaderResource3D* FSimulationShaderResource3D::Get()
{
	if (GInstance == nullptr)
	{
		GInstance = new FSimulationShaderResource3D();
	}
	return GInstance;
}

// Buffer初始化函数
void FSimulationShaderResource3D::InitRHI(FRHICommandListBase& RHICmdList)
{
	FRHITextureCreateDesc Desc =
		FRHITextureCreateDesc::Create2D(TEXT("LBM_Textures"), 128, 128, PF_A32B32G32R32F)
		.SetFlags(ETextureCreateFlags::UAV).SetClearValue(FClearValueBinding(FLinearColor(0.f, 0.f, 0.f, 0.f)));
	DebugTexture = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_DebugTexture")));

	Desc.ArraySize = 5*128;
	Desc.SetDimension(ETextureDimension::Texture2DArray);
	SimulationDataArray = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_SimulationDataArray")));
	SimulationDataArray2 = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_SimulationDataArray2")));

	FRHIViewDesc ViewDesc;
	
	DebugTextureUAV = RHICmdList.CreateUnorderedAccessView(DebugTexture, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(DebugTexture)
		.SetMipLevel(0)
		.SetArrayRange(0, 1));
	SimulationDataArrayUAV = RHICmdList.CreateUnorderedAccessView(SimulationDataArray, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(SimulationDataArray)
		.SetMipLevel(0)
		.SetArrayRange(0, 5 * 128));
	SimulationDataArray2UAV = RHICmdList.CreateUnorderedAccessView(SimulationDataArray2, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(SimulationDataArray2)
		.SetMipLevel(0)
		.SetArrayRange(0, 5 * 128));
}

// Buffer释放函数
void FSimulationShaderResource3D::ReleaseRHI()
{
	DebugTextureUAV.SafeRelease();
	SimulationDataArrayUAV.SafeRelease();
	SimulationDataArray2UAV.SafeRelease();

	DebugTexture.SafeRelease();
	SimulationDataArray.SafeRelease();
	SimulationDataArray2.SafeRelease();
}

void DispatchLBMInitalState3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMInitialState3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMInitialState3DShaderCS::FParameters Parameters{};
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMStreaming3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMStreaming3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMStreaming3DShaderCS::FParameters Parameters{};
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMCollision3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMCollision3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMCollision3DShaderCS::FParameters Parameters{};
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}
