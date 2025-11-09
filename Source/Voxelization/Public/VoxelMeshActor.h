// VoxelMeshActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "VoxelMesh.h"
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

    // The source static mesh to be voxelized
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    TObjectPtr<UStaticMesh> SourceMesh;

    // Procedural mesh for visualizing VoxelMesh
    TObjectPtr<UProceduralMeshComponent> MeshComponent;

    // Voxel mesh data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    FVoxelMesh VoxelMesh;

    void BuildMeshFromVoxels();

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};

