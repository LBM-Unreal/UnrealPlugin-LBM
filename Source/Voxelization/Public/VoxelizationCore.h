// VoxelizationCore.h
#pragma once
#include "RHIResources.h"
#include "RenderResource.h"
#include "ShaderParameterStruct.h"

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

struct FInstanceData_RenderThread
{
    void Init(FRHICommandList& RHICmdList);
};


void VOXELIZATION_API DispatchCopyVoxelGridToSim_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelGridResource* VoxelGridResource,
    FUnorderedAccessViewRHIRef DstUAV,
    FIntVector3 SimDimension,
    uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void VOXELIZATION_API DispatchVoxelizeMesh_RenderThread(
    FRHICommandList& RHICmdList, 
    class FVoxelizationShaderParameters* VoxelizationParameters);

void ClearVoxelGridBuffer_RenderThread();

void CopyVoxelGridToCPU_RenderThread(TArray<uint32>& DstBuffer);
