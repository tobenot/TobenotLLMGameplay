// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Save/TAGuidInterface.h"
#include "UObject/NoExportTypes.h"
#include "TAPlaceActor.generated.h"

// Declare delegates for broadcasting changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaceNameChanged, const FString&, NewPlaceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTextureChanged, UTexture2DDynamic*, NewTexture);

UCLASS()
class TOBENOTLLMGAMEPLAY_API ATAPlaceActor : public AActor
    , public ITAGuidInterface
{
    GENERATED_BODY()

public:
    // Place radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
    float PlaceRadius;

    // Place name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
    FString PlaceName;

    // Dynamic texture for the place
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
    UTexture2DDynamic* PlaceTexture;

    // Events for broadcasting changes
    UPROPERTY(BlueprintAssignable, Category = "Place")
    FOnPlaceNameChanged OnPlaceNameChanged;

    UPROPERTY(BlueprintAssignable, Category = "Place")
    FOnTextureChanged OnTextureChanged;

public:
    // Sets default values for this actor's properties
    ATAPlaceActor();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void SetPlaceRadius(float Radius);
    void SetPlaceName(const FString& NewName);
    void SetPlaceTexture(UTexture2DDynamic* NewTexture);

    // Used to show the place's area of effect
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Place")
    class USphereComponent* AreaDisplaySphere;
};
