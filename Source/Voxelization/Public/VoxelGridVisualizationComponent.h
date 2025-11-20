#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelGridVisualizationComponent.generated.h"

class UBoxComponent;
class UProceduralMeshComponent;

UCLASS()
class UVoxelGridVisualizationComponent : public UPrimitiveComponent
{
    GENERATED_BODY()

public:
    UVoxelGridVisualizationComponent();

    /** Internal procedural mesh used for voxel visualization */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Voxel")
    TObjectPtr<UProceduralMeshComponent> ProceduralMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
    TObjectPtr<UBoxComponent> VoxelGridArea;

    /** Rebuilds the procedural mesh using voxelized data */
    void UpdateVisualization(const struct FVoxelGrid& VoxelGrid);
protected:
    void OnRegister() override;
};
