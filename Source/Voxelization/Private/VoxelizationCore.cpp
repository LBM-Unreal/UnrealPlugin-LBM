#include "VoxelizationCore.h"
#include "ShaderParameterStruct.h"

#include "VoxelGrid.h"
#include "VoxelizationModule.h"
#include "Engine/StaticMeshActor.h"



/* ---------------------------------------- Shaders ---------------------------------------------------- */
// FVoxelizationCS
IMPLEMENT_SHADER_TYPE(, FVoxelizationCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("VoxelizeMesh"), SF_Compute);
// FCopyVoxelGridToSimCS
IMPLEMENT_SHADER_TYPE(, FCopyVoxelGridToSimCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("CpyVoxelGridToSimulation"), SF_Compute);
// FVertexVelocityCS
IMPLEMENT_SHADER_TYPE(, FVertexVelocityCS, TEXT("/VoxelizationShaderDir/VertexVelocityCompute.usf"), TEXT("CalculateVertexVelocity"), SF_Compute);


/* ---------------------------------------- Resources ---------------------------------------------------- */
FVertexVelocityResource* FVertexVelocityResource::GInstance = nullptr;

void FVertexVelocityResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	if (VertexCount > 0)
	{
		VertexPositionsBuffer.Initialize(RHICmdList, TEXT("VertexPositionsBuffer"), sizeof(FVector3f), VertexCount);
		VertexVelocitiesBuffer.Initialize(RHICmdList, TEXT("VertexVelocitiesBuffer"), sizeof(FVector3f), VertexCount);
	}
    else
    {
        UE_LOG(LogVoxelization, Warning, TEXT("FVertexVelocityResource::InitRHI: Invalid VertexCount: %d. Stopped execution."), VertexCount);
        return;
    }
}

void FVertexVelocityResource::SetVertexCount(FRHICommandListBase& RHICmdList, uint32 InVertexCount)
{
    if (VertexCount != InVertexCount)
    {
        // Release old buffers if they exist
        if (VertexCount > 0)
        {
            VertexPositionsBuffer.Release();
            VertexVelocitiesBuffer.Release();
        }

        VertexCount = InVertexCount;

        // Initialize new buffers if count is valid
        if (InVertexCount > 0)
        {
            VertexPositionsBuffer.Initialize(RHICmdList, TEXT("VertexPositionsBuffer"), sizeof(FVector3f), VertexCount);
            VertexVelocitiesBuffer.Initialize(RHICmdList, TEXT("VertexVelocitiesBuffer"), sizeof(FVector3f), VertexCount);
        }
    }
}

void FVertexVelocityResource::ReleaseRHI()
{
	VertexPositionsBuffer.Release();
	VertexVelocitiesBuffer.Release();
}

FVertexVelocityResource* FVertexVelocityResource::Get()
{
	if (!GInstance)
	{
		GInstance = new FVertexVelocityResource();
	}
	return GInstance;
}


/* ---------------------------------------- Functions ---------------------------------------------------- */
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


void VOXELIZATION_API DispatchVoxelizeMesh_RenderThread(
    FRHICommandList& RHICmdList,
    FVoxelGridResource* VoxelGridResource,
    FVertexVelocityResource* VertexVelocityResource,
    AStaticMeshActor* SMActor,
    FVector3f VoxelGridOrigin,
    FIntVector3 VoxelGridDimension)
{
    if (VoxelGridDimension != VoxelGridResource->GridDim)
    {
        UE_LOG(LogVoxelization, Warning,
            TEXT("DispatchVoxelizeMesh_RenderThread: mismatched dimension:VoxelGridBuffer[%d, %d, %d], specified voxel grid dimension[%d, %d, %d]. Stopped execution."),
            VoxelGridResource->GridDim.X, VoxelGridResource->GridDim.Y, VoxelGridResource->GridDim.Z,
            VoxelGridDimension.X, VoxelGridDimension.Y, VoxelGridDimension.Z)
            return;
    }

    TShaderMapRef<FVoxelizationCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    SetComputePipelineState(RHICmdList, Shader.GetComputeShader());

    UE_LOG(LogVoxelization, Display, TEXT("DispatchVoxelizeMesh_RenderThread: Fetching mesh data"));

    // Mesh data
    const auto& MeshLOD0 = SMActor->GetStaticMeshComponent()->GetStaticMesh()->GetRenderData()->LODResources[0];
    const FRawStaticIndexBuffer& IB = MeshLOD0.IndexBuffer;
    const FPositionVertexBuffer& VB = MeshLOD0.VertexBuffers.PositionVertexBuffer;
    const int bHasReversedIndices = MeshLOD0.bHasReversedIndices;

    UE_LOG(LogVoxelization, Display, TEXT("DispatchVoxelizeMesh_RenderThread: Fetching mesh data: [ReversedIndices = %d]"), bHasReversedIndices);

	{
        // Create SRV for mesh index buffer
        FShaderResourceViewRHIRef MeshIndexBufferSRV;
        FBufferRHIRef IndexBufferRHIRef = IB.IndexBufferRHI;
        const bool bAllowCPUAccess = IB.GetAllowCPUAccess();
        const bool bCanCreateIndexSRV = IndexBufferRHIRef.IsValid() &&
            ((IndexBufferRHIRef->GetUsage() & EBufferUsageFlags::ShaderResource) == EBufferUsageFlags::ShaderResource) &&
            bAllowCPUAccess;
        if (bCanCreateIndexSRV)
        {
            bool b32Bit = IB.Is32Bit();
            MeshIndexBufferSRV = RHICmdList.CreateShaderResourceView(
                IndexBufferRHIRef,
                FRHIViewDesc::CreateBufferSRV()
                .SetType(FRHIViewDesc::EBufferType::Typed)
                .SetFormat(b32Bit ? PF_R32_UINT : PF_R16_UINT));
            UE_LOG(LogVoxelization, Display, TEXT("DispatchVoxelizeMesh_RenderThread: Created SRV from mesh IB"));
        }
        else
        {
            MeshIndexBufferSRV = nullptr;
            UE_LOG(LogVoxelization, Warning, 
                TEXT("DispatchVoxelizeMesh_RenderThread: Cannot create SRV for mesh IB, IndexBufferRHIRefValid(%d), EBufferUsageIsShared(%d), bAllowCPUAccess(%d). Stopped execution."),
                IndexBufferRHIRef.IsValid(), (IndexBufferRHIRef->GetUsage() & EBufferUsageFlags::ShaderResource), bAllowCPUAccess);
            return;
        }

		FVoxelizationShaderParameters Params{};

    	Params.TriangleIndices = MeshIndexBufferSRV;
    	Params.TriangleVerts = VB.GetSRV();
    	Params.VoxelGridBuffer = VoxelGridResource->ImmovableMeshOccupancyBuffer.UAV;
    	Params.GridVelocityBuffer = VoxelGridResource->GridVelocityBuffer.UAV;
        Params.VertexVelocitiesWorldSpace = VertexVelocityResource->VertexVelocitiesBuffer.SRV;

    	// Parameters
    	Params.GridMin = VoxelGridOrigin;
    	Params.GridDim = VoxelGridDimension;
    	Params.LocalToWorld = static_cast<FMatrix44f>(SMActor->GetTransform().ToMatrixWithScale().GetTransposed());
    	Params.TriangleCount = IB.GetNumIndices() / 3;
    	Params.VertexCount = VB.GetNumVertices();
    	Params.IndexCount = IB.GetNumIndices();

    	SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Params);
	}

    constexpr int32 ShaderBlockDimX = 4;
    int32 Blocks = FMath::CeilToInt32(IB.GetNumIndices() / static_cast<float>(ShaderBlockDimX));
    DispatchComputeShader(RHICmdList, Shader.GetShader(), Blocks, 1, 1);

    UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
    UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}


