// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "Scene/TAPlaceActor.h"
#include "Components/SphereComponent.h"

// Sets default values
ATAPlaceActor::ATAPlaceActor()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the sphere component to visualize the place's area
	AreaDisplaySphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaDisplaySphere"));
	AreaDisplaySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = AreaDisplaySphere;

	// Default place radius
	PlaceRadius = 100.0f;

	// By default, update the sphere radius to match our place radius
	AreaDisplaySphere->SetSphereRadius(PlaceRadius);
}

// Called when the game starts or when spawned
void ATAPlaceActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATAPlaceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATAPlaceActor::SetPlaceRadius(float Radius)
{
	PlaceRadius = Radius;
	// Ensure the sphere component always matches the specified place radius
	if(AreaDisplaySphere->GetUnscaledSphereRadius() != PlaceRadius)
	{
		AreaDisplaySphere->SetSphereRadius(PlaceRadius);
	}
}

// Implementation of SetPlaceName
void ATAPlaceActor::SetPlaceName(const FString& NewName)
{
	if (PlaceName != NewName)
	{
		PlaceName = NewName;
		OnPlaceNameChanged.Broadcast(NewName);
	}
}

// Implementation of SetPlaceTexture
void ATAPlaceActor::SetPlaceTexture(UTexture2DDynamic* NewTexture)
{
	if (PlaceTexture != NewTexture)
	{
		PlaceTexture = NewTexture;
		OnTextureChanged.Broadcast(NewTexture);
	}
}
