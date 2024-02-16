// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TASaveGameSubsystem.generated.h"


class ITAGuidInterface;
class UTASaveGame;
/**
 *  
 */
UCLASS()
class TOBENOTLLMGAMEPLAY_API UTASaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem BEGIN
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// USubsystem END
	
	// 一局游戏只读一次档就行了，回主菜单记得调用一下这个
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void AllowRestoreNameTAGuidMap();
	
	// Call it in GameMode's InitGame!!!
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void RestoreNameTAGuidMap();

	// Call it when server save game
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void StoreAllTAData();

	// 现在是每个actor生成时自己来拿存档
	//UFUNCTION(BlueprintCallable, Category = "SaveGame")
	//void RestoreAllTAData();
	
	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	void RegisterActorTAGuid(AActor* Actor, FName Name);

	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	void UnregisterActorName(FName Name);
	
	UFUNCTION(BlueprintCallable, Category = "TAGuid")
	FGuid FindNamedTAGuid(FName Name);
	
private:
	bool hasRestoreNameTAGuidMap = false;
	
	// 这里定义了Actor定位名和GUID的对应关系
	TMap<FName, FGuid> NameGuidMap;
	
	void SerializeActorData(AActor* Actor, ITAGuidInterface* TAGuidInterface);
	
	void RestoreActorData(AActor* Actor, ITAGuidInterface* TAGuidInterface, FGuid ActorGuid);

	UPROPERTY()
	UTASaveGame* SaveGameInstance;
};
