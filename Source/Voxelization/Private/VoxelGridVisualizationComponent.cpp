#include "VoxelGridVisualizationComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "VoxelGrid.h"
#include "ProceduralMeshComponent.h"
#include "Components/BoxComponent.h"

UVoxelGridVisualizationComponent::UVoxelGridVisualizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVoxelGridVisualizationComponent::OnRegister()
{
    Super::OnRegister();

    if (!ProceduralMesh)
    {
        ProceduralMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("VoxelGridProceduralMesh"));
        ProceduralMesh->SetupAttachment(this);
        ProceduralMesh->RegisterComponent();
        ProceduralMesh->SetAffectDistanceFieldLighting(false);
    }

    if (!VoxelGridArea)
    {
        VoxelGridArea = NewObject<UBoxComponent>(GetOwner(), TEXT("VoxelGridArea"));
        VoxelGridArea->SetupAttachment(this);

        VoxelGridArea->SetVisibility(true);
        VoxelGridArea->SetHiddenInGame(true);
        VoxelGridArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        VoxelGridArea->RegisterComponent();
    }
}


void UVoxelGridVisualizationComponent::UpdateVisualization(const FVoxelGrid& VoxelGrid)
{
    // Visualize bound

    // Visualize mesh
    TArray<FVector> Vertices;
    TArray<int32> Triangles;


    const FVector Origin(VoxelGrid.Origin);
    const FVector Step(VoxelGrid.VoxelSize);
    const FVector GridDim(VoxelGrid.GridDim);
    const auto& VoxelData = VoxelGrid.ImmovableMeshOccupancy;

    if (VoxelData.Num() != GridDim.X * GridDim.Y * GridDim.Z)
    {
        UE_LOG(LogTemp, Warning, TEXT("VoxelGridVisualizationComponent::UpdateVisualization: Voxel data size mismatch"));
        return;
    }

    constexpr int CubeVertsCount = 8;
    constexpr int CubeTrianglesCount = 36;

    auto AddCube = [&](const FVector& BasePos)
        {
            int vStart = Vertices.Num();
            FVector P[CubeVertsCount] = {
                BasePos + FVector(0,0,0) * Step,
                BasePos + FVector(1,0,0) * Step,
                BasePos + FVector(1,1,0) * Step,
                BasePos + FVector(0,1,0) * Step,
                BasePos + FVector(0,0,1) * Step,
                BasePos + FVector(1,0,1) * Step,
                BasePos + FVector(1,1,1) * Step,
                BasePos + FVector(0,1,1) * Step
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

    for (int Z = 0; Z < GridDim.Z; ++Z)
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

    ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, {}, {}, {}, {}, false);
}
