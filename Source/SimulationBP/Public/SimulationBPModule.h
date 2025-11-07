// SimulationBPModule.h
#pragma once


#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimulationCore.h"
#include "IRenderDocPlugin.h"

#include "SimulationBPModule.generated.h"

class FSimulationBPModule : public IModuleInterface {
    virtual void StartupModule() override;
    // virtual void ShutdownModule() override;
};

UCLASS(meta=(ScriptName="SimulationLibrary"), MinimalAPI)
class USimulationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta=(DisplayName="Execute Compute Shader"), Category="LBM Sim")
	static SIMULATIONBP_API float ExecuteExampleComputeShader(float InputVal, float Scale, float Translate) {
		float OutputVal;
		DispatchExampleComputeShader_GameThread(InputVal, Scale, Translate, FSimulationShaderResource::Get());
		return GetGPUReadback(FSimulationShaderResource::Get(), OutputVal);
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Debug Texture Value"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetDebugTextureValue(UTexture* OutTexture)
	{
		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
		{
			FRHICopyTextureInfo CopyInfo;
			CopyInfo.Size = {256,256,1};
			RHICmdList.CopyTexture(FSimulationShaderResource::Get()->DebugTexture, OutTexture->GetResource()->GetTexture2DRHI(), CopyInfo);
		});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Grab Output Texture By Array Id"), Category = "LBM Sim")
	static SIMULATIONBP_API void GrabOutputTextureByArrayId(UTexture* OutTexture, int Index)
	{
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture, Index](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.Size = { 256,256,1 };
				CopyInfo.SourceSliceIndex = Index;
				RHICmdList.CopyTexture(FSimulationShaderResource::Get()->SimulationDataArray, OutTexture->GetResource()->GetTexture2DRHI(), CopyInfo);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Renderdoc"), Category = "LBM Sim")
	static SIMULATIONBP_API void StartRenderdoc() {
		ENQUEUE_RENDER_COMMAND(FStartRenderdoc)([](FRHICommandListImmediate& RHICmdList)
			{
				UE_LOG(LogTemp, Warning, TEXT("RDC saved in %s"), *FString(FPaths::Combine(FPaths::ProjectDir(), TEXT("Captures"))));
				IRenderDocPlugin::Get().BeginCapture(&RHICmdList, 0, FPaths::Combine(FPaths::ProjectDir(), TEXT("Captures")));
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Renderdoc"), Category = "LBM Sim")
	static SIMULATIONBP_API void EndRenderdoc() {
		ENQUEUE_RENDER_COMMAND(FEndRenderdoc)([](FRHICommandListImmediate& RHICmdList)
			{
				IRenderDocPlugin::Get().EndCapture(&RHICmdList);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Resource"), Category = "LBM Sim")
	static SIMULATIONBP_API void UpdateResource() {
		ENQUEUE_RENDER_COMMAND(FUpdateResource)([](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource::Get()->UpdateRHI(RHICmdList);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM InitialState"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMInitialState() {
		ENQUEUE_RENDER_COMMAND(FLBMInitialState)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMInitalState_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 16);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Streaming"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMStreaming() {
		ENQUEUE_RENDER_COMMAND(FLBMStreaming)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMStreaming_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 16);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Collision"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMCollision() {
		ENQUEUE_RENDER_COMMAND(FLBMCollision)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMCollision_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 16);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Debug Texture Value 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetDebugTextureValue3D(UTexture* OutTexture)
	{
		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.Size = { 128,128,1 };
				RHICmdList.CopyTexture(FSimulationShaderResource3D::Get()->DebugTexture, OutTexture->GetResource()->GetTexture2DRHI(), CopyInfo);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Resource 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void UpdateResource3D() {
		ENQUEUE_RENDER_COMMAND(FUpdateResource)([](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->UpdateRHI(RHICmdList);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM InitialState 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMInitialState3D() {
		ENQUEUE_RENDER_COMMAND(FLBMInitialState)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMInitalState3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 16, 16, 16);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Streaming 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMStreaming3D() {
		ENQUEUE_RENDER_COMMAND(FLBMStreaming)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMStreaming3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 16, 16, 16);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Collision 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMCollision3D() {
		ENQUEUE_RENDER_COMMAND(FLBMCollision)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMCollision3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 16, 16, 16);
			});
	}
};