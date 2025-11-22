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
private:

};



