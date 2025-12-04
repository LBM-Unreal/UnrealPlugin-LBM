#include "VoxelizationModule.h"
#include "Modules/ModuleManager.h"
#include "SimulationCore.h"
#include "VoxelGridActor.h"
#include "VoxelizationCore.h"


DEFINE_LOG_CATEGORY(LogVoxelization);

void FVoxelizationModule::StartupModule()
{
    UE_LOG(LogVoxelization, Log, TEXT("Voxelization module started"));
	FString ShaderDir = FPaths::Combine(FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Shaders/Voxelization"));
	AddShaderSourceDirectoryMapping(TEXT("/VoxelizationShaderDir"), ShaderDir);
}

void FVoxelizationModule::ShutdownModule()
{
    UE_LOG(LogVoxelization, Log, TEXT("Voxelization module shutdown"));
}

IMPLEMENT_MODULE(FVoxelizationModule, Voxelization)

