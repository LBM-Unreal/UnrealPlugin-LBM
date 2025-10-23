// SimulationBPModule.h
#pragma once


#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimulationCore.h"

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
};