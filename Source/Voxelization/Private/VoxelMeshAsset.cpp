#include "VoxelMeshAsset.h"

#if WITH_EDITOR
void UVoxelMeshAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UVoxelMeshAsset, SourceMesh))
    {
        if (SourceMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("Source mesh for %s set to %s"), *GetName(), *SourceMesh->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Source mesh cleared for %s"), *GetName());
        }
    }
}
#endif