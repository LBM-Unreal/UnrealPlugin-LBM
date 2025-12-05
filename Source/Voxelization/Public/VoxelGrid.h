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
    TArray<FVector4f> ImmovableMeshNormal;
    TArray<FVector3f> ImmovableMeshVelocity;

    /** Voxel data of movable mesh */
    TArray<uint32> MovableMeshOccupancy;

public:
    FORCEINLINE void SetGridDim(FIntVector InGridDim)
    {
        GridDim = InGridDim;
        auto NewBufferLength = InGridDim.X * InGridDim.Y * InGridDim.Z;
        ImmovableMeshOccupancy.SetNumZeroed(NewBufferLength, EAllowShrinking::Yes);
        MovableMeshOccupancy.SetNumZeroed(NewBufferLength, EAllowShrinking::Yes);
    }

    /** Convert 3D coordinates to linear index */
    FORCEINLINE int32 Index(int32 X, int32 Y, int32 Z) const
    {
        return X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
    }

    /** Get voxel occupancy */
    FORCEINLINE int32 ReadGridImmovable(int32 X, int32 Y, int32 Z) const
    {
        return ImmovableMeshOccupancy[Index(X, Y, Z)];
    }

    /** Set voxel occupancy */
    FORCEINLINE void WriteGridImmovable(int32 X, int32 Y, int32 Z, uint32 Value)
    {
        ImmovableMeshOccupancy[Index(X, Y, Z)] = Value;
    }

    FORCEINLINE void ClearGridImmovable()
    {
	    for (int32 Idx = 0; Idx < ImmovableMeshOccupancy.Num(); Idx++)
        {
            ImmovableMeshOccupancy[Idx] = 0;
            ImmovableMeshNormal[Idx] = FVector4f(0);
        }
    }

    FORCEINLINE int32 GetBuffLength() const
    {
        return GridDim.X * GridDim.Y * GridDim.Z;
    }
};


class VOXELIZATION_API FVoxelGridResource : public FRenderResource
{
    static FVoxelGridResource* GInstance;
public:
    // Render Resources
    FRWBufferStructured ImmovableMeshOccupancyBuffer;
    FRWBufferStructured ImmovableMeshNormalBuffer;
    FRWBufferStructured ImmovableMeshVelocityBuffer;

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