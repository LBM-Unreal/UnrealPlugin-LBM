#include "VoxelMesh.h"
#include "StaticMeshResources.h"
#include "VoxelizationModule.h"
#include "Engine/StaticMeshActor.h"
#include "Math/UnrealMathUtility.h"

// Helper function for voxel triangle intersection 
bool TriangleAABBTest(FVector3f V0, FVector3f V1, FVector3f V2, FVector3f BoxCenter, FVector3f BoxHalfWidth);


void FVoxelMesh::SetGridDim(const FIntVector Dim)
{
    this->GridDim = Dim;
    ReallocateVoxelGrid();
}

// Adjust the voxel grid to contain the StaticMesh, then voxelize the mesh into the grid
void FVoxelMesh::VoxelizeMesh(const UStaticMesh* Mesh)
{
    // Validation and setup
    if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
    {
        // Mesh->GetRenderData()->IsInitialized(); ?
        UE_LOG(LogVoxelization, Warning, TEXT("FVoxelMesh::VoxelizeMesh: Invalid mesh"));
        return;
    }

    if (VoxelSize < 0)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("Voxelization: invalid voxel size, got %f"), VoxelSize);
        return;
    }

    FBox meshBound = Mesh->GetBoundingBox();
    FVector meshBoundCenter = meshBound.GetCenter();
    FVector meshBoundExtent = meshBound.GetExtent() + 0.01f; // Slightly expand to avoid precision issues in later indexing

    // Resize grid to contain the mesh, and (re)allocate space for voxels
    GridDim.X = 2 * FMath::CeilToInt(meshBoundExtent.X / VoxelSize);
    GridDim.Y = 2 * FMath::CeilToInt(meshBoundExtent.Y / VoxelSize);
    GridDim.Z = 2 * FMath::CeilToInt(meshBoundExtent.Z / VoxelSize);

    // Set VoxelMesh's origin to the mesh bound min
    Origin = FVector3f(meshBoundCenter - FVector(GridDim) * 0.5f * VoxelSize);

    ReallocateVoxelGrid();
    ResetVoxelGrid();

    // Extract raw positions
    const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
    const FPositionVertexBuffer& VB = LOD.VertexBuffers.PositionVertexBuffer;
    const FRawStaticIndexBuffer& IB = LOD.IndexBuffer;

    // Voxelize
    for (int32 I = 0; I < IB.GetNumIndices(); I += 3)
    {
        FVector3f V0 = VB.VertexPosition(IB.GetIndex(I + 0));
        FVector3f V1 = VB.VertexPosition(IB.GetIndex(I + 1));
        FVector3f V2 = VB.VertexPosition(IB.GetIndex(I + 2));

        // Transform to voxel space (0..GridSize)
        FVector3f V0VoxelSpace = (V0 - Origin) / VoxelSize;
        FVector3f V1VoxelSpace = (V1 - Origin) / VoxelSize;
        FVector3f V2VoxelSpace = (V2 - Origin) / VoxelSize;

        // Compute triangle bounds in voxel space for intersection efficiency
        FIntVector MaxGridBound{};
    	FIntVector MinGridBound{};

        FVector3f MinV = FVector3f::Min3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);
        FVector3f MaxV = FVector3f::Max3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);

        /*MinGridBound.X = FMath::FloorToInt(MinV.X);
        MinGridBound.Y = FMath::FloorToInt(MinV.Y);
        MinGridBound.Z = FMath::FloorToInt(MinV.Z);

        MaxGridBound.X = FMath::FloorToInt(MaxV.X);
        MaxGridBound.Y = FMath::FloorToInt(MaxV.Y);
        MaxGridBound.Z = FMath::FloorToInt(MaxV.Z);*/
        MinGridBound.X = FMath::Clamp(FMath::FloorToInt(MinV.X), 0, GridDim.X - 1);
        MinGridBound.Y = FMath::Clamp(FMath::FloorToInt(MinV.Y), 0, GridDim.Y - 1);
        MinGridBound.Z = FMath::Clamp(FMath::FloorToInt(MinV.Z), 0, GridDim.Z - 1);

        MaxGridBound.X = FMath::Clamp(FMath::FloorToInt(MaxV.X), 0, GridDim.X - 1);
        MaxGridBound.Y = FMath::Clamp(FMath::FloorToInt(MaxV.Y), 0, GridDim.Y - 1);
        MaxGridBound.Z = FMath::Clamp(FMath::FloorToInt(MaxV.Z), 0, GridDim.Z - 1);

    	UE_LOG(LogVoxelization, Display, TEXT("Voxelizing Triangle %d / %d"), I / 3, IB.GetNumIndices() / 3 - 1)

        // Naively traverse the voxel bounds and test intersection
        for (int Z = MinGridBound.Z; Z <= MaxGridBound.Z; ++Z)
        {
            for (int Y = MinGridBound.Y; Y <= MaxGridBound.Y; ++Y)
            {
                for (int X = MinGridBound.X; X <= MaxGridBound.X; ++X)
                {
                    const FVector3f VoxelHalfSize = FVector3f(VoxelSize * 0.5f);
                	FVector3f VoxelCenter = Origin + VoxelSize * FVector3f(X, Y, Z) + VoxelHalfSize;

                    if (TriangleAABBTest(V0, V1, V2, VoxelCenter, VoxelHalfSize))
                    {
                        Set(X, Y, Z, 1);
                    }
                }
            }
        }
    }
}


