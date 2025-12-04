// Voxelizer.cpp
#include "VoxelGridActor.h"

#include "VoxelizationModule.h"

#include "VoxelGridVisualizationComponent.h"
#include "VoxelMesh.h"
#include "VoxelGrid.h"
#include "VoxelizationCore.h"
#include "Engine/StaticMeshActor.h"
#if WITH_EDITOR
#include "LandscapeDataAccess.h"
#endif


AVoxelGridActor::AVoxelGridActor()	
{
	PrimaryActorTick.bCanEverTick = true;

	auto SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneRootComponent);

	// Visual
	VoxelMeshVisualization = CreateDefaultSubobject<UVoxelGridVisualizationComponent>(TEXT("VoxelMeshVisualization"));
	VoxelMeshVisualization->SetupAttachment(RootComponent);
}

void AVoxelGridActor::FillImmovableVoxelGrid()
{
	UE_LOG(LogVoxelization, Display, TEXT("FillImmovableVoxelGrid"));
	FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();
	FVoxelMesh VoxelMesh;
	VoxelMesh.SetGridDim(VoxelGrid.GridDim);
	VoxelMesh.SetOrigin(VoxelGrid.Origin);
	VoxelMesh.SetVoxelSize(VoxelGrid.VoxelSize);

	VoxelMesh.ResetVoxelGrid();

	for (const auto& meshActor : ImmovableMeshes)
	{
		VoxelMesh.VoxelizeMeshInGrid(meshActor, false);
	}

	VoxelGrid.ImmovableMeshOccupancy = VoxelMesh.Occupancy;

	// Copy to GPU buffer
	bool ResizeOccupancyBuffer = VoxelGridResource->GridDim != VoxelGrid.GridDim;
    VoxelGridResource->GridDim = VoxelGrid.GridDim;

	auto BufferSize = VoxelGrid.ImmovableMeshOccupancy.Num();
	const auto* Occupancy = VoxelGrid.ImmovableMeshOccupancy.GetData();

	FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(CopyBufferToGPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			if (ResizeOccupancyBuffer)
			{
				VoxelGridResource->ImmovableMeshOccupancyBuffer.Release();
				VoxelGridResource->GridVelocityBuffer.Release();
				VoxelGridResource->ImmovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("VoxelBuffer"), sizeof(uint32), BufferSize);
				VoxelGridResource->GridVelocityBuffer.Initialize(RHICmdList, TEXT("GridVelocityBuffer"), sizeof(FVector3f), BufferSize);
				VoxelGridResource->GridDim = VoxelGrid.GridDim;
			}
			void* OutputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferSize * sizeof(uint32), RLM_WriteOnly));
			FMemory::Memcpy(OutputGPUBuffer, Occupancy, BufferSize * sizeof(uint32));

			// UnlockBuffer
			RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
		});
	FlushRenderingCommands();
}

void AVoxelGridActor::FillImmovableVoxelGridGPU()
{
    UE_LOG(LogVoxelization, Display, TEXT("FillImmovableVoxelGridGPU: Clear GPU buffer"));
	FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(FillImmovableVoxelGridGPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			if (VoxelGridResource->GridDim != VoxelGrid.GridDim)
			{
				VoxelGridResource->GridDim = VoxelGrid.GridDim;
				VoxelGridResource->UpdateRHI(RHICmdList);
			}
			ClearVoxelGridBuffer_RenderThread(RHICmdList, FVoxelGridResource::Get());
			DispatchVoxelizeMesh_RenderThread(RHICmdList, FVoxelGridResource::Get(), FVertexVelocityResource::Get(), ImmovableMeshes[0], VoxelGrid.Origin, VoxelGrid.GridDim);
		});
	FlushRenderingCommands();
}

void AVoxelGridActor::Visualize()
{
	VoxelMeshVisualization->UpdateVisualization(VoxelGrid);
}

void AVoxelGridActor::CopyBufferToCPU()
{
	FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();
	UE_LOG(LogVoxelization, Display, TEXT("VoxelGrid: Copying to CPU"));

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(CopyBufferToCPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			CopyVoxelGridToCPU_RenderThread(RHICmdList, VoxelGridResource, &VoxelGrid);

		});
	FlushRenderingCommands();

}

void AVoxelGridActor::CalculateVertexVelocity()
{
	float DeltaTime = 0.1f;

	UE_LOG(LogVoxelization, Display, TEXT("AVoxelGridActor::CalculateVertexVelocity"));
	FVertexVelocityResource* VertexVelocityResource = FVertexVelocityResource::Get();

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(CalculateVertexVelocity)([&](FRHICommandListImmediate& RHICmdList)
	{
        DispatchCalculateVertexVelocity_RenderThread(RHICmdList, VertexVelocityResource, ImmovableMeshes[0], DeltaTime);
	});
	FlushRenderingCommands();
}

