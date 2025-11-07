// SimulationCore.h

class SIMULATIONCORE_API FSimulationShaderResource : public FRenderResource
{
	static FSimulationShaderResource* GInstance;
public:
	FRWBufferStructured InputBuffer;  // RWStructuredBuffer<float> InputBuffer;
	FRWBufferStructured OutputBuffer; // RWStructuredBuffer<float> OutputBuffer;
	FTextureRHIRef DebugTexture;
	FUnorderedAccessViewRHIRef DebugTextureUAV;
	FTexture2DArrayRHIRef SimulationDataArray;
	FUnorderedAccessViewRHIRef SimulationDataArrayUAV;
	FTexture2DArrayRHIRef SimulationDataArray2;
	FUnorderedAccessViewRHIRef SimulationDataArray2UAV;
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
	FUnorderedAccessViewRHIRef DebugTextureUAV;
	FTexture2DArrayRHIRef SimulationDataArray;
	FUnorderedAccessViewRHIRef SimulationDataArrayUAV;
	FTexture2DArrayRHIRef SimulationDataArray2;
	FUnorderedAccessViewRHIRef SimulationDataArray2UAV;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	virtual void ReleaseRHI() override;

	static FSimulationShaderResource3D* Get();

	struct SimulationParams {
		float InitialVelocity;
		float InitialDensity;
		float RelaxationFactor;
	} Params;

private:
	FSimulationShaderResource3D() : Params({0.0005, 0.0003, 0.8})
	{
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