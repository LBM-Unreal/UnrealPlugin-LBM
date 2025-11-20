#include "VoxelizationCore.h"
#include "ShaderParameterStruct.h"

#include "VoxelGrid.h"
#include "VoxelizationModule.h"

// FVoxelizationCS
IMPLEMENT_SHADER_TYPE(, FVoxelizationCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("VoxelizeMesh"), SF_Compute);

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

IMPLEMENT_SHADER_TYPE(, FCopyVoxelGridToSimCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("CpyVoxelGridToSimulation"), SF_Compute);


void DispatchCopyVoxelGridToSim_RenderThread(
    FRHICommandList& RHICmdList,
    FVoxelGridResource* VoxelGridResource,
    FUnorderedAccessViewRHIRef DstUAV,
    FIntVector3 SimDimension,
    uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ)
{
    if (VoxelGridResource->GridDim != SimDimension)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("DispatchCopyVoxelGridToSim_RenderThread: mismatched dimension: VoxelGrid[%d, %d, %d], SimulationBuffer[%d, %d, %d]"), 
            VoxelGridResource->GridDim.X, VoxelGridResource->GridDim.Y, VoxelGridResource->GridDim.Z, 
            SimDimension.X, SimDimension.Y, SimDimension.Z)
        //return;
    }

    TShaderMapRef<FCopyVoxelGridToSimCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    SetComputePipelineState(RHICmdList, Shader.GetComputeShader());

    {
        typename FCopyVoxelGridToSimCS::FParameters Parameters{};

        Parameters.CpySrcVoxelGridBuffer = VoxelGridResource->ImmovableMeshOccupancyBuffer.UAV;
        Parameters.CpyDstDebugBuffer = DstUAV;
        Parameters.SimDimension = SimDimension;
        Parameters.VoxelGridDimension = VoxelGridResource->GridDim;

        SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Parameters);
    }

    DispatchComputeShader(RHICmdList, Shader.GetShader(), ThreadGroupX, ThreadGroupY, ThreadGroupZ);
    UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
    UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}


void DispatchVoxelizeMesh_RenderThread(
    FRHICommandList& RHICmdList,
    class FVoxelizationShaderParameters* VoxelizationParameters)
{
    FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();

    if (VoxelGridResource->GridDim != VoxelizationParameters->GridDim)
    {
        UE_LOG(LogVoxelization, Error, TEXT("DispatchCopyVoxelGridToSim_RenderThread: mismatched dimension: VoxelGrid[%d, %d, %d], SimulationBuffer[%d, %d, %d]"),
            VoxelGridResource->GridDim.X, VoxelGridResource->GridDim.Y, VoxelGridResource->GridDim.Z,
            VoxelizationParameters->GridDim.X, VoxelizationParameters->GridDim.Y, VoxelizationParameters->GridDim.Z)
            return;
    }

    TShaderMapRef<FVoxelizationCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    SetComputePipelineState(RHICmdList, Shader.GetComputeShader());

	SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), *VoxelizationParameters);

    constexpr int32 ShaderBlockDimX = 4;
    int32 Blocks = FMath::CeilToInt32(VoxelizationParameters->TriangleCount / static_cast<float>(ShaderBlockDimX));
    DispatchComputeShader(RHICmdList, Shader.GetShader(), Blocks, 1, 1);

    UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
    UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}


void ClearVoxelGridBuffer_RenderThread()
{
    FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();

    auto BufferLength = VoxelGridResource->GetBufferLength();

    // TODO: optimize using compute shader
    FlushRenderingCommands();
    ENQUEUE_RENDER_COMMAND(CopyOccupancyBuffer)([VoxelGridResource, BufferLength](FRHICommandListImmediate& RHICmdList)
        {
            void* OutputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferLength * sizeof(uint32), RLM_WriteOnly));
            FMemory::Memset(OutputGPUBuffer, 0, BufferLength * sizeof(uint32));
            // UnlockBuffer
            RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
        });
    FlushRenderingCommands();
}


void CopyVoxelGridToCPU_RenderThread(TArray<uint32>& DstBuffer)
{
    FVoxelGridResource* VoxelGridResource = FVoxelGridResource::Get();

    uint32 BufferSize = VoxelGridResource->GetBufferLength();

    if (static_cast<uint32>(DstBuffer.Max()) < BufferSize)
    {
        UE_LOG(LogVoxelization, Warning, 
            TEXT("CopyVoxelGridToCPU_RenderThread: Not enough space, resizing destination buffer from %d to %d"),
            DstBuffer.Max(), VoxelGridResource->GetBufferLength());

        DstBuffer.SetNum(BufferSize);
    }

    auto* DstBufferPtr = DstBuffer.GetData();

    FlushRenderingCommands();
    ENQUEUE_RENDER_COMMAND(CopyOccupancyBuffer)([VoxelGridResource, DstBufferPtr, BufferSize](FRHICommandListImmediate& RHICmdList)
        {
            void* SrcBuffer = static_cast<float*>(RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferSize * sizeof(uint32), RLM_ReadOnly));
            FMemory::Memcpy(DstBufferPtr, SrcBuffer, BufferSize * sizeof(uint32));
            // UnlockBuffer
            RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
        });
    FlushRenderingCommands();
}