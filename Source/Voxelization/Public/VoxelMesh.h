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

	/** Size of one voxel in world units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
    float VoxelSize = 1.0f;

    /** Number of voxels along each axis (resolution) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
    FIntVector GridDim = FIntVector::ZeroValue;

    /** World-space origin (corner) of the grid */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
    FVector3f Origin = FVector3f::ZeroVector;

    /** Voxel data */
    UPROPERTY(VisibleAnywhere, Category = "Voxel")
    TArray<uint32> Occupancy;


    // ------------------------------------------------------------------------
    // Set up
    // ------------------------------------------------------------------------
    VOXELIZATION_API void  SetGridDim(const FIntVector Dim);

    FORCEINLINE VOXELIZATION_API void SetVoxelSize(const float Size)
    {
        this->VoxelSize = Size;
    }

    FORCEINLINE VOXELIZATION_API void SetOrigin(const FVector3f Min)
    {
        Origin = Min;
    }

    /** Clear grid to empty (0) */
    FORCEINLINE VOXELIZATION_API void ResetVoxelGrid()
    {
        for (uint32& V : Occupancy)
        {
            V = 0;
        }
    }

    // ------------------------------------------------------------------------
	// Core operations
	// ------------------------------------------------------------------------
    void VoxelizeMesh(const UStaticMesh* Mesh);
    void VOXELIZATION_API VoxelizeMeshInGrid(const AStaticMeshActor* MeshActor, bool ResetGrid = true);

    // ------------------------------------------------------------------------
    // Basic operations
    // ------------------------------------------------------------------------

    /** Total voxel count */
    FORCEINLINE int32 NumVoxels() const
    {
        return GridDim.X * GridDim.Y * GridDim.Z;
    }

    /** Resize Occupancy to match GridDim */
    FORCEINLINE void ReallocateVoxelGrid()
    {
        Occupancy.SetNumZeroed(NumVoxels());
    }

    /** Convert 3D coordinates to linear index */  
    FORCEINLINE int32 Index(int32 X, int32 Y, int32 Z) const
    {
        return X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
    }

    /** Get voxel occupancy */
    FORCEINLINE uint32 Get(int32 X, int32 Y, int32 Z) const
    {
        return Occupancy[Index(X, Y, Z)];
    }

    /** Set voxel occupancy */
    FORCEINLINE void Set(int32 X, int32 Y, int32 Z, uint32 Value)
    {
        Occupancy[Index(X, Y, Z)] = Value;
    }
};
