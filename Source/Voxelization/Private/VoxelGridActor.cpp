// Voxelizer.cpp
#include "VoxelGridActor.h"

#include "VoxelizationModule.h"

#include "VoxelGridVisualizationComponent.h"
#include "VoxelMesh.h"
#include "VoxelGrid.h"
#include "VoxelizationCore.h"
#include "Engine/StaticMeshActor.h"

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
				VoxelGridResource->ImmovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("VoxelBuffer"), sizeof(uint32), BufferSize);
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

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(FillImmovableVoxelGridGPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			if (FVoxelGridResource::Get()->GridDim != VoxelGrid.GridDim)
			{
				FVoxelGridResource::Get()->GridDim = VoxelGrid.GridDim;
				FVoxelGridResource::Get()->UpdateRHI(RHICmdList);
			}
			ClearVoxelGridBuffer_RenderThread(RHICmdList, FVoxelGridResource::Get());
			DispatchVoxelizeMesh_RenderThread(RHICmdList, FVoxelGridResource::Get(), ImmovableMeshes[0], VoxelGrid.Origin, VoxelGrid.GridDim);
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

