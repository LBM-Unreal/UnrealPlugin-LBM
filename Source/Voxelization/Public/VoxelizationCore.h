// VoxelizationCore.h
#pragma once
#include "RHIResources.h"
#include "RenderResource.h"
#include "Landscape.h"
#include "VoxelGrid.h"

/* ---------------------------------------- Resources ---------------------------------------------------- */
// FVertexVelocityResource
class VOXELIZATION_API FVertexVelocityResource : public FRenderResource
{
	static FVertexVelocityResource* GInstance;

    // Parameters
	uint32 VertexCount;

public:
	// Render Resources
	FRWBufferStructured VertexPositionsBuffer;
	FRWBufferStructured VertexVelocitiesBuffer;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
	virtual void ReleaseRHI() override;

	static FVertexVelocityResource* Get();

	void SetVertexCount(FRHICommandListBase& RHICmdList, uint32 InVertexCount);

	FORCEINLINE uint32 GetVertexCount() const
	{
		return VertexCount;
	}

private:
	FVertexVelocityResource() : VertexCount(3)
	{
		ENQUEUE_RENDER_COMMAND(FCreateVertexVelocityRes)([this](FRHICommandListImmediate& RHICmdList)
		{
			this->InitResource(RHICmdList);
		});
	};
};


/* ---------------------------------------- Utility Functions ---------------------------------------------------- */
void VOXELIZATION_API DispatchCopyImmovableMeshVoxelGridToSim_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelGridResource* VoxelGridResource,
    FUnorderedAccessViewRHIRef DstUAV,
    FIntVector3 SimDimension,
    uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);


void VOXELIZATION_API DispatchVoxelizeMesh_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelGridResource* VoxelGridResource,
    class FVertexVelocityResource* VertexVelocityResource,
    AStaticMeshActor* SMActor,
    FVector3f VoxelGridOrigin,
    FIntVector3 VoxelGridDimension);


void VOXELIZATION_API ClearVoxelGridBuffer_RenderThread(FRHICommandList& RHICmdList, class FVoxelGridResource* VoxelGridResource);


void VOXELIZATION_API CopyVoxelGridToCPU_RenderThread(FRHICommandList& RHICmdList,  FVoxelGridResource* VoxelGridResource, struct FVoxelGrid* VoxelGrid);

void VOXELIZATION_API DispatchCalculateVertexVelocity_RenderThread(
	FRHICommandList& RHICmdList, 
    FVertexVelocityResource* VertexVelocityResource,
    AStaticMeshActor* SMActor, 
	float DeltaTime);

TArray<ULandscapeComponent*> GetOverlappingLandscapeComponents(ALandscape* LandscapeActor, FBox VoxelGridBounds);


#pragma region CPU Voxelization

void VOXELIZATION_API VoxelizeMesh_Host(TArray<uint32>& VoxelGridBuffer, TArray<FVector4f>& VoxelGridNormalBuffer, TArray<FVector3f>& VoxelGridVelocityBuffer,
	const AStaticMeshActor* MeshActor, FVector3f Origin, FIntVector GridDim, float VoxelSize = 1.0f, FVector3f ConstantVelocity = FVector3f(0));

void VOXELIZATION_API VoxelizeActorSubMesh_Host(TArray<uint32>& VoxelGridBuffer, TArray<FVector4f>& VoxelGridNormalBuffer, TArray<FVector3f>& VoxelGridVelocityBuffer,
	const AActor* Actor, FVector3f Origin, FIntVector GridDim, float VoxelSize = 1.0f, FVector3f ConstantVelocity = FVector3f(0));
#pragma endregion CPU Voxelization

