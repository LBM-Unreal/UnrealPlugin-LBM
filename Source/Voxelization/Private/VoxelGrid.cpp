#include "VoxelGrid.h"

FVoxelGridResource* FVoxelGridResource::GInstance = nullptr;

void FVoxelGridResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	int32 NumElements = GetBufferLength();
	ImmovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("ImmovableOccupancyBuffer"), sizeof(uint32), NumElements);
	ImmovableMeshNormalBuffer.Initialize(RHICmdList, TEXT("ImmovableMeshNormalBuffer"), sizeof(FVector4f), NumElements);
	ImmovableMeshVelocityBuffer.Initialize(RHICmdList, TEXT("ImmovableMeshVelocityBuffer"), sizeof(FVector3f), NumElements);

	MovableMeshOccupancyBuffer.Initialize(RHICmdList, TEXT("MovableOccupancyBuffer"), sizeof(uint32), NumElements);
	GridVelocityBuffer.Initialize(RHICmdList, TEXT("GridVelocityBuffer"), sizeof(FVector3f), NumElements);
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