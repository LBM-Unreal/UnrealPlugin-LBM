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

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Output Texture Value"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetOutputTextureValue(UTexture* OutTexture)
	{
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
		{
			FRHICopyTextureInfo CopyInfo;
			CopyInfo.Size = {256,256,1};
			RHICmdList.CopyTexture(FSimulationShaderResource::Get()->OutputTexture, OutTexture->GetResource()->GetTexture2DRHI(), CopyInfo);
		});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Renderdoc"), Category = "LBM Sim")
	static SIMULATIONBP_API void StartRenderdoc() {
		ENQUEUE_RENDER_COMMAND(FStartRenderdoc)([](FRHICommandListImmediate& RHICmdList)
			{
				UE_LOG(LogTemp, Warning, TEXT("Saved in %s"), *FString(FPaths::Combine(FPaths::ProjectDir(), TEXT("Captures"))));
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
};