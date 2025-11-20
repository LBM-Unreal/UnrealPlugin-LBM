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
		ENQUEUE_RENDER_COMMAND(CopyOccupancyBuffer)([&](FRHICommandListImmediate& RHICmdList)
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
		// TODO: expand to multiple meshes
		const AStaticMeshActor* meshActor = ImmovableMeshes[0];
		const UStaticMesh* Mesh = meshActor->GetStaticMeshComponent()->GetStaticMesh();
		const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
		const FPositionVertexBuffer& VB = LOD.VertexBuffers.PositionVertexBuffer;
		const FRawStaticIndexBuffer& IB = LOD.IndexBuffer;

		FVoxelizationShaderParameters params{};
		params.VoxelGridBuffer = FVoxelGridResource::Get()->ImmovableMeshOccupancyBuffer.UAV;
		params.TriangleVerts = VB.GetSRV();
		

		// Create an SRV for index manually
		FBufferRHIRef IndexRHI = IB.IndexBufferRHI;
		auto a = IB.GetArrayView();
		const bool bIs32Bit = IB.Is32Bit();
		
		params.GridDim = VoxelGrid.GridDim;
	

	// DispatchVoxelizeMesh_RenderThread
	if (!IB.Is32Bit())
	{

	}
}

void AVoxelGridActor::Visualize()
{
	VoxelMeshVisualization->UpdateVisualization(VoxelGrid);
}

void AVoxelGridActor::TriggerDebugFunc()
{
	UE_LOG(LogVoxelization, Display, TEXT("VoxelGrid: Copying to CPU"));
    CopyVoxelGridToCPU_RenderThread(VoxelGrid.ImmovableMeshOccupancy);
}

