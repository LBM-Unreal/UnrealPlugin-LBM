#include "SimulationCore.h"

#include "ShaderParameterStruct.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSimulationLBM3DShaderParameters, )
	SHADER_PARAMETER(float, InitialVelocity)
	SHADER_PARAMETER(float, InitialDensity)
	SHADER_PARAMETER(float, RelaxationFactor)
	SHADER_PARAMETER(FIntVector3, SimDimension)
	SHADER_PARAMETER(int, DebugTextureSlice)
	SHADER_PARAMETER_ARRAY(FIntVector4, c, [20])
	SHADER_PARAMETER_ARRAY(FVector4f, w, [20])
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
	SHADER_PARAMETER_UAV(RWTexture3D<half4>, DebugTexture3D)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, DebugBuffer)
END_SHADER_PARAMETER_STRUCT()

class FLBMInitialState3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMInitialState3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMInitialState3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMInitialState3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_InitialState3D"), SF_Compute)



class FLBMStreaming3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMStreaming3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMStreaming3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMStreaming3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_Streaming3D"), SF_Compute)


class FLBMCollision3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMCollision3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMCollision3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMCollision3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_Collision3D"), SF_Compute)

class FLBMMRInitialState3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMMRInitialState3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMMRInitialState3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMMRInitialState3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_MR_InitialState3D"), SF_Compute)


class FLBMMRStreamingCollision3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMMRStreamingCollision3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMMRStreamingCollision3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMMRStreamingCollision3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_MR_Streaming_Collision3D"), SF_Compute)

class FLBMBoundaryTreatment3DShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMBoundaryTreatment3DShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMBoundaryTreatment3DShaderCS, FGlobalShader)
		using FParameters = FSimulationLBM3DShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMBoundaryTreatment3DShaderCS, TEXT("/LBM/Shaders/SimulationCore3DCompute.usf"), TEXT("LBM_BoundaryTreatment3D"), SF_Compute)

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
		FRHITextureCreateDesc::Create2D(TEXT("LBM_Textures"), TextureSize[0], TextureSize[1], PF_A32B32G32R32F)
		.SetFlags(ETextureCreateFlags::UAV).SetClearValue(FClearValueBinding(FLinearColor(0.f, 0.f, 0.f, 0.f)));
	DebugTexture = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_DebugTexture")));

	FRHITextureCreateDesc Desc3D =
		FRHITextureCreateDesc::Create3D(TEXT("LBM_Textures"), TextureSize[0], TextureSize[1], TextureSize[2], PF_A16B16G16R16)
		.SetFlags(ETextureCreateFlags::UAV).SetClearValue(FClearValueBinding(FLinearColor(0.f, 0.f, 0.f, 0.f)));
	DebugTexture3D = RHICmdList.CreateTexture(Desc3D.SetDebugName(TEXT("LBM_DebugTexture3D")));

	DebugTextureUAV = RHICmdList.CreateUnorderedAccessView(DebugTexture, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(DebugTexture)
		.SetMipLevel(0)
		.SetArrayRange(0, 1));
	DebugTexture3DUAV = RHICmdList.CreateUnorderedAccessView(DebugTexture3D, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(DebugTexture3D)
		.SetMipLevel(0)
		.SetArrayRange(0, 1));
	DebugBuffer.Initialize(RHICmdList, TEXT("LBM_DebugBuffer"), sizeof(int), (TextureSize[0]+4) * (TextureSize[1]+4) * (TextureSize[2]+4) * 20);
}

// Buffer释放函数
void FSimulationShaderResource3D::ReleaseRHI()
{
	DebugTextureUAV.SafeRelease();
	DebugTexture3DUAV.SafeRelease();
	DebugTexture.SafeRelease();
	DebugTexture3D.SafeRelease();
	DebugTexture.SafeRelease();
	DebugBuffer.Release();
}

void DispatchLBMInitalState3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMInitialState3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMInitialState3DShaderCS::FParameters Parameters{};
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		for(int i = 0; i < 19; ++i)
		{
			Parameters.c[i] = FIntVector4(Resource->c[i], 0);
			Parameters.w[i][0] = Resource->w[i];
		}
		//Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
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
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		for (int i = 0; i < 19; ++i)
		{
			Parameters.c[i] = FIntVector4(Resource->c[i], 0);
			Parameters.w[i][0] = Resource->w[i];
		}
		//Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
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
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		for (int i = 0; i < 19; ++i)
		{
			Parameters.c[i] = FIntVector4(Resource->c[i], 0);
			Parameters.w[i][0] = Resource->w[i];
		}
		//Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMMRInitialState3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMMRInitialState3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMMRInitialState3DShaderCS::FParameters Parameters{};
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMMRStreamingCollision3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMMRStreamingCollision3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMMRStreamingCollision3DShaderCS::FParameters Parameters{};
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMBoundaryTreatment3D_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource3D* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMBoundaryTreatment3DShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMBoundaryTreatment3DShaderCS::FParameters Parameters{};
		Parameters.InitialDensity = Resource->Params.InitialDensity;
		Parameters.InitialVelocity = Resource->Params.InitialVelocity;
		Parameters.RelaxationFactor = Resource->Params.RelaxationFactor;
		Parameters.SimDimension = Resource->Params.SimDimensions;
		Parameters.DebugTextureSlice = Resource->Params.DebugTextureSlice;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		for (int i = 0; i < 19; ++i)
		{
			Parameters.c[i] = FIntVector4(Resource->c[i], 0);
			Parameters.w[i][0] = Resource->w[i];
		}
		//Parameters.DebugTexture3D = Resource->DebugTexture3DUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}
