// SimulationBPModule.h
#pragma once


#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimulationCore.h"
#include "IRenderDocPlugin.h"
#include "Engine/Texture2DArray.h"
#include "VoxelizationCore.h"

#include "SimulationBPModule.generated.h"

class FSimulationBPModule : public IModuleInterface {
    virtual void StartupModule() override;
    // virtual void ShutdownModule() override;
};

UCLASS(meta=(ScriptName="SimulationLibrary"), MinimalAPI)
class USimulationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta=(DisplayName="Execute Compute Shader"), Category="LBM Sim")
	static SIMULATIONBP_API float ExecuteExampleComputeShader(float InputVal, float Scale, float Translate) {
		float OutputVal;
		DispatchExampleComputeShader_GameThread(InputVal, Scale, Translate, FSimulationShaderResource::Get());
		return GetGPUReadback(FSimulationShaderResource::Get(), OutputVal);
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Debug Texture Value"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetDebugTextureValue(UTexture* OutTexture)
	{
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
			{
				FRHITexture* SrcTex = FSimulationShaderResource::Get()->DebugTexture;
				FRHITexture* DstTex = OutTexture->GetResource()->GetTexture2DRHI();
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.Size = { 256,256,1 };
				RHICmdList.CopyTexture(SrcTex, DstTex, CopyInfo);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Start Renderdoc"), Category = "LBM Sim")
	static SIMULATIONBP_API void StartRenderdoc() {
		ENQUEUE_RENDER_COMMAND(FStartRenderdoc)([](FRHICommandListImmediate& RHICmdList)
			{
				UE_LOG(LogTemp, Warning, TEXT("RDC saved in %s"), *FString(FPaths::Combine(FPaths::ProjectDir(), TEXT("Captures"))));
				IRenderDocPlugin::Get().BeginCapture(&RHICmdList, 0, FPaths::Combine(FPaths::ProjectDir(), TEXT("Captures")));
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "End Renderdoc"), Category = "LBM Sim")
	static SIMULATIONBP_API void EndRenderdoc() {
		ENQUEUE_RENDER_COMMAND(FEndRenderdoc)([](FRHICommandListImmediate& RHICmdList)
			{
				IRenderDocPlugin::Get().EndCapture(&RHICmdList);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Resource"), Category = "LBM Sim")
	static SIMULATIONBP_API void UpdateResource() {
		ENQUEUE_RENDER_COMMAND(FUpdateResource)([](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource::Get()->UpdateRHI(RHICmdList);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM InitialState"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMInitialState() {
		ENQUEUE_RENDER_COMMAND(FLBMInitialState)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMInitalState_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Streaming"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMStreaming() {
		ENQUEUE_RENDER_COMMAND(FLBMStreaming)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMStreaming_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Collision"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMCollision() {
		ENQUEUE_RENDER_COMMAND(FLBMCollision)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMCollision_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Advect Mass"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMAdvectMass() {
		ENQUEUE_RENDER_COMMAND(FLBMAdvectMass)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMAdvectMass_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM MR InitialState"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMMRInitialState() {
		ENQUEUE_RENDER_COMMAND(FLBMMRInitialState)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMMRInitialState_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM MR Streaming Collision"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMMRStreamingCollision() {
		ENQUEUE_RENDER_COMMAND(FLBMMRStreamingCollision)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMMRStreamingCollision_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 63, 63, 1); // margin the last row/col of cells
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Boundary Treatment"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMBoundaryTreatment() {
		ENQUEUE_RENDER_COMMAND(FLBMBoundaryTreatment)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMBoundaryTreatment_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Initial Interface"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMInitialInterface() {
		ENQUEUE_RENDER_COMMAND(FLBMInitialInterface)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMInitialInterface_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Surface 1"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMSurface1() {
		ENQUEUE_RENDER_COMMAND(FLBMSurface1)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMSurface1_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
				RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Surface 2"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMSurface2() {
		ENQUEUE_RENDER_COMMAND(FLBMSurface2)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMSurface2_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Surface 3"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMSurface3() {
		ENQUEUE_RENDER_COMMAND(FLBMSurface3)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMSurface3_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Reverse DF"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMReverseDF() {
		ENQUEUE_RENDER_COMMAND(FLBMReverseDF)([](FRHICommandListImmediate& RHICmdList)
			{
				DispatchLBMReverseDF_RenderThread(RHICmdList, FSimulationShaderResource::Get(), 16, 16, 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Debug Texture Value 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetDebugTextureValue3D(UTexture* OutTexture)
	{
		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.Size = { FMath::Min(256, FSimulationShaderResource3D::Get()->TextureSize[0]),FMath::Min(256, FSimulationShaderResource3D::Get()->TextureSize[1]),1 };
				RHICmdList.CopyTexture(FSimulationShaderResource3D::Get()->DebugTexture, OutTexture->GetResource()->GetTexture2DRHI(), CopyInfo);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Debug Texture 3D Value 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void GetDebugTexture3DValue3D(UTexture* OutTexture)
	{
		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FGetTexVal)([OutTexture](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.Size = { 
					FMath::Min(256, FSimulationShaderResource3D::Get()->TextureSize[0]),
					FMath::Min(256, FSimulationShaderResource3D::Get()->TextureSize[1]),
					FMath::Min(256, FSimulationShaderResource3D::Get()->TextureSize[2])
				};
				auto srcRHI = FSimulationShaderResource3D::Get()->DebugTexture3D;
				auto dstRHI = OutTexture->GetResource()->GetTexture3DRHI();
				RHICmdList.CopyTexture(srcRHI, dstRHI, CopyInfo);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Update Resource 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void UpdateResource3D(FVector3f TextureSize) {
		ENQUEUE_RENDER_COMMAND(FUpdateResource)([TextureSize](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->TextureSize = FIntVector3(TextureSize);
				FSimulationShaderResource3D::Get()->Params.SimDimensions = FIntVector3(TextureSize);
				FSimulationShaderResource3D::Get()->UpdateRHI(RHICmdList);
			});
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM InitialState 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMInitialState3D(float InitialVelocity, float InitialDensity) {
		ENQUEUE_RENDER_COMMAND(FLBMInitialState)([InitialVelocity, InitialDensity](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.InitialVelocity = InitialVelocity;
				FSimulationShaderResource3D::Get()->Params.InitialDensity = InitialDensity;
				DispatchLBMInitalState3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4, 
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4
				);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Streaming 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMStreaming3D(int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMStreaming)([debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				DispatchLBMStreaming3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM Collision 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMCollision3D(float RelaxationFactor, int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMCollision)([RelaxationFactor, debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				FSimulationShaderResource3D::Get()->Params.RelaxationFactor = RelaxationFactor;
				DispatchLBMCollision3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM MR InitialState 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMMRInitialState3D(int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMMRInitialState)([debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				DispatchLBMMRInitialState3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(), 
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4 + 1, 
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4 + 1,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4 + 1);
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM MR Streaming Collision 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMMRStreamingCollision3D(int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMMRStreamingCollision)([debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				DispatchLBMMRStreamingCollision3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(),
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4 - 1,
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4 - 1,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4 - 1); // margin the last row/col of cells
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM HOME Streaming Collision 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMHOMEStreamingCollision3D(int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMMRStreamingCollision)([debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				DispatchLBMHOMEStreamingCollision3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(),
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4 - 1,
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4 - 1,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4 - 1); // margin the last row/col of cells
			});
		FlushRenderingCommands();
	}

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LBM BoundaryTreatment 3D"), Category = "LBM Sim")
	static SIMULATIONBP_API void LBMBoundaryTreatment3D(int debugTextureSlice) {
		ENQUEUE_RENDER_COMMAND(FLBMBoundaryTreatment3D)([debugTextureSlice](FRHICommandListImmediate& RHICmdList)
			{
				FSimulationShaderResource3D::Get()->Params.DebugTextureSlice = debugTextureSlice;
				DispatchLBMBoundaryTreatment3D_RenderThread(RHICmdList, FSimulationShaderResource3D::Get(),
					FSimulationShaderResource3D::Get()->TextureSize[0] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[1] / 4,
					FSimulationShaderResource3D::Get()->TextureSize[2] / 4);
			});
		FlushRenderingCommands();
	}


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Voxelize Actor"), Category = "LBM Sim")
	static SIMULATIONBP_API void VoxelizeActor(AStaticMeshActor* SMActor, FVector3f Origin, FVector3f GridDim, float VoxelSize)
	{
		TArray<uint32> VoxelGridBuffer{};
		TArray<FVector4f> VoxelNormalBuffer{};

		VoxelizeMesh_Host(VoxelGridBuffer, VoxelNormalBuffer, SMActor, Origin, FIntVector(GridDim), VoxelSize);

		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FUpdateVoxelDataCPU)([VoxelGrid=MoveTemp(VoxelGridBuffer), SMActor, GridDim, Origin, VoxelSize](FRHICommandListImmediate& RHICmdList)
			{
				auto* VoxelizeResource = FVoxelGridResource::Get();
				VoxelizeResource->GridDim = FIntVector(GridDim);
				VoxelizeResource->GridOrigin = Origin;
				auto SimDim = FSimulationShaderResource3D::Get()->Params.SimDimensions;

				VoxelizeResource->UpdateRHI(RHICmdList);

				// Copy to GPU
				void* OutputGPUBuffer = static_cast<float*>(RHICmdList.LockBuffer(VoxelizeResource->ImmovableMeshOccupancyBuffer.Buffer, 0,
					VoxelGrid.Num() * sizeof(uint32), RLM_WriteOnly));
				FMemory::Memcpy(OutputGPUBuffer, VoxelGrid.GetData(), VoxelGrid.Num() * sizeof(uint32));
				RHICmdList.UnlockBuffer(VoxelizeResource->ImmovableMeshOccupancyBuffer.Buffer);
			});
		FlushRenderingCommands();

		// Copy to Sim
		ENQUEUE_RENDER_COMMAND(FUpdateVoxelDataCPU)([SMActor, GridDim, Origin, VoxelSize](FRHICommandListImmediate& RHICmdList)
			{
				auto SimDim = FSimulationShaderResource3D::Get()->Params.SimDimensions;
				constexpr int BlockSize = 4; // Shader block size

				DispatchCopyVoxelGridToSim_RenderThread(RHICmdList,
					FVoxelGridResource::Get(),
					FSimulationShaderResource3D::Get()->DebugBuffer.UAV, SimDim,
					(SimDim.X + BlockSize - 1) / BlockSize,
					(SimDim.Y + BlockSize - 1) / BlockSize,
					(SimDim.Z + BlockSize - 1) / BlockSize);
            });
		FlushRenderingCommands();
	}


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Voxelize Actor GPU"), Category = "LBM Sim")
	static SIMULATIONBP_API void VoxelizeActorGPU(AStaticMeshActor* SMActor, FVector3f GridDim, FVector3f Origin)
	{

		FlushRenderingCommands();
		ENQUEUE_RENDER_COMMAND(FUpdateVoxelData)([SMActor, GridDim, Origin](FRHICommandListImmediate& RHICmdList)
			{
				auto* VoxelGridResource = FVoxelGridResource::Get();
                auto* VertexVelocityResource = FVertexVelocityResource::Get();
				auto* SimResource = FSimulationShaderResource3D::Get();
				auto SimDim = SimResource->Params.SimDimensions;
				constexpr int BlockSize = 4;

				VoxelGridResource->GridOrigin = Origin;
				if (VoxelGridResource->GridDim != FIntVector3(GridDim))
				{
					VoxelGridResource->GridDim = FIntVector3(GridDim);
					VoxelGridResource->UpdateRHI(RHICmdList);
				}

				// Clear Buffer
				ClearVoxelGridBuffer_RenderThread(RHICmdList, VoxelGridResource);

				// Voxelize
				DispatchVoxelizeMesh_RenderThread(RHICmdList, VoxelGridResource, VertexVelocityResource, SMActor, Origin, FIntVector(GridDim));
			});

		FlushRenderingCommands();

		ENQUEUE_RENDER_COMMAND(FUpdateVoxelData)([SMActor, GridDim, Origin](FRHICommandListImmediate& RHICmdList)
			{
				auto* VoxelGridResource = FVoxelGridResource::Get();
				auto* VertexVelocityResource = FVertexVelocityResource::Get();
				auto* SimResource = FSimulationShaderResource3D::Get();
				auto SimDim = SimResource->Params.SimDimensions;
				constexpr int BlockSize = 4;
				// Copy to Simulation Buffer
				DispatchCopyVoxelGridToSim_RenderThread(RHICmdList,
					VoxelGridResource,
					SimResource->DebugBuffer.UAV, SimDim,
					(SimDim.X + BlockSize - 1) / BlockSize,
					(SimDim.Y + BlockSize - 1) / BlockSize,
					(SimDim.Z + BlockSize - 1) / BlockSize);
			});
		FlushRenderingCommands();
	}
};