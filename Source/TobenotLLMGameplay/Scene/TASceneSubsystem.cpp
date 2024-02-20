// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "TASceneSubsystem.h"
#include "TAPlaceActor.h"
#include "Event/Data/TAEventInfo.h"
#include "TASceneLogCategory.h"
#include "TASettings.h"
#include "TAAreaScene.h"

FString UTASceneSubsystem::QuerySceneMapInfo()
{
	return "The Mushroom Village on the grassland, where the Mushroom Monsters are hostile.";
}

FString UTASceneSubsystem::QueryLocationInfo(const FVector& Location)
{
	return "High GrassLand";
}

ATAPlaceActor* UTASceneSubsystem::CreateAndAddPlace(const FVector& Location, float Radius, const FString& Name)
{
	// 假设这个方法被正确的调用在允许创建Actor的上下文中
	if (UWorld* World = GetWorld())
	{
		UClass* PlaceActorClass = nullptr;
        
		// 从配置文件中获取类路径
		const UTASettings* Settings = GetDefault<UTASettings>();
		if (Settings)
		{
			PlaceActorClass = Settings->PlaceActorClass.TryLoadClass<ATAPlaceActor>();
		}

		// 如果没有指定类或者类加载失败，使用默认的ATAPlaceActor类
		if (!PlaceActorClass)
		{
			PlaceActorClass = ATAPlaceActor::StaticClass();
		}

		// 使用加载的类生成Actor实例
		ATAPlaceActor* NewPlaceActor = World->SpawnActor<ATAPlaceActor>(PlaceActorClass, Location, FRotator::ZeroRotator);
		if (NewPlaceActor)
		{
			NewPlaceActor->SetPlaceName(Name);
			NewPlaceActor->SetPlaceRadius(Radius);
			PlaceActors.Add(NewPlaceActor);
			ITAGuidInterface* GuidInterface = Cast<ITAGuidInterface>(NewPlaceActor);
			if (GuidInterface)
			{
				// todo: save recoverable name
				const FString GuidNameStr = FString::Printf(TEXT("%s%d"),*Name, PlaceActors.Num());
				GuidInterface->RegisterActorTAGuid(NewPlaceActor, FName(*GuidNameStr));
			}
			return NewPlaceActor;
		}
	}
	return nullptr;
}

UTAAreaScene* UTASceneSubsystem::CreateAndLoadAreaScene(const FTAEventInfo& EventInfo)
{
	// 创建区域地图实例
	UTAAreaScene* NewAreaScene = NewObject<UTAAreaScene>(this, UTAAreaScene::StaticClass());
	if (NewAreaScene)
	{
		AreaScenesMap.Add(EventInfo.EventID, NewAreaScene);
		NewAreaScene->LoadAreaScene(EventInfo);
	}
	return NewAreaScene; // 返回创建的实例
}

ATAPlaceActor* UTASceneSubsystem::QueryEventLocationByInfo(const FTAEventInfo& EventInfo)
{
    UE_LOG(LogTASceneSystem, Log, TEXT("开始查询事件位置信息..."));
    // ... 这里可以引入基于EventInfo的更复杂的逻辑 ...
    bool bIsValidLocation;
    int32 RetryCount = 0;

    do
    {
        bIsValidLocation = true;
        float MinX = 500.0f;
        float MaxX = 1500.0f;
        float MinY = -500.0f;
        float MaxY = 500.0f;

        FVector RandomPoint(FMath::FRandRange(MinX, MaxX), FMath::FRandRange(MinY, MaxY), 0);

        float MinRadius = 100.0f;
        float MaxRadius = 300.0f;
        float RandomRadius = FMath::RandRange(MinRadius, MaxRadius);

        // 测试新生成的点与现有位点之间的距离
        for (ATAPlaceActor* PlaceActor : PlaceActors)
        {
            if (PlaceActor)
            {
                float Distance = (PlaceActor->GetActorLocation() - RandomPoint).Size();
                if (Distance < (PlaceActor->PlaceRadius + RandomRadius))
                {
                    bIsValidLocation = false;
                    break;
                }
            }
        }

        if (bIsValidLocation)
        {
            UE_LOG(LogTASceneSystem, Log, TEXT("位置有效，位置选取重试次数： %d，开始创建并添加新的位点到列表中... "),RetryCount);
            // 创建并添加新的位点到列表中
            ATAPlaceActor* NewPlaceActor = CreateAndAddPlace(RandomPoint, RandomRadius, EventInfo.LocationName);
            return NewPlaceActor;
        }
        else
        {
            // 如果位置不合适，增加重试计数
            RetryCount++;

            // 如果达到最大重试次数，退出循环
            if (RetryCount >= MaxQueryEventLocationRetryCount)
            {
                UE_LOG(LogTASceneSystem, Error, TEXT("达到最大重试次数%d，查询失败，返回空指针。"), MaxQueryEventLocationRetryCount);
                // 可以在这里添加适当的处理，比如打印日志或者抛出异常
                break;
            }
        }

    } while (!bIsValidLocation);

    return nullptr;
}
