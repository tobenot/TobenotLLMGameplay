// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TASaveGameSubsystem.h"

#include "EngineUtils.h"
#include "TAGuidInterface.h"
#include "TASaveGame.h"
#include "Chat/TAChatComponent.h"
#include "Kismet/GameplayStatics.h"


DECLARE_LOG_CATEGORY_EXTERN(logTASave, Log, All);
DEFINE_LOG_CATEGORY(logTASave);

void UTASaveGameSubsystem::StoreAllTAData()
{
	//创建一个存档实例
	SaveGameInstance = Cast<UTASaveGame>(UGameplayStatics::CreateSaveGameObject(UTASaveGame::StaticClass()));

	//打印日志，表示开始存储所有TA数据
	UE_LOG(logTASave, Display, TEXT("StoreAllTAData - Begin storing all TA data."));
	
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		ITAGuidInterface* TAGuidInterface = Cast<ITAGuidInterface>(Actor);
		if (TAGuidInterface)
		{
			// 调用序列化Actor数据的函数
			SerializeActorData(Actor, TAGuidInterface);
		}
	}
	
	//存储NameGuidMap到存档实例中
	SaveGameInstance->NameGuidMap = this->NameGuidMap;
	
	//存档
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, "TASaveGameSlot", 0);

	//打印日志，表示存储完成
	UE_LOG(logTASave, Display, TEXT("StoreAllTAData - All TA data stored successfully."));
}

void UTASaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTASaveGameSubsystem::AllowRestoreNameTAGuidMap()
{
	hasRestoreNameTAGuidMap = false;
}

void UTASaveGameSubsystem::RestoreNameTAGuidMap()
{
	if(hasRestoreNameTAGuidMap)
	{
		UE_LOG(logTASave, Warning, TEXT("RestoreNameTAGuidMap - Please load only once, now return"));
		return;
	}
	//从存档槽中加载存档实例
	SaveGameInstance = Cast<UTASaveGame>(UGameplayStatics::LoadGameFromSlot("TASaveGameSlot", 0));
    
	if(!SaveGameInstance)
	{
		//如果没有找到对应的存档，就创建一个
		SaveGameInstance = Cast<UTASaveGame>(UGameplayStatics::CreateSaveGameObject(UTASaveGame::StaticClass()));
		UE_LOG(logTASave, Display, TEXT("RestoreNameTAGuidMap - No TASave Data found, Create New Save."));
		if(!SaveGameInstance)
		{
			UE_LOG(logTASave, Error, TEXT("RestoreNameTAGuidMap - Create New Save Failed, return."));
			return;
		}
	}

	hasRestoreNameTAGuidMap = true;
	//恢复NameGuidMap
	this->NameGuidMap = SaveGameInstance->NameGuidMap;

	UE_LOG(logTASave, Display, TEXT("RestoreNameTAGuidMap - Restored NameGuidMap data."));
}

/*void UTASaveGameSubsystem::RestoreAllTAData()
{
	//从存档槽中加载存档实例
	UTASaveGame* SaveGameInstance = Cast<UTASaveGame>(UGameplayStatics::LoadGameFromSlot("TASaveGameSlot", 0));
    
	if(!SaveGameInstance)
	{
		//如果没有找到对应的存档，就直接返回
		UE_LOG(logTASave, Display, TEXT("RestoreAllTAData - No TASave Data found."));
		return;
	}
	
	//打印日志，表示开始恢复所有TA数据
	UE_LOG(logTASave, Display, TEXT("RestoreAllTAData - Begin restoring all TA data."));
	
	//我们需要遍历所有带有UTAChatComponent的actor
	for (TObjectIterator<UTAChatComponent> Itr; Itr; ++Itr)
	{
		//这是一个带有UTAChatComponent组件的actor
		UTAChatComponent* ChatComponent = *Itr;

		//通过ITAGuid接口获取该actor的TAGuid
		ITAGuidInterface* TAGuidInterface = Cast<ITAGuidInterface>(ChatComponent->GetOwner());
		if (TAGuidInterface)
		{
			FGuid ActorGuid = TAGuidInterface->GetTAGuid();

			//查找对应的存档聊天数据
			const FTAChatComponentSaveData* ActorChatData = SaveGameInstance->TAChatDataMap.Find(ActorGuid);

			if (ActorChatData)
			{
				//如果找到，那么就用存档的聊天数据来设置这个ChatComponent
				ChatComponent->SetChatHistoryData(*ActorChatData);
			}
		}
	}
	
	//打印日志，表示恢复完成
	UE_LOG(logTASave, Display, TEXT("RestoreAllTAData - All TA data restored successfully."));
}*/

