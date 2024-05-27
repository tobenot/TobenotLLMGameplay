// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Scene/TASceneSubsystem.h"

#include "NavigationSystem.h"
#include "Scene/TAPlaceActor.h"
#include "Event/Data/TAEventInfo.h"
#include "Scene/TASceneLogCategory.h"
#include "TASettings.h"
#include "Scene/TAAreaScene.h"

FString UTASceneSubsystem::QuerySceneMapInfo()
{
	return "The Mushroom Village on the grassland, where the mushroom monsters are hostile, mushroom villagers may be friendly.";
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
		AreaScenesMap.Add(EventInfo.PresetData.EventID, NewAreaScene);
		NewAreaScene->LoadAreaScene(EventInfo);
	}
	return NewAreaScene; // 返回创建的实例
}

void UTASceneSubsystem::PopulateMapWithMonsters(const TArray<FVector>& ForbiddenLocations)
{
	UE_LOG(LogTASceneSystem, Error, TEXT("PopulateMapWithMonsters Deprecated"));
	return;
	
    // 检查世界是否有效
    if (GetWorld() == nullptr)
    {
        UE_LOG(LogTASceneSystem, Error, TEXT("Invalid world in PopulateMapWithMonsters"));
        return;
    }
    bHasPopulateMapWithMonsters = true;

    // 初始化随机数生成器
    FRandomStream RandomStream;
    RandomStream.GenerateNewSeed();

    // 定义地图生成怪物的配置项
    constexpr float Interval = 800.f;   // 间隔
    constexpr float SafeZoneRadius = 2000.f; // PlayerStart 安全区域半径
	// 定义方形地图的边界
	constexpr float MinX = 200.f;
	constexpr float MaxX = 19800.f;
	constexpr float MinY = 200.f;
	constexpr float MaxY = 4800.f;

    FVector GenStartLocation = FVector::ZeroVector;

    // 获取怪物类引用
    UClass* MonsterClass = GetMonsterClass();
    if (!MonsterClass || MonsterClass == AActor::StaticClass())
    {
        UE_LOG(LogTASceneSystem, Error, TEXT("Invalid MonsterClass in PopulateMapWithMonsters"));
        return;
    }

    int32 GeneratedMonsterCount = 0;

	// 在地图上随机生成怪物
	for (float X = MinX; X < MaxX; X += Interval)
	{
		for (float Y = MinY; Y < MaxY; Y += Interval)
		{
			FVector RandomLocation = FVector(X, Y, 0.f) + GenStartLocation;
			RandomLocation.Z = GenStartLocation.Z; // 假设地图是平坦的，使用PlayerStart的Z值

            // 确保不在ForbiddenLocations中的位置生成怪物
            bool bIsForbiddenLocation = false;
            for (const FVector& ForbiddenLocation : ForbiddenLocations)
            {
                if (FVector::Dist2D(RandomLocation, ForbiddenLocation) < SafeZoneRadius)
                {
                    bIsForbiddenLocation = true;
                    break;
                }
            }

            if (!bIsForbiddenLocation)
            {
                // 使用导航系统来确保点是可达的
                FNavLocation NavLocation;
                UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
                if (NavSys && NavSys->GetRandomPointInNavigableRadius(RandomLocation, Interval, NavLocation, nullptr))
                {
                    // 在这个位置生成一个怪物
                    AActor* NewMonster = GetWorld()->SpawnActor<AActor>(MonsterClass, NavLocation.Location, FRotator::ZeroRotator);
                    if (NewMonster)
                    {
                        GeneratedMonsterCount++;
                    }
                }
            }
        }
    }

    UE_LOG(LogTASceneSystem, Log, TEXT("怪物生成完毕，生成数量：%d"), GeneratedMonsterCount);
}

