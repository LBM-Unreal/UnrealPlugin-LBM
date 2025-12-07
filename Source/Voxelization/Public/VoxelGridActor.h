#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"
#include "VoxelGrid.h"
#include "VoxelGridActor.generated.h"

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
    UPROPERTY(EditAnywhere, Category = "Voxel")
    TArray<AActor*> ImmovableActors;


    UVoxelGridVisualizationComponent* VoxelMeshVisualization;

    UPROPERTY(EditAnywhere, Category = "Voxel")
    ALandscape* LandscapeActor;

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

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Debug")
    void DebugFunc0();

    // Store overlapping landscape components
    UPROPERTY(VisibleAnywhere, Category = "Debug")
    TArray<ULandscapeComponent*> OverlappingLandscapeComponents;
    
};
