// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TAGuidInterface.generated.h"

UINTERFACE(BlueprintType, NotBlueprintable)
class UTAGuidInterface : public UInterface
{
	GENERATED_BODY()
};

class TOBENOTLLMGAMEPLAY_API ITAGuidInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	virtual FGuid GetTAGuid();

	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	virtual void RegisterActorTAGuid(AActor* Actor, FName Name);
    
	virtual void SetTAGuid(FGuid NewGuid);

	virtual FString SerializeCustomData();
	
	virtual void DeserializeCustomData(const FString& SerializedData);

	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	virtual FName GetIdentityPositionName() const
	{
		return IdentityPositionName;
	}

	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	virtual void SetIdentityPositionName(FName NewIdentityPositionName)
	{
		IdentityPositionName = NewIdentityPositionName;
	}

private:
	FGuid TAGuid;
	
	FName IdentityPositionName;
};
