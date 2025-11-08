#include "VoxelMeshAssetFactory.h"
#include "VoxelMeshAsset.h"

UVoxelMeshAssetFactory::UVoxelMeshAssetFactory()
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = UVoxelMeshAsset::StaticClass();
}

UObject* UVoxelMeshAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName Name,
    EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UVoxelMeshAsset>(InParent, InClass, Name, Flags);
}
