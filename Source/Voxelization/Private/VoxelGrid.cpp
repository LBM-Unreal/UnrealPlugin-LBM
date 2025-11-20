#include "VoxelGrid.h"

FVoxelGridResource* FVoxelGridResource::GInstance = nullptr;

void FVoxelGridResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	ImmovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("ImmovableVoxelGridBuffer"), sizeof(uint32), GridDim.X * GridDim.Y * GridDim.Z);
}

void FVoxelGridResource::ReleaseRHI()
{
    ImmovableMeshOccupancyBuffer.Release();
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