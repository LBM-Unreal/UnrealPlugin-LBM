#include "VoxelGridVisualizationComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "VoxelGrid.h"
#include "ProceduralMeshComponent.h"
#include "VoxelizationModule.h"
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
    UE_LOG(LogVoxelization, Display, TEXT("VoxelGridVisualizationComponent::UpdateVisualization: Converting voxel to mesh..."));
    // Visualize bound

    // Visualize mesh
    TArray<FVector> Vertices{};
    TArray<FColor> VertexColors{};
    TArray<int32> Triangles{};

    const FVector Origin(VoxelGrid.Origin);
    const FVector Step(VoxelGrid.VoxelSize);
    const FIntVector GridDim(VoxelGrid.GridDim);
    const auto& VoxelData = VoxelGrid.ImmovableMeshOccupancy;

    if (VoxelData.Num() != GridDim.X * GridDim.Y * GridDim.Z)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelGridVisualizationComponent::UpdateVisualization: Voxel data size mismatch"));
        return;
    }

    // Helper function to check if a neighbor voxel exists and is solid
    auto IsVoxelSolid = [&](int32 X, int32 Y, int32 Z) -> bool
    {
        if (X < 0 || X >= GridDim.X || Y < 0 || Y >= GridDim.Y || Z < 0 || Z >= GridDim.Z)
        {
            return false; // Out of bounds = no neighbor = face is exposed
        }
        int32 idx = X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
        return VoxelData[idx] == 1;
    };

    // Helper function to add a quad face
    auto AddQuad = [&](const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3, const FColor& Color)
    {
        int32 vStart = Vertices.Num();
        Vertices.Add(V0);
        Vertices.Add(V1);
        Vertices.Add(V2);
        Vertices.Add(V3);

        VertexColors.Add(Color);
        VertexColors.Add(Color);
        VertexColors.Add(Color);
        VertexColors.Add(Color);
        
        // Add two triangles for the quad (counter-clockwise winding)
        Triangles.Add(vStart + 0);
        Triangles.Add(vStart + 1);
        Triangles.Add(vStart + 2);
        Triangles.Add(vStart + 0);
        Triangles.Add(vStart + 2);
        Triangles.Add(vStart + 3);
    };

    int WrittenVoxels = 0;
    // Iterate through all voxels and only add exposed faces
    for (int32 Z = 0; Z < GridDim.Z; ++Z)
    {
        for (int32 Y = 0; Y < GridDim.Y; ++Y)
        {
            for (int32 X = 0; X < GridDim.X; ++X)
            {
                int32 idx = X + Y * GridDim.X + Z * GridDim.X * GridDim.Y;
                if (VoxelData[idx] != 1)
                {
                    continue; // Skip empty voxels
                }

                FVector BasePos = Origin + FVector(X, Y, Z) * Step;
                
                // Define the 8 vertices of the cube
                FVector V000 = BasePos + FVector(0, 0, 0) * Step;
                FVector V100 = BasePos + FVector(1, 0, 0) * Step;
                FVector V110 = BasePos + FVector(1, 1, 0) * Step;
                FVector V010 = BasePos + FVector(0, 1, 0) * Step;
                FVector V001 = BasePos + FVector(0, 0, 1) * Step;
                FVector V101 = BasePos + FVector(1, 0, 1) * Step;
                FVector V111 = BasePos + FVector(1, 1, 1) * Step;
                FVector V011 = BasePos + FVector(0, 1, 1) * Step;

                FVector4f TriangleNormal = VoxelGrid.ImmovableMeshNormal[idx];
                FColor FVertexColor = FColor(FMath::Max(0, TriangleNormal.X) * 255, 
                    FMath::Max(0, TriangleNormal.Y) * 255,
                    FMath::Max(0, TriangleNormal.Z) * 255);
            	// Only add faces that are exposed (no solid neighbor)
                // Front face (+Y direction)
                if (!IsVoxelSolid(X, Y + 1, Z))
                {
                    AddQuad(V010, V110, V111, V011, FVertexColor);
                }

                // Back face (-Y direction)
                if (!IsVoxelSolid(X, Y - 1, Z))
                {
                    AddQuad(V100, V000, V001, V101, FVertexColor);
                }

                // Right face (+X direction)
                if (!IsVoxelSolid(X + 1, Y, Z))
                {
                    AddQuad(V110, V100, V101, V111, FVertexColor);
                }

                // Left face (-X direction)
                if (!IsVoxelSolid(X - 1, Y, Z))
                {
                    AddQuad(V000, V010, V011, V001, FVertexColor);
                }

                // Top face (+Z direction)
                if (!IsVoxelSolid(X, Y, Z + 1))
                {
                    AddQuad(V011, V111, V101, V001, FVertexColor);
                }

                // Bottom face (-Z direction)
                if (!IsVoxelSolid(X, Y, Z - 1))
                {
                    AddQuad(V000, V100, V110, V010, FVertexColor);
                }

                WrittenVoxels++;
            }
        }
    }
    ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, {}, {}, VertexColors, {}, false);
    UE_LOG(LogVoxelization, Display, TEXT("VoxelGridVisualizationComponent::UpdateVisualization: Mesh generated with %d voxels"), WrittenVoxels);
}
