#include "Voxelization.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogVoxelization);

void FVoxelizationModule::StartupModule()
{
    UE_LOG(LogVoxelization, Log, TEXT("Voxelization module started"));
}

void FVoxelizationModule::ShutdownModule()
{
    UE_LOG(LogVoxelization, Log, TEXT("Voxelization module shutdown"));
}

IMPLEMENT_MODULE(FVoxelizationModule, Voxelization)

