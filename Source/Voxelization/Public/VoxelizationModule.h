#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "VoxelizationModule.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVoxelization, Log, All);

class FVoxelizationModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};



UCLASS(meta = (ScriptName = "VoxelizationLibrary"), MinimalAPI)
class UVoxelizationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:


    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Voxelize Grid"), Category = "LBM Sim")
    static VOXELIZATION_API void VoxelizeGrid(class AVoxelGridActor* VoxelGrid, bool UseCPU = true);
};
