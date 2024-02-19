// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Save/TAGuidInterface.h"
#include "TAPlaceActor.generated.h"

UCLASS()
class TOBENOTLLMGAMEPLAY_API ATAPlaceActor : public AActor
	,public ITAGuidInterface
{
	GENERATED_BODY()
public:
	// Place radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
	float PlaceRadius;

	// Place name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
	FString PlaceName;
	
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
	
	// Used to show the place's area of effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Place")
	class USphereComponent* AreaDisplaySphere;
};