void AVoxelGridActor::CopyVelocityToCPU()
{
	UE_LOG(LogVoxelization, Display, TEXT("AVoxelGridActor::CopyVelocityToCPU"));
	FVertexVelocityResource* VertexVelocityResource = FVertexVelocityResource::Get();

	auto BufferSize = VertexVelocityResource->GetVertexCount();
    VertexVelocitiesCPU.SetNumZeroed(BufferSize);
    auto* DstBufferPtr = VertexVelocitiesCPU.GetData();

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(CopyVelocityToCPU)([&](FRHICommandListImmediate& RHICmdList)
	{
		void* SrcBuffer = RHICmdList.LockBuffer(VertexVelocityResource->VertexVelocitiesBuffer.Buffer, 0, BufferSize * sizeof(FVector3f), RLM_ReadOnly);
        FMemory::Memcpy(DstBufferPtr, SrcBuffer, BufferSize * sizeof(FVector3f));
        RHICmdList.UnlockBuffer(VertexVelocityResource->VertexVelocitiesBuffer.Buffer);
	});
	FlushRenderingCommands();
}

void AVoxelGridActor::CopyVoxelGridVelocityToCPU()
{
	UE_LOG(LogVoxelization, Display, TEXT("AVoxelGridActor::CopyVoxelGridVelocityToCPU"));
	FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();

    auto BufferSize = VoxelGridResource->GetBufferLength();
	VoxelGridVelocitiesCPU.SetNumZeroed(BufferSize);
	auto* DstBufferPtr = VoxelGridVelocitiesCPU.GetData();

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(CopyVelocityToCPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			void* SrcBuffer = RHICmdList.LockBuffer(VoxelGridResource->GridVelocityBuffer.Buffer, 0, BufferSize * sizeof(FVector3f), RLM_ReadOnly);
			FMemory::Memcpy(DstBufferPtr, SrcBuffer, BufferSize * sizeof(FVector3f));
			RHICmdList.UnlockBuffer(VoxelGridResource->GridVelocityBuffer.Buffer);
		});
	FlushRenderingCommands();
}


void AVoxelGridActor::DemoTick(float DeltaTime)
{
	FillImmovableVoxelGridGPU();
}

void AVoxelGridActor::DemoVisualize()
{
	CopyBufferToCPU();
	Visualize();
}

void AVoxelGridActor::DemoTickCPU()
{
	FillImmovableVoxelGrid();
}

void AVoxelGridActor::DemoVisualizeCPU()
{
	Visualize();
}

void AVoxelGridActor::GPUDebug()
{
	FillImmovableVoxelGridGPU();
	CopyBufferToCPU();
	Visualize();
}

void AVoxelGridActor::CPUDebug()
{
	FillImmovableVoxelGrid();
	Visualize();
} 

void AVoxelGridActor::DebugFunc0()
{
	for (auto& i : VoxelGrid.ImmovableMeshOccupancy)
	{
		i = 0;
	}

    // Clear previous results
    OverlappingLandscapeComponents.Empty();

    // Validate landscape actor
    if (!LandscapeActor)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("DebugFunc0: Invalid LandscapeActor"));
        return;
    }

    // Calculate voxel grid bounding box in world space
    FVector GridOrigin = FVector(VoxelGrid.Origin);
    FBox VoxelGridBounds(GridOrigin, GridOrigin + FVector(VoxelGrid.GridDim) * VoxelGrid.VoxelSize);

    UE_LOG(LogVoxelization, Display, TEXT("DebugFunc0: VoxelGrid bounds - Min: (%f, %f, %f), Max: (%f, %f, %f)"),
        VoxelGridBounds.Min.X, VoxelGridBounds.Min.Y, VoxelGridBounds.Min.Z,
        VoxelGridBounds.Max.X, VoxelGridBounds.Max.Y, VoxelGridBounds.Max.Z);

	OverlappingLandscapeComponents = GetOverlappingLandscapeComponents(LandscapeActor, VoxelGridBounds);

	if (OverlappingLandscapeComponents.Num() == 0)
	{
		UE_LOG(LogVoxelization, Warning, TEXT("DebugFunc0: No overlapping landscape components found"));

		return;
	}

