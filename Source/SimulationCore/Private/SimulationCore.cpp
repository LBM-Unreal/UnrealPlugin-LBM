#include "SimulationCore.h"

#include "ShaderParameterStruct.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSimulationCoreShaderParameters, )
	SHADER_PARAMETER(float, Scale)
	SHADER_PARAMETER(float, Translate)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<float>, InputBuffer)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<float>, OutputBuffer)
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray)
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray2)
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
END_SHADER_PARAMETER_STRUCT()


BEGIN_SHADER_PARAMETER_STRUCT(FSimulationLBMShaderParameters, )
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray)
	SHADER_PARAMETER_UAV(RWTexture2DArray<float4>, SimulationDataArray2)
	SHADER_PARAMETER_UAV(RWTexture2D<float4>, DebugTexture)
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
	InputBuffer.Initialize(RHICmdList, TEXT("InputBuffer"), sizeof(float), 1);
	OutputBuffer.Initialize(RHICmdList, TEXT("OutputBuffer"), sizeof(float), 1);
	DebugTexture = RHICmdList.CreateTexture(Desc.SetDebugName(TEXT("LBM_DebugTexture")));

	Desc.ArraySize = 9;
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
		.SetArrayRange(0, 9));
	SimulationDataArray2UAV = RHICmdList.CreateUnorderedAccessView(SimulationDataArray2, FRHIViewDesc::CreateTextureUAV()
		.SetDimensionFromTexture(SimulationDataArray2)
		.SetMipLevel(0)
		.SetArrayRange(0, 9));


}

// Buffer释放函数
void FSimulationShaderResource::ReleaseRHI()
{
	InputBuffer.Release();
	OutputBuffer.Release();
	DebugTextureUAV.SafeRelease();
	SimulationDataArrayUAV.SafeRelease();
	SimulationDataArray2UAV.SafeRelease();

	DebugTexture.SafeRelease();
	SimulationDataArray.SafeRelease();
	SimulationDataArray2.SafeRelease();
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
		Parameters.InputBuffer = Resource->InputBuffer.UAV;
		Parameters.OutputBuffer = Resource->OutputBuffer.UAV;
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;

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
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
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
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
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
		Parameters.SimulationDataArray = Resource->SimulationDataArrayUAV;
		Parameters.SimulationDataArray2 = Resource->SimulationDataArray2UAV;
		Parameters.DebugTexture = Resource->DebugTextureUAV;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
	}
	DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
	//UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}

void DispatchExampleComputeShader_GameThread(float InputVal, float Scale, float Translate, FSimulationShaderResource* Resource)
{
	// 加入RenderThread任务
	ENQUEUE_RENDER_COMMAND(FDispatchExampleComputeShader)([Resource, InputVal, Scale, Translate](FRHICommandListImmediate& RHICmdList)
	{
		// LockBuffer并写入数据
		float* InputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(Resource->InputBuffer.Buffer, 0, sizeof(float), RLM_WriteOnly));
		*InputGPUBuffer = InputVal; // 可以使用FMemory::Memcpy
		// UnlockBuffer
		RHICmdList.UnlockBuffer(Resource->InputBuffer.Buffer);
		// 调用RenderThread版本的函数
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
			float* OutputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(Resource->OutputBuffer.Buffer, 0, sizeof(float), RLM_ReadOnly));
			*pOutputVal = *OutputGPUBuffer;
			// UnlockBuffer
			RHICmdList.UnlockBuffer(Resource->OutputBuffer.Buffer);
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
