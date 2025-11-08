#include "VoxelMesh.h"
#include "StaticMeshResources.h"
#include "Math/UnrealMathUtility.h"

// Helper function for voxel triangle intersection 
bool TriangleAABBTest(FVector3f V0, FVector3f V1, FVector3f V2, FVector3f BoxCenter, FVector3f BoxHalfWidth);
bool AxisTestX();

void FVoxelMesh::VoxelizeMesh(const UStaticMesh* Mesh)
{
    if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
    {
        // Mesh->GetRenderData()->IsInitialized(); ?
        UE_LOG(LogTemp, Warning, TEXT("FVoxelMesh::VoxelizeMesh: Invalid mesh"));
        return;
    }

    FBox meshBound = Mesh->GetBoundingBox();
    FVector meshBoundCenter = meshBound.GetCenter();
    FVector meshBoundExtent = meshBound.GetExtent();

    // Resize grid to contain the mesh
    GridDim.X = 2 * FMath::CeilToInt(meshBoundExtent.X / VoxelSize);
    GridDim.Y = 2 * FMath::CeilToInt(meshBoundExtent.Y / VoxelSize);
    GridDim.Z = 2 * FMath::CeilToInt(meshBoundExtent.Z / VoxelSize);

    // Set VoxelMesh's origin to the mesh bound min
    Origin = FVector3f(meshBoundCenter - FVector(GridDim) * 0.5f);

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
        V0 = (V0 - Origin) / VoxelSize;
        V1 = (V1 - Origin) / VoxelSize;
        V2 = (V2 - Origin) / VoxelSize;

        // Compute triangle bounds in voxel space for intersection efficiency
        FIntVector MinGridBound{};
        FVector3f MinV = FMath::Min3(V0, V1, V2);
        MinGridBound.X = FMath::FloorToInt(MinV.X);
        MinGridBound.Y = FMath::FloorToInt(MinV.Y);
        MinGridBound.Z = FMath::FloorToInt(MinV.Z);

        FIntVector MaxGridBound{};
        FVector3f MaxV = FMath::Max3(V0, V1, V2);
        MaxGridBound.X = FMath::FloorToInt(MaxV.X);
        MaxGridBound.Y = FMath::FloorToInt(MaxV.Y);
        MaxGridBound.Z = FMath::FloorToInt(MaxV.Z);

        // Naively traverse the voxel bounds and test intersection
        for (int Z = MinV.Z; Z <= MaxV.Z; ++Z)
        {
            for (int Y = MinV.Y; Y <= MaxV.Y; ++Y)
            {
                for (int X = MinV.X; X <= MaxV.X; ++X)
                {
                    const FVector3f VoxelHalfSize = FVector3f(VoxelSize * 0.5f);
                	FVector3f VoxelCenter = FVector3f(Origin) + FVector3f(X, Y, Z) * VoxelSize;

                    if (TriangleAABBTest(V0, V1, V2, VoxelCenter, VoxelHalfSize))
                    {
                        Set(X, Y, Z, 1);
                    }
                }
            }
        }
    }
}


// We use the algorithm from "Fast 3D Triangle-Box Overlap Testing" by Tomas Akenine-Moller 
// but skip the redundant AABB's x, y, z axis tests
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

    float BoxMaxDistToTriangle = FVector3f::DotProduct(TriangleNormal, BoxMaxWithTriangleNormal);
    float BoxMinDistToTriangle = FVector3f::DotProduct(TriangleNormal, BoxMinWithTriangleNormal);
	float TriangleDistToCenter = FVector3f::DotProduct(TriangleNormal, V0);

    if (BoxMinDistToTriangle > TriangleDistToCenter || BoxMaxDistToTriangle < TriangleDistToCenter)
    {
        return false;
    }

    /* Test the 9 cross products of triangle edges and aabb axes */
    FVector3f E0F0 = FVector3f::CrossProduct(E0, F0);
    FVector3f E0F1 = FVector3f::CrossProduct(E0, F1);
    FVector3f E0F2 = FVector3f::CrossProduct(E0, F2);
    FVector3f E1F0 = FVector3f::CrossProduct(E1, F0);
    FVector3f E1F1 = FVector3f::CrossProduct(E1, F1);
    FVector3f E1F2 = FVector3f::CrossProduct(E1, F2);
    FVector3f E2F0 = FVector3f::CrossProduct(E2, F0);
    FVector3f E2F1 = FVector3f::CrossProduct(E2, F1);
    FVector3f E2F2 = FVector3f::CrossProduct(E2, F2);

    TArray<FVector3f> CrossDir {
        E0F0, E0F1, E0F2,
        E1F0, E1F1, E1F2,
        E2F0, E2F1, E2F2
    };


    for (const FVector3f& Axis : CrossDir)
    {
        float P0 = FVector3f::DotProduct(V0, Axis);
        float P1 = FVector3f::DotProduct(V1, Axis);
        float P2 = FVector3f::DotProduct(V2, Axis);
        float R = FVector3f::DotProduct(BoxHalfWidth, FMath::Abs(Axis));
        if (FMath::Min3(P0, P1, P2) > R || FMath::Max3(P0, P1, P2) < -R)
        {
            return false;
        }
    }

    return true;
}