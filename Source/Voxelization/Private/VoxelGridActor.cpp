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

	FlushRenderingCommands();
	ENQUEUE_RENDER_COMMAND(FillImmovableVoxelGridGPU)([&](FRHICommandListImmediate& RHICmdList)
		{
			if (FVoxelGridResource::Get()->GridDim != VoxelGrid.GridDim)
			{
				FVoxelGridResource::Get()->GridDim = VoxelGrid.GridDim;
				FVoxelGridResource::Get()->UpdateRHI(RHICmdList);
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