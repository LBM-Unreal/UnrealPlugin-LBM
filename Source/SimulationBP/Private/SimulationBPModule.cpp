// SimulationBPModule.cpp
#include "SimulationBPModule.h"


IMPLEMENT_MODULE(FSimulationBPModule, SimulationBP)
void FSimulationBPModule::StartupModule()
{ }


void SIMULATIONBP_API USimulationLibrary::UpdateVoxelData3D()
{
	//auto Dim = FSimulationShaderResource3D::Get()->Params.SimDimensions;
	//constexpr int BlockSize = 4;
	//ENQUEUE_RENDER_COMMAND(FUpdateVoxelData)([Dim](FRHICommandListImmediate& RHICmdList)
	//	{
	//		DispatchCopyVoxelGridToSim_RenderThread(RHICmdList, FSimulationShaderResource3D::Get()->DebugBuffer.UAV, Dim,
	//			(Dim.X + BlockSize - 1) / BlockSize,
	//			(Dim.Y + BlockSize - 1) / BlockSize,
	//			(Dim.Z + BlockSize - 1) / BlockSize);
	//	});
}