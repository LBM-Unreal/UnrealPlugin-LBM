// VoxelMeshActor.cpp
#include "VoxelMeshActor.h"
#include "Voxelization.h"



AVoxelMeshActor::AVoxelMeshActor()
{
    PrimaryActorTick.bCanEverTick = false;
    MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    RootComponent = MeshComponent;

}

void AVoxelMeshActor::BeginPlay()
{
    Super::BeginPlay();
}

void AVoxelMeshActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (SourceMesh)
    {
        UE_LOG(LogVoxelization, Display, TEXT("Generating voxel mesh"))
        VoxelMesh.VoxelizeMesh(SourceMesh);
        BuildMeshFromVoxels();
    }
}

void AVoxelMeshActor::BuildMeshFromVoxels()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;


    const FVector Origin(VoxelMesh.Origin);
    const FVector Step(VoxelMesh.VoxelSize);
    const FVector GridDim(VoxelMesh.GridDim);
    const auto& VoxelData = VoxelMesh.Occupancy;

    constexpr int CubeVertsCount = 8;
    constexpr int CubeTrianglesCount = 36;

    auto AddCube = [&](const FVector& BasePos)
    {
        int vStart = Vertices.Num();
        FVector P[CubeVertsCount] = {
            BasePos + FVector(0,0,0)*Step,
            BasePos + FVector(1,0,0)*Step,
            BasePos + FVector(1,1,0)*Step,
            BasePos + FVector(0,1,0)*Step,
            BasePos + FVector(0,0,1)*Step,
            BasePos + FVector(1,0,1)*Step,
            BasePos + FVector(1,1,1)*Step,
            BasePos + FVector(0,1,1)*Step
        };
        Vertices.Append(P, CubeVertsCount);

        int F[CubeTrianglesCount] = {
            3,2,6, 3,6,7, // front
            1,0,4, 1,4,5, // back
            0,3,7, 0,7,4, // left
            2,1,5, 2,5,6, // right
            7,6,5, 7,5,4, // top
            0,1,2, 0,2,3  // bottom
           
        };

        for (int i = 0; i < CubeTrianglesCount; ++i)
        {
            Triangles.Add(vStart + F[i]);
        }
    };

    for (int Z=0; Z < GridDim.Z; ++Z)
    {
        for (int Y = 0; Y < GridDim.Y; ++Y)
        {
            for (int X = 0; X < GridDim.X; ++X)
            {
                int idx = X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
                if (VoxelData[idx] == 1)
                    AddCube(Origin + FVector(X, Y, Z) * Step);
            }
        }
    }
   

    MeshComponent->CreateMeshSection(0, Vertices, Triangles, {}, {}, {}, {}, false);
}


void AVoxelMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName PropertyName = PropertyChangedEvent.GetPropertyName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AVoxelMeshActor, SourceMesh) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FVoxelMesh, VoxelSize))
    {
        if (SourceMesh)
        {
            UE_LOG(LogVoxelization, Display, TEXT("Regenerating voxel mesh"))
            VoxelMesh.VoxelizeMesh(SourceMesh);
            BuildMeshFromVoxels();
        }
    }
}
