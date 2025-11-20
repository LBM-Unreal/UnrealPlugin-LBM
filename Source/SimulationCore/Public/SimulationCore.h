// SimulationCore.h

class SIMULATIONCORE_API FSimulationShaderResource : public FRenderResource
{
	static FSimulationShaderResource* GInstance;
public:
	FTextureRHIRef DebugTexture;
	FUnorderedAccessViewRHIRef DebugTextureUAV;
	FRWBufferStructured DebugBuffer;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	virtual void ReleaseRHI() override; 

	static FSimulationShaderResource* Get();
private:
	FSimulationShaderResource()
	{
		ENQUEUE_RENDER_COMMAND(FCreateSimShaderRes)([this](FRHICommandListImmediate& RHICmdList)
			{
				this->InitResource(RHICmdList);
			});
	}
};

class SIMULATIONCORE_API FSimulationShaderResource3D : public FRenderResource
{
	static FSimulationShaderResource3D* GInstance;
public:
	FTextureRHIRef DebugTexture;
	FTextureRHIRef DebugTexture3D;
	FUnorderedAccessViewRHIRef DebugTexture3DUAV;
	FRWBufferStructured DebugBuffer;
	FUnorderedAccessViewRHIRef DebugTextureUAV;
	FIntVector3 TextureSize;

	TArray<FIntVector3> c;
	TArray<float> w;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	virtual void ReleaseRHI() override;

	static FSimulationShaderResource3D* Get();

	struct SimulationParams {
		float InitialVelocity;
		float InitialDensity;
		float RelaxationFactor;
		int DebugTextureSlice;
		FIntVector3 SimDimensions;
	} Params;

private:
	FSimulationShaderResource3D() : TextureSize({ 256, 256, 256 }), Params({ 0.0005, 0.0003, 0.8, 16, FIntVector3(256, 256, 256) })
	{
		c = {
			FIntVector3(0, 0, 0  ),
			FIntVector3(-1, -1, 0),
			FIntVector3(0, -1, 0 ),
			FIntVector3(1, -1, 0 ),
			FIntVector3(-1, 0, 0 ),
			FIntVector3(0, -1, 1 ),
			FIntVector3(-1, 0, 1 ),
			FIntVector3(0, 0, 1  ),
			FIntVector3(1, 0, 1  ),
			FIntVector3(0, 1, 1  ),
			FIntVector3(0, -1, -1),
			FIntVector3(-1, 0, -1),
			FIntVector3(0, 0, -1 ),
			FIntVector3(1, 0, -1 ),
			FIntVector3(0, 1, -1 ),
			FIntVector3(1, 0, 0  ),
			FIntVector3(-1, 1, 0 ),
			FIntVector3(0, 1, 0  ),
			FIntVector3(1, 1, 0  ),
		};
		w = {
			1. / 3. , // [0]
			1. / 36., // [1 - 4]
			2. / 36.,
			1. / 36.,
			2. / 36.,
			1. / 36., // [5 - 9]
			1. / 36.,
			2. / 36.,
			1. / 36.,
			1. / 36.,
			1. / 36., // [10 - 14]
			1. / 36.,
			2. / 36.,
			1. / 36.,
			1. / 36.,
			2. / 36., // [15 - 18]
			1. / 36.,
			2. / 36.,
			1. / 36.
		};

		ENQUEUE_RENDER_COMMAND(FCreateSimShaderRes)([this](FRHICommandListImmediate& RHICmdList)
			{
				this->InitResource(RHICmdList);
			});
	}
};

void SIMULATIONCORE_API DispatchExampleComputeShader_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, float Scale, float Translate, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);
void SIMULATIONCORE_API DispatchExampleComputeShader_GameThread(float InputVal, float Scale, float Translate, FSimulationShaderResource* Resource);
float SIMULATIONCORE_API GetGPUReadback(FSimulationShaderResource* Resource, float& OutputVal);


void SIMULATIONCORE_API DispatchLBMInitalState_RenderThread(
	FRHICommandList& RHICmdList, 
	FSimulationShaderResource* Resource, 
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMStreaming_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMCollision_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMMRInitialState_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMMRStreamingCollision_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMBoundaryTreatment_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMInitialInterface_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMAdvectMass_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMSurface1_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMSurface2_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMSurface3_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMReverseDF_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMInitalState3D_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource3D* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMStreaming3D_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource3D* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMCollision3D_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource3D* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);

void SIMULATIONCORE_API DispatchLBMBoundaryTreatment3D_RenderThread(
	FRHICommandList& RHICmdList,
	FSimulationShaderResource3D* Resource,
	uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);