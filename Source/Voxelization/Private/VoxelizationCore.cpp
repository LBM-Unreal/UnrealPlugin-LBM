#include "VoxelizationCore.h"
#include "ShaderParameterStruct.h"

#include "VoxelGrid.h"
#include "VoxelizationModule.h"
#include "Engine/StaticMeshActor.h"


#pragma region Shaders
/* ---------------------------------------- Shaders ---------------------------------------------------- */
// FVoxelizationCS
IMPLEMENT_SHADER_TYPE(, FVoxelizationCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("VoxelizeMesh"), SF_Compute);
// FCopyVoxelGridToSimCS
IMPLEMENT_SHADER_TYPE(, FCopyVoxelGridToSimCS, TEXT("/VoxelizationShaderDir/VoxelizationCoreCompute.usf"), TEXT("CpyVoxelGridToSimulation"), SF_Compute);
// FVertexVelocityCS
IMPLEMENT_SHADER_TYPE(, FVertexVelocityCS, TEXT("/VoxelizationShaderDir/VertexVelocityCompute.usf"), TEXT("CalculateVertexVelocity"), SF_Compute);
#pragma endregion Shaders

#pragma region Resource
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
#pragma endregion Resource

#pragma region HelperFunctions
/* ---------------------------------------- Helper Functions ---------------------------------------- */

// Algorithm adapted from "Fast 3D Triangle-Box Overlap Testing" by Tomas Akenine-Moller,
// but skipped the redundant AABB's x, y, z axis tests.
// We also replace the hard coded separating axis test with a more general form to simplify the implementation,
// will replace them if encounter performance issue.
bool TriangleAABBTest(FVector3f V0, FVector3f V1, FVector3f V2, FVector3f BoxCenter, FVector3f BoxHalfWidth)
{
    // Convert triangle to AABB's local space
    V0 -= BoxCenter;
    V1 -= BoxCenter;
    V2 -= BoxCenter;

    // aabb axis
    FVector3f E0{ 1, 0, 0 };
    FVector3f E1{ 0, 1, 0 };
    FVector3f E2{ 0, 0, 1 };

    // Triangle edges
    FVector3f F0 = V1 - V0;
    FVector3f F1 = V2 - V1;
    FVector3f F2 = V0 - V2;

    /* Fast plane-AABB overlap test with the triangle normal axis */
    // find the two most extreme points along the triangle normal
    FVector3f TriangleNormal = FVector3f::CrossProduct(F0, F1); // no need to normalize it for we are only using for comparisons
    FVector3f BoxMaxWithTriangleNormal
    {
        TriangleNormal.X > 0 ? BoxHalfWidth.X : -BoxHalfWidth.X,
        TriangleNormal.Y > 0 ? BoxHalfWidth.Y : -BoxHalfWidth.Y,
        TriangleNormal.Z > 0 ? BoxHalfWidth.Z : -BoxHalfWidth.Z
    };
    FVector3f BoxMinWithTriangleNormal = -BoxMaxWithTriangleNormal;

    float BoxMaxDistToTriangle = FVector3f::DotProduct(BoxMaxWithTriangleNormal - V0, TriangleNormal);
    float BoxMinDistToTriangle = FVector3f::DotProduct(BoxMinWithTriangleNormal - V0, TriangleNormal);

    if (BoxMinDistToTriangle > 0 || BoxMaxDistToTriangle < 0)
    {
        return false;
    }

    /* Test the 9 cross products of triangle edges and aabb axes (separating axes) */
    TArray<FVector3f> SeparatingAxes{
        FVector3f::CrossProduct(E0, F0),
        FVector3f::CrossProduct(E0, F1),
        FVector3f::CrossProduct(E0, F2),
        FVector3f::CrossProduct(E1, F0),
        FVector3f::CrossProduct(E1, F1),
        FVector3f::CrossProduct(E1, F2),
        FVector3f::CrossProduct(E2, F0),
        FVector3f::CrossProduct(E2, F1),
        FVector3f::CrossProduct(E2, F2),
    };

    for (const FVector3f& Axis : SeparatingAxes)
    {
        float P0 = FVector3f::DotProduct(V0, Axis);
        float P1 = FVector3f::DotProduct(V1, Axis);
        float P2 = FVector3f::DotProduct(V2, Axis);
        float R = FVector3f::DotProduct(BoxHalfWidth, Axis.GetAbs());
        if (FMath::Min3(P0, P1, P2) > R || FMath::Max3(P0, P1, P2) < -R)
        {
            return false;
        }
    }

    return true;
}
#pragma endregion HelperFunctions


