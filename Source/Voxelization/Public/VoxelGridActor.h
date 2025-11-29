#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelGrid.h"
#include "VoxelGridActor.generated.h"


// Affecting area (clamp to grid )
// Static Mesh Actor list

// voxel buffer (tex3d? structured buffer?)

class UVoxelGridVisualizationComponent;

UCLASS(BlueprintType)
class AVoxelGridActor : public AActor
{
    GENERATED_BODY()

public:
    AVoxelGridActor();

    UPROPERTY(EditAnywhere, Category = "Voxel")
    FVoxelGrid VoxelGrid;

    UPROPERTY(EditAnywhere, Category = "Voxel")
    TArray<AStaticMeshActor*> ImmovableMeshes;


    UVoxelGridVisualizationComponent* VoxelMeshVisualization;


    UFUNCTION(CallInEditor)
    void CopyBufferToCPU();

    UFUNCTION(CallInEditor)
    void FillImmovableVoxelGrid();

    UFUNCTION(CallInEditor)
    void FillImmovableVoxelGridGPU();

    UFUNCTION(CallInEditor)
    void Visualize();

    UFUNCTION(CallInEditor)
    void CalculateVertexVelocity();

/** Debug */ 
    TArray<FVector3f> VertexVelocitiesCPU;

    UFUNCTION(CallInEditor, Category = "Debug")
    void CopyVelocityToCPU();


    TArray<FVector3f> VoxelGridVelocitiesCPU;

    UFUNCTION(CallInEditor, Category = "Debug")
    void CopyVoxelGridVelocityToCPU();


    UFUNCTION(BlueprintCallable, Category = "Debug")
	void DemoTick(float DeltaTime);


    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DemoVisualize();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DemoTickCPU();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DemoVisualizeCPU();

    UFUNCTION(CallInEditor, Category = "Debug")
    void GPUDebug();

    UFUNCTION(CallInEditor, Category = "Debug")
    void CPUDebug();

};