// Voxelize the StaticMeshActor into the current voxel grid, the actor's transformation is applied to the mesh vertices
// If ResetGrid is false, the new voxelization will be merged into the existing grid
void FVoxelMesh::VoxelizeMeshInGrid(const AStaticMeshActor* MeshActor, bool ResetGrid)
{
    // Validation and setup
    if (!MeshActor || !MeshActor->GetStaticMeshComponent())
    {
        UE_LOG(LogVoxelization, Warning, TEXT("FVoxelMesh::VoxelizeMeshInGrid: Invalid StaticMeshActor"));
        return;
    }

    UStaticMesh* Mesh = MeshActor->GetStaticMeshComponent()->GetStaticMesh();
    if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
    {
        // Mesh->GetRenderData()->IsInitialized(); ?
        UE_LOG(LogVoxelization, Warning, TEXT("FVoxelMesh::VoxelizeMesh: Invalid mesh"));
        return;
    }

    if (VoxelSize < 0)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("Voxelization: invalid voxel size, got %f"), VoxelSize);
        return;
    }

    if (GridDim.X <= 0 || GridDim.Y <= 0 || GridDim.Z <= 0)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("Voxelization: invalid GridDim, got [%d, %d, %d]"), GridDim.X, GridDim.Y, GridDim.Z);
        return;
    }

    if (ResetGrid)
    {
        ResetVoxelGrid();
    }

    // Extract raw positions
    const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
    const FPositionVertexBuffer& VB = LOD.VertexBuffers.PositionVertexBuffer;
    const FRawStaticIndexBuffer& IB = LOD.IndexBuffer;

    const FTransform LocalToWorld = MeshActor->GetActorTransform();

    // Voxelize
    for (int32 I = 0; I < IB.GetNumIndices(); I += 3)
    {
        FVector3f V0Local = VB.VertexPosition(IB.GetIndex(I + 0));
        FVector3f V1Local = VB.VertexPosition(IB.GetIndex(I + 1));
        FVector3f V2Local = VB.VertexPosition(IB.GetIndex(I + 2));

        FVector3f V0World = FVector3f(LocalToWorld.TransformPosition(FVector(V0Local)));
        FVector3f V1World = FVector3f(LocalToWorld.TransformPosition(FVector(V1Local)));
        FVector3f V2World = FVector3f(LocalToWorld.TransformPosition(FVector(V2Local)));

        // Transform to voxel space (0..GridSize)
        FVector3f V0VoxelSpace = (V0World - Origin) / VoxelSize;
        FVector3f V1VoxelSpace = (V1World - Origin) / VoxelSize;
        FVector3f V2VoxelSpace = (V2World - Origin) / VoxelSize;

        // Compute triangle bounds in voxel space for intersection efficiency
        FIntVector MaxGridBound{};
        FIntVector MinGridBound{};

        FVector3f MinV = FVector3f::Min3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);
        FVector3f MaxV = FVector3f::Max3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);

        MinGridBound.X = FMath::Clamp(FMath::FloorToInt(MinV.X), 0, GridDim.X - 1);
        MinGridBound.Y = FMath::Clamp(FMath::FloorToInt(MinV.Y), 0, GridDim.Y - 1);
        MinGridBound.Z = FMath::Clamp(FMath::FloorToInt(MinV.Z), 0, GridDim.Z - 1);

        MaxGridBound.X = FMath::Clamp(FMath::FloorToInt(MaxV.X), 0, GridDim.X - 1);
        MaxGridBound.Y = FMath::Clamp(FMath::FloorToInt(MaxV.Y), 0, GridDim.Y - 1);
        MaxGridBound.Z = FMath::Clamp(FMath::FloorToInt(MaxV.Z), 0, GridDim.Z - 1);

        UE_LOG(LogVoxelization, Display, TEXT("Voxelizing Triangle %d / %d"), I / 3, IB.GetNumIndices() / 3 - 1)
        // Naively traverse the voxel bounds and test intersection
        for (int Z = MinGridBound.Z; Z <= MaxGridBound.Z; ++Z)
        {
            for (int Y = MinGridBound.Y; Y <= MaxGridBound.Y; ++Y)
            {
                for (int X = MinGridBound.X; X <= MaxGridBound.X; ++X)
                {
                    const FVector3f VoxelHalfSize = FVector3f(VoxelSize * 0.5f);
                    FVector3f VoxelCenter = Origin + VoxelSize * FVector3f(X, Y, Z) + VoxelHalfSize;

                    if (TriangleAABBTest(V0World, V1World, V2World, VoxelCenter, VoxelHalfSize))
                    {
                        Set(X, Y, Z, 1);
                    }
                }
            }
        }
    }
}


