#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VoxelMeshAssetFactory.generated.h"

UCLASS()
class UVoxelMeshAssetFactory : public UFactory
{
    GENERATED_BODY()

public:
    UVoxelMeshAssetFactory();

    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName Name,
        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

    virtual bool ShouldShowInNewMenu() const override { return true; }
};