#if WITH_EDITOR

	VoxelGrid.SetGridDim(VoxelGrid.GridDim);	// Ensure data storage space is up-todate

	int WrittenVoxels = 0;
	for (ULandscapeComponent* LandscapeComponent : OverlappingLandscapeComponents)
	{
		// Copy height to grid
		FLandscapeComponentDataInterface DataInterface(LandscapeComponent);

		// Get component transform for coordinate conversion
		FTransform ComponentTransform = LandscapeComponent->GetComponentTransform();
		FTransform ComponentTransformInv = ComponentTransform.Inverse();

		// Get component bounds for validation
		FBox ComponentBounds = LandscapeComponent->Bounds.GetBox();

		UE_LOG(LogVoxelization, Display, TEXT("DebugFunc0: Filling voxel grid from landscape heightmap. Component size: %d vertices"),
			DataInterface.GetComponentSizeVerts());

		UE_LOG(LogVoxelization, Display, TEXT("DebugFunc0: Bound Min(%f, %f, %f), Bound Max(%f, %f, %f)"),
			ComponentBounds.Min.X, ComponentBounds.Min.Y, ComponentBounds.Min.Z,
			ComponentBounds.Max.X, ComponentBounds.Max.Y, ComponentBounds.Max.Z);

		for (int32 VoxelX = 0; VoxelX < VoxelGrid.GridDim.X; ++VoxelX)
		{
			for (int32 VoxelY = 0; VoxelY < VoxelGrid.GridDim.Y; ++VoxelY)
			{
				// Convert voxel grid position to world space
				FVector VoxelWorldPos = FVector(VoxelGrid.Origin) + FVector((VoxelX + 0.5) * VoxelGrid.VoxelSize, (VoxelY + 0.5) * VoxelGrid.VoxelSize, ComponentBounds.GetCenter().Z);

				// Check if this position is within the component bounds (rough check)
				if (!ComponentBounds.IsInside(VoxelWorldPos))
				{
					continue;
				}

				// Convert voxel position to landscape vertex indices
				FVector LocalPos = ComponentTransformInv.TransformPosition(VoxelWorldPos);
				float ScaleFactor = DataInterface.GetScaleFactor();
				float LocalX = LocalPos.X / ScaleFactor;
				float LocalY = LocalPos.Y / ScaleFactor;

				int32 ComponentSizeVerts = DataInterface.GetComponentSizeVerts();
				LocalX = FMath::Clamp(LocalX, 0.0f, static_cast<float>(ComponentSizeVerts - 1));
				LocalY = FMath::Clamp(LocalY, 0.0f, static_cast<float>(ComponentSizeVerts - 1));

				// Lerp height value
				int32 LocalX0 = FMath::FloorToInt(LocalX);
				int32 LocalY0 = FMath::FloorToInt(LocalY);
				int32 LocalX1 = FMath::Min(LocalX0 + 1, ComponentSizeVerts - 1);
				int32 LocalY1 = FMath::Min(LocalY0 + 1, ComponentSizeVerts - 1);

				float FracX = LocalX - LocalX0;
				float FracY = LocalY - LocalY0;

				FVector WorldVertex00 = DataInterface.GetWorldVertex(LocalX0, LocalY0);
				FVector WorldVertex10 = DataInterface.GetWorldVertex(LocalX1, LocalY0);
				FVector WorldVertex01 = DataInterface.GetWorldVertex(LocalX0, LocalY1);
				FVector WorldVertex11 = DataInterface.GetWorldVertex(LocalX1, LocalY1);

				float Height00 = WorldVertex00.Z;
				float Height10 = WorldVertex10.Z;
				float Height01 = WorldVertex01.Z;
				float Height11 = WorldVertex11.Z;

				float Height = FMath::Lerp(
					FMath::Lerp(Height00, Height10, FracX),
					FMath::Lerp(Height01, Height11, FracX),
					FracY
				);

				// Convert height to voxel grid Z coordinate
				float HeightRelativeToGrid = Height - VoxelGrid.Origin.Z;
				int32 MaxVoxelZ = FMath::Clamp(
					FMath::FloorToInt(HeightRelativeToGrid / VoxelGrid.VoxelSize),
					0,
					VoxelGrid.GridDim.Z - 1
				);

				// Fill voxels below height
				for (int32 VoxelZ = 0; VoxelZ <= MaxVoxelZ; ++VoxelZ)
				{
					int32 VoxelIndex = VoxelX + VoxelY * VoxelGrid.GridDim.X + VoxelZ * VoxelGrid.GridDim.X * VoxelGrid.GridDim.Y;
					if (VoxelIndex >= 0 && VoxelIndex < VoxelGrid.ImmovableMeshOccupancy.Num())
					{
						VoxelGrid.ImmovableMeshOccupancy[VoxelIndex] = 1;
						WrittenVoxels++;
					}
				}
			}
		}
	}
	UE_LOG(LogVoxelization, Display, TEXT("DebugFunc0: Complete. Written voxels %d"), WrittenVoxels);

#else
	UE_LOG(LogVoxelization, Warning, TEXT("DebugFunc0: LandscapeDataInterface only available in editor mode"));
#endif
}