void ClearVoxelGridBuffer_RenderThread(FRHICommandList& RHICmdList, FVoxelGridResource* VoxelGridResource)
{
    auto BufferLength = VoxelGridResource->GetBufferLength();

    // TODO: optimize using compute shader
    void* OutputGPUBuffer = RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferLength * sizeof(uint32), RLM_WriteOnly);
    FMemory::Memset(OutputGPUBuffer, 0, BufferLength * sizeof(uint32));
    // UnlockBuffer
    RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
}


void CopyVoxelGridToCPU_RenderThread(FRHICommandList& RHICmdList, FVoxelGridResource* VoxelGridResource, FVoxelGrid* VoxelGrid)
{
    uint32 BufferSize = VoxelGridResource->GetBufferLength();
    VoxelGrid->Origin = VoxelGridResource->GridOrigin;
    if (VoxelGrid->GridDim != VoxelGridResource->GridDim || static_cast<uint32>(VoxelGrid->ImmovableMeshOccupancy.Max()) < BufferSize)
    {
        VoxelGrid->GridDim = VoxelGridResource->GridDim;
        VoxelGrid->ImmovableMeshOccupancy.SetNumZeroed(BufferSize);

        UE_LOG(LogVoxelization, Warning, TEXT("CopyVoxelGridToCPU_RenderThread: mismatched voxel grid dim, resizing to %d, %d, %d"),
            VoxelGridResource->GridDim.X, VoxelGridResource->GridDim.Y, VoxelGridResource->GridDim.Z);
    }

    auto* DstBufferPtr = VoxelGrid->ImmovableMeshOccupancy.GetData();
    void* SrcBuffer = RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferSize * sizeof(uint32), RLM_ReadOnly);
    FMemory::Memcpy(DstBufferPtr, SrcBuffer, BufferSize * sizeof(uint32));
    RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
}


void DispatchCalculateVertexVelocity_RenderThread(FRHICommandList& RHICmdList, FVertexVelocityResource* VertexVelocityResource, AStaticMeshActor* SMActor, float DeltaTime)
{
	TShaderMapRef<FVertexVelocityCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	SetComputePipelineState(RHICmdList, Shader.GetComputeShader());

    const auto& MeshLOD0 = SMActor->GetStaticMeshComponent()->GetStaticMesh()->GetRenderData()->LODResources[0];
    const uint32 VertexCount = MeshLOD0.GetNumVertices();

    VertexVelocityResource->SetVertexCount(RHICmdList, VertexCount);
	{
		FVertexVelocityShaderParameters Params{};
		Params.TriangleVerts = MeshLOD0.VertexBuffers.PositionVertexBuffer.GetSRV();
		Params.VertexPositionsWorldSpace = VertexVelocityResource->VertexPositionsBuffer.UAV;
		Params.VertexVelocitiesWorldSpace = VertexVelocityResource->VertexVelocitiesBuffer.UAV;

		Params.LocalToWorld = static_cast<FMatrix44f>(SMActor->GetTransform().ToMatrixWithScale().GetTransposed());
		Params.DeltaTime = DeltaTime;
        Params.VertexCount = VertexCount;
		SetShaderParameters(RHICmdList, Shader, Shader.GetComputeShader(), Params);
	}

	int32 Blocks = FMath::CeilToInt32(VertexCount / static_cast<float>(FVertexVelocityCS::BlockSize));
	DispatchComputeShader(RHICmdList, Shader.GetShader(), Blocks, 1, 1);

	UnsetShaderSRVs(RHICmdList, Shader, Shader.GetComputeShader());
	UnsetShaderUAVs(RHICmdList, Shader, Shader.GetComputeShader());
}