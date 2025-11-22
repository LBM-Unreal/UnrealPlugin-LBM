// VoxelizationCore.h
#pragma once
#include "RHIResources.h"
#include "RenderResource.h"
#include "ShaderParameterStruct.h"

// FVoxelizationCS
BEGIN_SHADER_PARAMETER_STRUCT(FVoxelizationShaderParameters,)
	// Buffers
    SHADER_PARAMETER_UAV(RWStructuredBuffer<uint32>, VoxelGridBuffer)
    SHADER_PARAMETER_SRV(StructuredBuffer<float>, TriangleVerts)
    SHADER_PARAMETER_SRV(StructuredBuffer<uint32>, TriangleIndices)

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


/* ----------------- FCopyVoxelGridToSimCS ------------------------------------------------------------------------------------------------------*/ 
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


/* ----------------- FVertexVelocityCS ------------------------------------------------------------------------------------------------------*/ 
BEGIN_SHADER_PARAMETER_STRUCT(FVertexVelocityShaderParameters,)
	// Buffers
	SHADER_PARAMETER_SRV(StructuredBuffer<float>, TriangleVerts)
	SHADER_PARAMETER_UAV(RWStructuredBuffer<float>, VertexVelocities)

	// Params
	SHADER_PARAMETER(FMatrix44f, LocalToWorld)
	SHADER_PARAMETER(float, DeltaTime)
	SHADER_PARAMETER(uint32, VertexCount)
END_SHADER_PARAMETER_STRUCT()

class FVertexVelocityCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FVertexVelocityCS, Global)
	SHADER_USE_PARAMETER_STRUCT(FVertexVelocityCS, FGlobalShader)

	using FParameters = FVertexVelocityShaderParameters;
};


/* ----------------- Utility Functions ------------------------------------------------------------------------------------------------------*/ 
void VOXELIZATION_API DispatchCopyVoxelGridToSim_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelGridResource* VoxelGridResource,
    FUnorderedAccessViewRHIRef DstUAV,
    FIntVector3 SimDimension,
    uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);


void VOXELIZATION_API DispatchVoxelizeMesh_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelGridResource* VoxelGridResource,
    AStaticMeshActor* SMActor,
    FVector3f VoxelGridOrigin,
    FIntVector3 VoxelGridDimension);


void VOXELIZATION_API ClearVoxelGridBuffer_RenderThread(FRHICommandList& RHICmdList, class FVoxelGridResource* VoxelGridResource);


void VOXELIZATION_API CopyVoxelGridToCPU_RenderThread(FRHICommandList& RHICmdList,  FVoxelGridResource* VoxelGridResource, struct FVoxelGrid* VoxelGrid);

void VOXELIZATION_API DispatchCalculateVertexVelocity_RenderThread(
	FRHICommandList& RHICmdList,
	class FVertexVelocityShaderParameters* VelocityParameters);