/* ---------------------------------------- Exposed Functions ---------------------------------------------------- */
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
            TEXT("DispatchVoxelizeMesh_RenderThread: mismatched dimension:VoxelGridBuffer[%d, %d, %d], specified voxel grid dimension[%d, %d, %d]."),
            VoxelGridResource->GridDim.X, VoxelGridResource->GridDim.Y, VoxelGridResource->GridDim.Z,
            VoxelGridDimension.X, VoxelGridDimension.Y, VoxelGridDimension.Z)

    	VoxelGridResource->GridDim = VoxelGridDimension;
    }
    VoxelGridResource->GridOrigin = VoxelGridOrigin;

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
    uint32 BufferLength = VoxelGridResource->GetBufferLength();
    uint32 BufferSize = BufferLength * sizeof(uint32);

	if (VoxelGridResource->ImmovableMeshOccupancyBuffer.NumBytes != BufferSize)
	{
        UE_LOG(LogVoxelization, Warning, TEXT("ClearVoxelGridBuffer_RenderThread: Buffer size mismatch. Requested %d, allocated %d. Reallocating buffer..."),
            BufferSize,
            VoxelGridResource->ImmovableMeshOccupancyBuffer.NumBytes)
		VoxelGridResource->UpdateRHI(RHICmdList);
	}

    // TODO: optimize using compute shader
    void* OutputGPUBuffer = RHICmdList.LockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer, 0, BufferSize, RLM_WriteOnly);
    FMemory::Memset(OutputGPUBuffer, 0, BufferSize);
    // UnlockBuffer
    RHICmdList.UnlockBuffer(VoxelGridResource->ImmovableMeshOccupancyBuffer.Buffer);
}


void CopyVoxelGridToCPU_RenderThread(FRHICommandList& RHICmdList, FVoxelGridResource* VoxelGridResource, FVoxelGrid* VoxelGrid)
{
    uint32 BufferSize = VoxelGridResource->GetBufferLength();
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

TArray<ULandscapeComponent*> GetOverlappingLandscapeComponents(ALandscape* LandscapeActor, FBox VoxelGridBounds)
{
    TArray<ULandscapeComponent*> OverlappingLandscapeComponents{};

    // Validate landscape actor
    if (!LandscapeActor)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("GetOverlappingLandscapeComponents: No LandscapeActor assigned"));
        return OverlappingLandscapeComponents;
    }

    // Get all landscape components from the landscape actor
    TArray<ULandscapeComponent*> AllComponents;
    LandscapeActor->GetComponents<ULandscapeComponent>(AllComponents);

    UE_LOG(LogVoxelization, Display, TEXT("GetOverlappingLandscapeComponents: Scanning %d landscape components"), AllComponents.Num());

    // Check each component for overlap
    for (ULandscapeComponent* Component : AllComponents)
    {
        if (!Component || !Component->IsValidLowLevel())
        {
            continue;
        }

        // Get component bounding box
        FBox ComponentBounds = Component->Bounds.GetBox();

        // Check if bounding boxes overlap
        if (VoxelGridBounds.Intersect(ComponentBounds))
        {
            OverlappingLandscapeComponents.Add(Component);

            UE_LOG(LogVoxelization, Display, TEXT("GetOverlappingLandscapeComponents: Found overlapping component - SectionBase: (%d, %d), Bounds: Min(%f, %f, %f), Max(%f, %f, %f)"),
                Component->SectionBaseX, Component->SectionBaseY,
                ComponentBounds.Min.X, ComponentBounds.Min.Y, ComponentBounds.Min.Z,
                ComponentBounds.Max.X, ComponentBounds.Max.Y, ComponentBounds.Max.Z);
        }
    }

    UE_LOG(LogVoxelization, Display, TEXT("DebugFunc0: Found %d overlapping landscape components"), OverlappingLandscapeComponents.Num());

    return OverlappingLandscapeComponents;
}

void ClearImmovableMeshVoxelGridBuffer_Host(FVoxelGrid* VoxelGrid)
{
    if (!VoxelGrid)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("ClearImmovableMeshVoxelGridBuffer_Host: VoxelGrid is null"));
		return;    
    }

	for (uint32& VoxelValue : VoxelGrid->ImmovableMeshOccupancy)
    {
        VoxelValue = 0;
    }
}

