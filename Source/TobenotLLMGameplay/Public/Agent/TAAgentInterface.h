#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TAAgentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType, NotBlueprintable)
class TOBENOTLLMGAMEPLAY_API UTAAgentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TOBENOTLLMGAMEPLAY_API ITAAgentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
	public:
	virtual FString GetSystemPrompt() = 0;

	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual FString GetPersonalityPrompt() const = 0;
    
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual const FString& GetAgentName() const = 0;

	virtual int32 GetAgentSpeakPriority() const;

	// 增加或更新Agent的欲望
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual void AddOrUpdateDesire(const FGuid& DesireId, const FString& DesireDescription);
    
	// 移除Agent的欲望
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual void RemoveDesire(const FGuid& DesireId);

	// 目前的作用：如果是旁白的话，UTAShoutManager就不会检查消息前面有没有带着Agent名字
	virtual bool IsVoiceover() const;
    
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual TSoftObjectPtr<UTexture2D> GetAgentPortrait() const
	{
		return TSoftObjectPtr<UTexture2D>();
	}

	UFUNCTION(BlueprintCallable, Category = "TA|Inventory")
	virtual TMap<FName, int32> QueryInventoryItems() const;

	UFUNCTION(BlueprintCallable, Category = "TA|Inventory")
	virtual int32 QueryItemAmountByName(FName ItemName) const;
    
	UFUNCTION(BlueprintCallable, Category = "TA|Inventory")
	virtual bool ConsumeInventoryItem(FName ItemName, int32 ConsumeCount);
	
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual FString GetPerceptionData() const;
	
	UFUNCTION(BlueprintCallable, Category = "TA|Agent")
	virtual FString GetMemoryData() const;
};