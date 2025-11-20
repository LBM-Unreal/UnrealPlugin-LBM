#include "SimulationCore.h"

#include "ShaderParameterStruct.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSimulationCoreShaderParameters, )
	SHADER_PARAMETER(float, Scale)
	SHADER_PARAMETER(float, Translate)
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, DebugBuffer)
END_SHADER_PARAMETER_STRUCT()


BEGIN_SHADER_PARAMETER_STRUCT(FSimulationLBMShaderParameters, )
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, DebugBuffer)
END_SHADER_PARAMETER_STRUCT()


class FSimulationCoreShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FSimulationCoreShaderCS, Global)
		SHADER_USE_PARAMETER_STRUCT(FSimulationCoreShaderCS, FGlobalShader)
		using FParameters = FSimulationCoreShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FSimulationCoreShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("FunctionMultiply"), SF_Compute)


class FLBMInitialStateShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMInitialStateShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMInitialStateShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMInitialStateShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_InitialState"), SF_Compute)



class FLBMStreamingShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMStreamingShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMStreamingShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMStreamingShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Streaming"), SF_Compute)


class FLBMCollisionShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMCollisionShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMCollisionShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMCollisionShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Collision"), SF_Compute)


class FLBMMRInitialStateShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMMRInitialStateShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMMRInitialStateShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMMRInitialStateShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_MR_InitialState"), SF_Compute)


class FLBMMRStreamingCollisionShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMMRStreamingCollisionShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMMRStreamingCollisionShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMMRStreamingCollisionShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_MR_Streaming_Collision"), SF_Compute)


class FLBMBoundaryTreatmentShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMBoundaryTreatmentShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMBoundaryTreatmentShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMBoundaryTreatmentShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_BoundaryTreatment"), SF_Compute)

class FLBMInitialInterfaceShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMInitialInterfaceShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMInitialInterfaceShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMInitialInterfaceShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_InitialInterface"), SF_Compute)

class FLBMAdvectMassShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMAdvectMassShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMAdvectMassShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMAdvectMassShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_AdvectMass"), SF_Compute)


class FLBMSurface1ShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMSurface1ShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMSurface1ShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMSurface1ShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Surface_1"), SF_Compute)


class FLBMSurface2ShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMSurface2ShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMSurface2ShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMSurface2ShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Surface_2"), SF_Compute)


class FLBMSurface3ShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMSurface3ShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMSurface3ShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMSurface3ShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Surface_3"), SF_Compute)


class FLBMReverseDFShaderCS : public FGlobalShader
{
public:
	DECLARE_SHADER_TYPE(FLBMReverseDFShaderCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FLBMReverseDFShaderCS, FGlobalShader)
		using FParameters = FSimulationLBMShaderParameters;
};

IMPLEMENT_SHADER_TYPE(, FLBMReverseDFShaderCS, TEXT("/LBM/Shaders/SimulationCoreCompute.usf"), TEXT("LBM_Reverse_DF"), SF_Compute)


// ExampleComputeShaderModule.cpp

// 初始化单例指针
FSimulationShaderResource* FSimulationShaderResource::GInstance = nullptr;

FSimulationShaderResource* FSimulationShaderResource::Get()
{
	if (GInstance == nullptr)
	{
		GInstance = new FSimulationShaderResource();
	}
	return GInstance;
}

// Buffer初始化函数
void FSimulationShaderResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	FRHITextureCreateDesc Desc =
		FRHITextureCreateDesc::Create2D(TEXT("LBM_Textures"), 256, 256, PF_A32B32G32R32F)
		.SetFlags(ETextureCreateFlags::UAV).SetClearValue(FClearValueBinding(FLinearColor(0.f, 0.f, 0.f, 0.f)));

	DebugTexture = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_DebugTexture")));
	DebugBuffer.Initialize(RHICmdList, TEXT("LBM_DebugBuffer"), sizeof(int), 256*256*14);
	Desc.ArraySize = 14;
	Desc.SetDimension(ETextureDimension::Texture2DArray);

	FRHIViewDesc ViewDesc;
	
	DebugTextureUAV = RHICmdList.CreateUnorderedAccessView(DebugTexture, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(DebugTexture)
		.SetMipLevel(0)
		.SetArrayRange(0, 1));
}

