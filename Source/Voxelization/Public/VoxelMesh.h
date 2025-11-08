#pragma once

#include "CoreMinimal.h"
#include "VoxelMesh.generated.h"

/**
 * Pure data container representing a uniform 3D voxel grid.
 * Stores solid/empty occupancy for simulation or voxelization.
 */
USTRUCT(BlueprintType)
struct FVoxelMesh
{
    GENERATED_BODY()

    // ------------------------------------------------------------------------
    // 
    // ------------------------------------------------------------------------

    /** Number of voxels along each axis (resolution) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    FIntVector GridDim = FIntVector::ZeroValue;

    /** Size of one voxel in world units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    float VoxelSize = 1.0f;

    /** World-space origin (corner) of the grid */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    FVector3f Origin = FVector3f::ZeroVector;

    /** Voxel data */
    UPROPERTY()
    TArray<uint8> Occupancy;

    // ------------------------------------------------------------------------
	// Core operations
	// ------------------------------------------------------------------------
    void VoxelizeMesh(const UStaticMesh* Mesh);

    // ------------------------------------------------------------------------
    // Basic operations
    // ------------------------------------------------------------------------

    /** Total voxel count */
    FORCEINLINE int32 NumVoxels() const
    {
        return GridDim.X * GridDim.Y * GridDim.Z;
    }

    /** Allocate and zero the voxel buffer */
    FORCEINLINE void Allocate()
    {
        Occupancy.SetNumZeroed(NumVoxels());
    }

    /** Clear grid to a value (0 or 1) */
    FORCEINLINE void Clear(uint8 Value = 0)
    {
        for (uint8& V : Occupancy)
        {
            V = Value;
        }
    }

    /** Convert 3D coordinates to linear index */  
    FORCEINLINE int32 Index(int32 X, int32 Y, int32 Z) const
    {
        return X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
    }

    /** Get voxel occupancy */
    FORCEINLINE uint8 Get(int32 X, int32 Y, int32 Z) const
    {
        return Occupancy[Index(X, Y, Z)];
    }

    /** Set voxel occupancy */
    FORCEINLINE void Set(int32 X, int32 Y, int32 Z, uint8 Value)
    {
        Occupancy[Index(X, Y, Z)] = Value;
    }
};
