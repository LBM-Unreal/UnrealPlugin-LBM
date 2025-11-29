#pragma once

#include "CoreMinimal.h"
#include "VoxelGrid.generated.h"

/**
 * Pure data container representing a uniform 3D voxel grid.
 * Stores solid/empty occupancy for simulation or voxelization.
 */
USTRUCT(BlueprintType)
struct VOXELIZATION_API FVoxelGrid
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category = "Voxel")
    float VoxelSize = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Voxel")
    FIntVector GridDim = FIntVector::ZeroValue;

    UPROPERTY(EditAnywhere, Category = "Voxel")
    FVector3f Origin = FVector3f::ZeroVector;

    /** Voxel data of stationary mesh*/
    TArray<uint32> ImmovableMeshOccupancy;

    /** Voxel data of movable mesh */
    TArray<uint32> MovableMeshOccupancy;
};


class VOXELIZATION_API FVoxelGridResource : public FRenderResource
{
    static FVoxelGridResource* GInstance;
public:
    // Render Resources
    FRWBufferStructured ImmovableMeshOccupancyBuffer;
    FRWBufferStructured MovableMeshOccupancyBuffer;
    FRWBufferStructured GridVelocityBuffer;

    // Parameters
    FVector3f  GridOrigin;
    FIntVector GridDim;

    virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
    virtual void ReleaseRHI() override;

    static FVoxelGridResource* Get();

    FORCEINLINE uint32 GetBufferLength()
    {
        return GridDim.X * GridDim.Y * GridDim.Z;
    }
private:
	FVoxelGridResource() : GridDim(FIntVector(16,16,16))
    {
        ENQUEUE_RENDER_COMMAND(FCreateVoxelGridRes)([this](FRHICommandListImmediate& RHICmdList)
        {
        	this->InitResource(RHICmdList);
        });
    };
};