ATAPlaceActor* UTASceneSubsystem::QueryEventLocationByInfo(const FTAEventInfo& EventInfo)
{
    UE_LOG(LogTASceneSystem, Log, TEXT("开始查询事件位置信息..."));
    // ... 这里可以引入基于EventInfo的更复杂的逻辑 ...
    bool bIsValidLocation;
    int32 RetryCount = 0;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NavLocation;

    do
    {
        bIsValidLocation = true;
        constexpr float MinX = 2000.0f + 200.f;
        float MaxX = 20000.0f- 200.f;
        constexpr float MinY = 0.0f + 200.f;
        constexpr float MaxY = 5000.0f - 200.f; 
    	if (!bHasCreatedStartingEvent)
    	{
    		MaxX = 4000.0f; // 定义起始区域的X坐标
    		bHasCreatedStartingEvent = true; // 更新标志位，表明起始事件已经创建
    	}
        FVector RandomPoint(FMath::FRandRange(MinX, MaxX), FMath::FRandRange(MinY, MaxY), 0);

        constexpr float MinRadius = 1000.0f;
        constexpr float MaxRadius = 1500.0f;
        const float RandomRadius = FMath::RandRange(MinRadius, MaxRadius);

        // 使用导航系统来确保点是可达的
    	if (!NavSys || !NavSys->GetRandomPointInNavigableRadius(RandomPoint, RandomRadius, NavLocation, nullptr))
    	{
    		UE_LOG(LogTASceneSystem, Warning, TEXT("Failed to find a navigable point."));
    		bIsValidLocation = false;
    	}

    	if(bIsValidLocation)
    	{
    		// 测试新生成的点与现有位点之间的距离
    		for (ATAPlaceActor* PlaceActor : PlaceActors)
    		{
    			if (PlaceActor)
    			{
    				float Distance = (PlaceActor->GetActorLocation() - NavLocation.Location).Size();
    				if (Distance < (PlaceActor->PlaceRadius + RandomRadius))
    				{
    					bIsValidLocation = false;
    					break;
    				}
    			}
    		}
    	}

        if (bIsValidLocation)
        {
            UE_LOG(LogTASceneSystem, Log, TEXT("位置有效，位置选取重试次数：%d，开始创建并添加新的位点到列表中..."), RetryCount);
            // 创建并添加新的位点到列表中
            ATAPlaceActor* NewPlaceActor = CreateAndAddPlace(NavLocation.Location, RandomRadius, EventInfo.PresetData.LocationName);
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
                break;
            }
        }

    } while (!bIsValidLocation);

    return nullptr;
}


UClass* UTASceneSubsystem::GetMonsterClass() const
{
	UClass* MonsterClass = nullptr;

	// Fetch the default class path from settings
	const UTASettings* Settings = GetDefault<UTASettings>();
	if (Settings)
	{
		MonsterClass = Settings->MonsterClass.TryLoadClass<AActor>();
	}

	// If no specified class or class loading fails, use the default AActor class
	if (!MonsterClass)
	{
		MonsterClass = AActor::StaticClass();
	}

	return MonsterClass;
}

ATAPlaceActor* UTASceneSubsystem::CreatePlaceActorAtLocation(const FVector& NewLocation, float NewRadius, const FString& NewName)
{
	if (GetWorld())
	{
		ATAPlaceActor* NewPlaceActor = CreateAndAddPlace(NewLocation, NewRadius, NewName);
		if (NewPlaceActor)
		{
			// 位点创建成功，可以在这里执行任何额外的初始化逻辑
			UE_LOG(LogTASceneSystem, Log, TEXT("位点Actor在坐标 %s 创建成功。"), *NewLocation.ToString());
			return NewPlaceActor;
		}
		else
		{
			// 处理创建失败的情况
			UE_LOG(LogTASceneSystem, Warning, TEXT("位点Actor创建失败。"));
		}
	}
	else
	{
		UE_LOG(LogTASceneSystem, Error, TEXT("无法获取世界上下文（UWorld*），位点Actor创建失败。"));
	}
	return nullptr;
}