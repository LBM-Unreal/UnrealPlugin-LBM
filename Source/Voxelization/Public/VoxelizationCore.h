// VoxelizationCore.h
#pragma once
#include "RHIResources.h"
#include "RenderResource.h"
#include "ShaderParameterStruct.h"
#include "Landscape.h"
#include "VoxelGrid.h"

/* ---------------------------------------- Shaders ---------------------------------------------------- */
// FVoxelizationCS
BEGIN_SHADER_PARAMETER_STRUCT(FVoxelizationShaderParameters,)
	// Buffers
    SHADER_PARAMETER_UAV(RWStructuredBuffer<uint32>, VoxelGridBuffer)
    SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector3f>, GridVelocityBuffer)
    SHADER_PARAMETER_SRV(StructuredBuffer<float>, TriangleVerts)
    SHADER_PARAMETER_SRV(StructuredBuffer<uint32>, TriangleIndices)
    SHADER_PARAMETER_SRV(StructuredBuffer<FVector3f>, VertexVelocitiesWorldSpace)

    // Params
    SHADER_PARAMETER(FVector3f, GridMin)
    SHADER_PARAMETER(FIntVector, GridDim)
    SHADER_PARAMETER(FMatrix44f, LocalToWorld)
    SHADER_PARAMETER(uint32, TriangleCount)
    SHADER_PARAMETER(uint32, VertexCount)
    SHADER_PARAMETER(uint32, IndexCount)
END_SHADER_PARAMETER_STRUCT()

class FVoxelizationCS : public FGlobalShader
{
    DECLARE_SHADER_TYPE(FVoxelizationCS, Global)
    SHADER_USE_PARAMETER_STRUCT(FVoxelizationCS, FGlobalShader)

	using FParameters = FVoxelizationShaderParameters;
};


// FCopyVoxelGridToSimCS
BEGIN_SHADER_PARAMETER_STRUCT(FCopyVoxelGridToSimParameters, )
    // Buffers
    SHADER_PARAMETER_UAV(RWStructuredBuffer<uint32>, CpySrcVoxelGridBuffer)
    SHADER_PARAMETER_UAV(RWStructuredBuffer<int>, CpyDstDebugBuffer)

    // Params
    SHADER_PARAMETER(FIntVector3, SimDimension)
	SHADER_PARAMETER(FIntVector3, VoxelGridDimension)
END_SHADER_PARAMETER_STRUCT()

class FCopyVoxelGridToSimCS: public FGlobalShader
{
    DECLARE_SHADER_TYPE(FCopyVoxelGridToSimCS, Global)
    SHADER_USE_PARAMETER_STRUCT(FCopyVoxelGridToSimCS, FGlobalShader)

	using FParameters = FCopyVoxelGridToSimParameters;
};


// FVertexVelocityCS
BEGIN_SHADER_PARAMETER_STRUCT(FVertexVelocityShaderParameters,)
	// Buffers
	SHADER_PARAMETER_SRV(StructuredBuffer<float>, TriangleVerts)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector3f>, VertexPositionsWorldSpace)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector3f>, VertexVelocitiesWorldSpace)

	// Params
	SHADER_PARAMETER(FMatrix44f, LocalToWorld)
	SHADER_PARAMETER(uint32, VertexCount)
	SHADER_PARAMETER(float, DeltaTime)
END_SHADER_PARAMETER_STRUCT()

class FVertexVelocityCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexVelocityCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FVertexVelocityCS, FGlobalShader)

	using FParameters = FVertexVelocityShaderParameters;

	static constexpr uint32 BlockSize = 64;
};


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
void VOXELIZATION_API DispatchCopyVoxelGridToSim_RenderThread(
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
void VOXELIZATION_API ClearImmovableMeshVoxelGridBuffer_Host(FVoxelGrid* VoxelGrid);

void VOXELIZATION_API VoxelizeMesh_Host(TArray<uint32>& VoxelGridBuffer, TArray<FVector4f>& NormalBuffer, const AStaticMeshActor* MeshActor, FVector3f Origin, FIntVector GridDim, float VoxelSize = 1.0f);
#pragma endregion CPU Voxelization

