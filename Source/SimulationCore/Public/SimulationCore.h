// SimulationCore.h

class SIMULATIONCORE_API FSimulationShaderResource : public FRenderResource
{
	static FSimulationShaderResource* GInstance;
public:
	FRWBufferStructured InputBuffer;  // RWStructuredBuffer<float> InputBuffer;
	FRWBufferStructured OutputBuffer; // RWStructuredBuffer<float> OutputBuffer;
	FTextureRHIRef OutputTexture;
	FUnorderedAccessViewRHIRef OutputTextureUAV;
	FTexture2DArrayRHIRef OutputTextureArray;
	FUnorderedAccessViewRHIRef OutputTextureArrayUAV;

	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	virtual void ReleaseRHI() override; 

	static FSimulationShaderResource* Get(); 
private:
	FSimulationShaderResource()
	{
	}
};

void SIMULATIONCORE_API DispatchExampleComputeShader_RenderThread(FRHICommandList& RHICmdList, FSimulationShaderResource* Resource, float Scale, float Translate, uint32 ThreadGroupX, uint32 ThreadGroupY, uint32 ThreadGroupZ);
void SIMULATIONCORE_API DispatchExampleComputeShader_GameThread(float InputVal, float Scale, float Translate, FSimulationShaderResource* Resource);
float SIMULATIONCORE_API GetGPUReadback(FSimulationShaderResource* Resource, float& OutputVal);