void UTASaveGameSubsystem::RegisterActorTAGuid(AActor* Actor, FName Name)
{
	if(!hasRestoreNameTAGuidMap)
	{
		UE_LOG(logTASave, Error, TEXT("RegisterActorTAGuid befroe 'RestoreNameTAGuidMap'!!! Please be mindful of the timing; you might be calling 'RestoreNameTAGuidMap' too late! Consider trying call 'RestoreNameTAGuidMap' in 'InitGame' in the GameMode!"));
		return;
	}
	UE_LOG(logTASave, Display, TEXT("RegisterActorTAGuid"));
	if (Actor)
	{
		ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(Actor);
		if(GuidInterface)
		{
			if (NameGuidMap.Contains(Name) && NameGuidMap[Name].IsValid()) 
			{
				// 如果NameGuidMap已经包含了这个名字，我们就取出来并设置给Actor
				FGuid StoredGuid = NameGuidMap[Name];
				GuidInterface->SetTAGuid(StoredGuid);
				// 恢复存档数据给Actor
				RestoreActorData(Actor, GuidInterface, StoredGuid);
				UE_LOG(logTASave, Display, TEXT("RegisterActorTAGuid - RestoreActorData"));
			} 
			else
			{
				// 如果没有，我们就为Actor生成一个新的GUID并添加进NameGuidMap
				FGuid NewGuid = FGuid::NewGuid();
				GuidInterface->SetTAGuid(NewGuid);
				NameGuidMap.Add(Name, NewGuid);
				UE_LOG(logTASave, Display, TEXT("RegisterActorTAGuid - NewGuid"));
			}
			UE_LOG(logTASave, Display, TEXT("RegisterActorTAGuid - Actor: %s , NameGuid: %s， Guid: %s"), *Actor->GetName(), *Name.ToString(), *GuidInterface->GetTAGuid().ToString());
		}else
		{
			UE_LOG(logTASave, Display, TEXT("RegisterActorTAGuid - Actor is not ITAGuidInterface"));
		}
	}
}

void UTASaveGameSubsystem::SerializeActorData(AActor* Actor, ITAGuidInterface* TAGuidInterface)
{
	FGuid ActorGuid = TAGuidInterface->GetTAGuid();
            
	//获取ChatComponent的聊天数据
	UTAChatComponent* ChatComponent = Actor->FindComponentByClass<UTAChatComponent>();
	if (ChatComponent)
	{
		FTAChatComponentSaveData ActorChatData = ChatComponent->GetChatHistoryData();

		//把Actor的聊天数据和相应的TAGuid一起保存到存档实例中
		SaveGameInstance->TAChatDataMap.Add(ActorGuid, ActorChatData);
	}
	FString SerializedData = TAGuidInterface->SerializeCustomData();
	// 将序列化数据存储到TMap中
	SaveGameInstance->SerializedDataMap.Add(ActorGuid, SerializedData);
	// 需要存储的其他数据也可以在这里添加相应的逻辑
}

//新增函数，根据Guid恢复Actor数据
void UTASaveGameSubsystem::RestoreActorData(AActor* Actor, ITAGuidInterface* TAGuidInterface, FGuid ActorGuid)
{
	if(!SaveGameInstance)
	{
		//如果没有找到对应的存档，就直接返回
		UE_LOG(logTASave, Error, TEXT("RestoreActorData - No TASave Data found. Have you call RestoreNameTAGuidMap in GameMode's StartPlay? "));
		return;
	}

	//查找对应的存档聊天数据
	const FTAChatComponentSaveData* ActorChatData = SaveGameInstance->TAChatDataMap.Find(ActorGuid);

	if (ActorChatData)
	{
		UTAChatComponent* ChatComponent = Actor->FindComponentByClass<UTAChatComponent>();
		if (ChatComponent)
		{
			//如果找到，那么就用存档的聊天数据来设置这个ChatComponent
			ChatComponent->SetChatHistoryData(*ActorChatData);
		}
	}
	const FString* SerializedData = SaveGameInstance->SerializedDataMap.Find(ActorGuid);
	if (SerializedData)
	{
		// 存在序列化数据，调用接口方法进行反序列化操作
		TAGuidInterface->DeserializeCustomData(*SerializedData);
	}
}

void UTASaveGameSubsystem::UnregisterActorName(FName Name)
{
	if (NameGuidMap.Contains(Name))
	{
		NameGuidMap.Remove(Name);
		UE_LOG(logTASave, Display, TEXT("Unregister %s from NameGuidMap."), *Name.ToString());
	}
	else 
	{
		UE_LOG(logTASave, Warning, TEXT("Attempted to unregister a non-existent entry %s from NameGuidMap"), *Name.ToString());
	}
}

// 查找一个名字对应的GUID
FGuid UTASaveGameSubsystem::FindNamedTAGuid(FName Name) 
{
	FGuid ResultGuid;
	if (NameGuidMap.Contains(Name)) 
	{
		ResultGuid = NameGuidMap[Name];
	}
	return ResultGuid;
}