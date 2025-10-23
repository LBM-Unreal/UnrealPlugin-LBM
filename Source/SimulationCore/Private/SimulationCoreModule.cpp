// SimulationCoreModule.cpp
#include "SimulationCoreModule.h"

IMPLEMENT_MODULE(FSimulationCoreModule, SimulationCore)


void FSimulationCoreModule::StartupModule()
{
    UE_LOG(LogTemp, Warning, TEXT("SimulationCoreModule Started"));
    FString ShaderDir = FPaths::Combine(FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/LBM/Shaders"), ShaderDir);
}