// Buffer释放函数
void FSimulationShaderResource::ReleaseRHI()
{
	DebugTextureUAV.SafeRelease();
	DebugTexture.SafeRelease();
	DebugBuffer.Release();
}

void DispatchExampleComputeShader_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, float Scale, float Translate, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FSimulationCoreShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel)); // 声明ShaderMap
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader()); // 设置该RHICmdList的Pipeline为ComputeShader
	{
		typename FSimulationCoreShaderCS::FParameters Parameters{}; // 创建Shader参数

		// 设置基本类型
		Parameters.Scale = Scale;
		Parameters.Translate = Translate;

		// 设置buffer类型
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;

		// 传入参数
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}

	// 调用Compute shader
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);

	// 取消绑定buffer参数
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMInitalState_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMInitialStateShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMInitialStateShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMStreaming_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMStreamingShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMStreamingShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMCollision_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMCollisionShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMCollisionShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMMRInitialState_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMMRInitialStateShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMMRInitialStateShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMMRStreamingCollision_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMMRStreamingCollisionShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMMRStreamingCollisionShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMBoundaryTreatment_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMBoundaryTreatmentShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMBoundaryTreatmentShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMInitialInterface_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMInitialInterfaceShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMInitialInterfaceShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}


void DispatchLBMAdvectMass_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMAdvectMassShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMAdvectMassShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMSurface1_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMSurface1ShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMSurface1ShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMSurface2_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMSurface2ShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMSurface2ShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMSurface3_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMSurface3ShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMSurface3ShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchLBMReverseDF_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
	TShaderMapRef<FLBMReverseDFShaderCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());
	{
		typename FLBMReverseDFShaderCS::FParameters Parameters{};
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		Parameters.DebugBuffer = Resource->DebugBuffer.UAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchExampleComputeShader_GameThread(float InputVal, float Scale, float Translate, FSimulationShaderResource* Resource)
{
	// 加入RenderThread任务
	ENQUEUE_RENDER_COMMAND(FDispatchExampleComputeShader)([Resource, InputVal, Scale, Translate](FRHICommandListImmediate& RHICmdList)
	{
		DispatchExampleComputeShader_RenderThread(RHICmdList, Resource, Scale, Translate, 16, 16, 16);
	});

	FlushRenderingCommands();
}

float GetGPUReadback(FSimulationShaderResource* Resource, float& OutputVal)
{
	float* pOutputVal = &OutputVal;
	// Flush所有RenderingCommands, 确保我们的shader已经执行了
	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(FReadbackOutputBuffer)([Resource, &pOutputVal](FRHICommandListImmediate& RHICmdList)
		{
			// LockBuffer并读取数据
			//float* OutputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(Resource->OutputBuffer.Buffer, 0, sizeof(float), RLM_ReadOnly));
			//*pOutputVal = *OutputGPUBuffer;
			// UnlockBuffer
			//RHICmdList.UnlockBuffer(Resource->OutputBuffer.Buffer);
		});
	// FlushRenderingCommands, 确保上面的RenderCommand被执行了
	FlushRenderingCommands();
	return OutputVal;

	// 下面是使用FRHIGPUBufferReadback的调用方式, 效果是相同的
	//FRHIGPUBufferReadback ReadbackBuffer(TEXT("ExampleComputeShaderReadback"));
	//FlushRenderingCommands();
	//ENQUEUE_RENDER_COMMAND(FEnqueueGPUReadback)([&ReadbackBuffer, Resource](FRHICommandListImmediate& RHICmdList)
	//{
	//	ReadbackBuffer.EnqueueCopy(RHICmdList, Resource->OutputBuffer.Buffer, sizeof(int32));
	//});
	//FlushRenderingCommands();
	//float OutputVal;
	//ENQUEUE_RENDER_COMMAND(FEnqueueGPUReadbackLock)([&ReadbackBuffer, Resource, &OutputVal](FRHICommandListImmediate& RHICmdList)
	//{
	//	float* pOutputVal = static_cast<float*>(ReadbackBuffer.Lock(sizeof(float)));
	//	checkf(pOutputVal!=nullptr, TEXT("ReadbackBuffer failed to lock"));
	//	OutputVal = *pOutputVal;
	//	ReadbackBuffer.Unlock();
	//});
	//FlushRenderingCommands();
	//return OutputVal;
}