// We use the algorithm from "Fast 3D Triangle-Box Overlap Testing" by Tomas Akenine-Moller,
// but skipped the redundant AABB's x, y, z axis tests.
// We also replace the hard coded separating axis test with a more general form to simplify the implementation,
// will replace them if encounter performance issue.
bool TriangleAABBTest(FVector3f V0, FVector3f V1, FVector3f V2, FVector3f BoxCenter, FVector3f BoxHalfWidth)
{
    // Convert triangle to AABB's local space
    V0 -= BoxCenter;
    V1 -= BoxCenter;
    V2 -= BoxCenter;

    // aabb axis
    FVector3f E0{ 1, 0, 0 };
    FVector3f E1{ 0, 1, 0 };
    FVector3f E2{ 0, 0, 1 };

    // Triangle edges
    FVector3f F0 = V1 - V0;
    FVector3f F1 = V2 - V1;
    FVector3f F2 = V0 - V2;

    /* Fast plane-AABB overlap test with the triangle normal axis */
    // find the two most extreme points along the triangle normal
    FVector3f TriangleNormal = FVector3f::CrossProduct(F0, F1); // no need to normalize it for we are only using for comparisons
    FVector3f BoxMaxWithTriangleNormal
	{
		TriangleNormal.X > 0 ? BoxHalfWidth.X : -BoxHalfWidth.X,
        TriangleNormal.Y > 0 ? BoxHalfWidth.Y : -BoxHalfWidth.Y,
        TriangleNormal.Z > 0 ? BoxHalfWidth.Z : -BoxHalfWidth.Z
    };
    FVector3f BoxMinWithTriangleNormal = -BoxMaxWithTriangleNormal;

    float BoxMaxDistToTriangle = FVector3f::DotProduct(BoxMaxWithTriangleNormal - V0, TriangleNormal);
    float BoxMinDistToTriangle = FVector3f::DotProduct(BoxMinWithTriangleNormal - V0, TriangleNormal);

    if (BoxMinDistToTriangle > 0 || BoxMaxDistToTriangle < 0)
    {
        return false;
    }

    /* Test the 9 cross products of triangle edges and aabb axes (separating axes) */
    TArray<FVector3f> SeparatingAxes {
        FVector3f::CrossProduct(E0, F0),
        FVector3f::CrossProduct(E0, F1),
        FVector3f::CrossProduct(E0, F2),
        FVector3f::CrossProduct(E1, F0),
        FVector3f::CrossProduct(E1, F1),
        FVector3f::CrossProduct(E1, F2),
        FVector3f::CrossProduct(E2, F0),
        FVector3f::CrossProduct(E2, F1),
        FVector3f::CrossProduct(E2, F2),
    };

    for (const FVector3f& Axis : SeparatingAxes)
    {
        float P0 = FVector3f::DotProduct(V0, Axis);
        float P1 = FVector3f::DotProduct(V1, Axis);
        float P2 = FVector3f::DotProduct(V2, Axis);
        float R = FVector3f::DotProduct(BoxHalfWidth, Axis.GetAbs());
        if (FMath::Min3(P0, P1, P2) > R || FMath::Max3(P0, P1, P2) < -R)
        {
            return false;
        }
    }

    return true;
}
