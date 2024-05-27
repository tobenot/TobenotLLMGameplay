// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TAGameLibrary.generated.h"

USTRUCT(BlueprintType)
struct FCategoryTextureData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CategoryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Texture;
};

UCLASS()
class TOBENOTLLMGAMEPLAY_API UTAGameLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Utility|Textures")
    static TSoftObjectPtr<UTexture2D> GetRandomTextureFromCategory(const UDataTable* DataTable, const FName& CategoryName);
};