void VoxelizeMesh_Host(TArray<uint32>& VoxelGridBuffer, TArray<FVector4f>& NormalBuffer,
    const AStaticMeshActor* MeshActor, FVector3f Origin, FIntVector GridDim, float VoxelSize)
{
    // Validation and setup
    if (!MeshActor || !MeshActor->GetStaticMeshComponent())
    {
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelizeMesh_Host: Invalid StaticMeshActor"));
        return;
    }

    UStaticMesh* Mesh = MeshActor->GetStaticMeshComponent()->GetStaticMesh();
    if (!Mesh || !Mesh->GetRenderData() || Mesh->GetRenderData()->LODResources.Num() == 0)
    {
        // Mesh->GetRenderData()->IsInitialized(); ?
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelizeMesh_Host: Invalid mesh"));
        return;
    }

    if (VoxelSize <= 0)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelizeMesh_Host: invalid voxel size, got %f"), VoxelSize);
        return;
    }

    if (GridDim.X <= 0 || GridDim.Y <= 0 || GridDim.Z <= 0)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelizeMesh_Host: invalid GridDim, got [%d, %d, %d]."), GridDim.X, GridDim.Y, GridDim.Z);
        return;
    }

    if (VoxelGridBuffer.Num() != GridDim.X * GridDim.Y * GridDim.Z)
    {
        UE_LOG(LogVoxelization, Warning, TEXT("VoxelizeMesh_Host: Buffer size mismatch got %d."), GridDim.X * GridDim.Y * GridDim.Z);
        VoxelGridBuffer.SetNum(GridDim.X * GridDim.Y * GridDim.Z);
    }

    // Extract raw positions
    const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
    const FPositionVertexBuffer& VB = LOD.VertexBuffers.PositionVertexBuffer;
    const FRawStaticIndexBuffer& IB = LOD.IndexBuffer;

    const FTransform LocalToWorld = MeshActor->GetActorTransform();

    int WrittenVoxels = 0;
    // Voxelize
    UE_LOG(LogVoxelization, Display, TEXT("Voxelizing mesh with %d indices... "), IB.GetNumIndices());
    for (int32 I = 0; I < IB.GetNumIndices(); I += 3)
    {
        FVector3f V0Local = VB.VertexPosition(IB.GetIndex(I + 0));
        FVector3f V1Local = VB.VertexPosition(IB.GetIndex(I + 1));
        FVector3f V2Local = VB.VertexPosition(IB.GetIndex(I + 2));

        FVector3f V0World = FVector3f(LocalToWorld.TransformPosition(FVector(V0Local)));
        FVector3f V1World = FVector3f(LocalToWorld.TransformPosition(FVector(V1Local)));
        FVector3f V2World = FVector3f(LocalToWorld.TransformPosition(FVector(V2Local)));

        // Transform to voxel space (0..GridSize)
        FVector3f V0VoxelSpace = (V0World - Origin) / VoxelSize;
        FVector3f V1VoxelSpace = (V1World - Origin) / VoxelSize;
        FVector3f V2VoxelSpace = (V2World - Origin) / VoxelSize;

        // Compute triangle bounds in voxel space for intersection efficiency
        FIntVector MaxGridBound{};
        FIntVector MinGridBound{};

        FVector3f MinV = FVector3f::Min3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);
        FVector3f MaxV = FVector3f::Max3(V0VoxelSpace, V1VoxelSpace, V2VoxelSpace);

        MinGridBound.X = FMath::Clamp(FMath::FloorToInt(MinV.X), 0, GridDim.X - 1);
        MinGridBound.Y = FMath::Clamp(FMath::FloorToInt(MinV.Y), 0, GridDim.Y - 1);
        MinGridBound.Z = FMath::Clamp(FMath::FloorToInt(MinV.Z), 0, GridDim.Z - 1);

        MaxGridBound.X = FMath::Clamp(FMath::FloorToInt(MaxV.X), 0, GridDim.X - 1);
        MaxGridBound.Y = FMath::Clamp(FMath::FloorToInt(MaxV.Y), 0, GridDim.Y - 1);
        MaxGridBound.Z = FMath::Clamp(FMath::FloorToInt(MaxV.Z), 0, GridDim.Z - 1);

        // Naively traverse the voxel bounds and test intersection
        for (int Z = MinGridBound.Z; Z <= MaxGridBound.Z; ++Z)
        {
            for (int Y = MinGridBound.Y; Y <= MaxGridBound.Y; ++Y)
            {
                for (int X = MinGridBound.X; X <= MaxGridBound.X; ++X)
                {
                    const FVector3f VoxelHalfSize = FVector3f(VoxelSize * 0.5f);
                    FVector3f VoxelCenter = Origin + VoxelSize * FVector3f(X, Y, Z) + VoxelHalfSize;

                    if (TriangleAABBTest(V0World, V1World, V2World, VoxelCenter, VoxelHalfSize))
                    {
                        VoxelGridBuffer[X + Y * GridDim.X + Z * GridDim.X * GridDim.Y] = 1;
                        WrittenVoxels++;
                    }
                }
            }
        }
    }
    UE_LOG(LogVoxelization, Display, TEXT("VoxelizeMesh_Host: Finished, Voxelizd %d triangles into %d voxels"), IB.GetNumIndices() / 3 - 1, WrittenVoxels);

}