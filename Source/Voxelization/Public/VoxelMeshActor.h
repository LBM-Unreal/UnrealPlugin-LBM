// VoxelMeshActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelMeshAsset.h"
#include "VoxelMeshActor.generated.h"

// Visualize a voxel mesh 
UCLASS(BlueprintType)
class AVoxelMeshActor : public AActor
{
    GENERATED_BODY()

public:
    AVoxelMeshActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void OnConstruction(const FTransform& Transform) override;

    // Procedural mesh for rendering
    UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    TObjectPtr<UVoxelMeshAsset> VoxelMesh;

    void BuildMeshFromVoxels();
};

