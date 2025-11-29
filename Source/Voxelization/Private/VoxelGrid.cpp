#include "VoxelGrid.h"

FVoxelGridResource* FVoxelGridResource::GInstance = nullptr;

void FVoxelGridResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	ImmovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("ImmovableOccupancyBuffer"), sizeof(uint32), GridDim.X * GridDim.Y * GridDim.Z);
	MovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("MovableOccupancyBuffer"), sizeof(uint32), GridDim.X * GridDim.Y * GridDim.Z);
	GridVelocityBuffer.Initialize(RHICmdList, TEXT("GridVelocityBuffer"), sizeof(FVector3f), GridDim.X * GridDim.Y * GridDim.Z);
}

void FVoxelGridResource::ReleaseRHI()
{
    ImmovableMeshOccupancyBuffer.Release();
	MovableMeshOccupancyBuffer.Release();
    GridVelocityBuffer.Release();
}

FVoxelGridResource* FVoxelGridResource::Get()
{
	if (!GInstance)
	{
		GInstance = new FVoxelGridResource();
	}
	return GInstance;
}


FVoxelGridResource* GetVoxelGridResource() {
	return FVoxelGridResource::Get();
}