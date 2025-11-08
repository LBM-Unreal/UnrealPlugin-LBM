#pragma once

#include "CoreMinimal.h"
#include "VoxelMesh.h"
#include "VoxelMeshAsset.generated.h"


UCLASS(BlueprintType)
class UVoxelMeshAsset : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
    TObjectPtr<UStaticMesh> SourceMesh;

    UPROPERTY(EditAnywhere, Category = "Voxel")
    FVoxelMesh VoxelMeshData;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};