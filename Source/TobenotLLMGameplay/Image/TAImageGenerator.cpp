// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

// TAImageGenerator.cpp

#include "TAImageGenerator.h"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Common/TALLMLibrary.h"

#include "Engine/Texture2DDynamic.h"

#include "Event/Core/TAEventPool.h"
#include "Event/Core/TAEventSubsystem.h"
#include "Scene/TAPlaceActor.h"
#include "Save/TAGuidSubsystem.h"

UTAImageGenerator::UTAImageGenerator()
{
	OnDownloadCompleteDelegate.AddDynamic(this, &UTAImageGenerator::OnDownloadComplete);
	OnDownloadFailedDelegate.AddDynamic(this, &UTAImageGenerator::OnDownloadFailed);
}

// 请求生成图片的实现
void UTAImageGenerator::RequestGenerateImage(const FTAEventInfo& EventInfo)
{
	// 将当前事件信息保存到这个对象，为了在回调中使用
	BoundEventID = EventInfo.EventID;

	// 从 EventInfo 的描述出发，发起下载图片的异步请求
	// 注意：这里我们直接使用UTASystemLibrary提供的函数和委托类型
	CacheOpenAIChat = UTALLMLibrary::DownloadImageFromPollinations(
		EventInfo.LocationName+' '+EventInfo.Description,
		OnDownloadCompleteDelegate, OnDownloadFailedDelegate);
}

void UTAImageGenerator::OnDownloadComplete(UTexture2DDynamic* Texture)
{
	if (Texture != nullptr)
	{
		// 设置到位点上
		UTAEventSubsystem* EventSubsystem = GetWorld()->GetSubsystem<UTAEventSubsystem>();
		if (EventSubsystem)
		{
			auto Pool = EventSubsystem->GetEventPool();
			if(Pool)
			{
				bool bFlag;
				FTAEventInfo& BoundEventInfo = Pool->GetEventByID(BoundEventID,bFlag);
				if(bFlag)
				{
					UTAGuidSubsystem* GuidSubsystem = GetWorld()->GetSubsystem<UTAGuidSubsystem>();
					if(GuidSubsystem)
					{
						ATAPlaceActor* PlaceActor = Cast<ATAPlaceActor>(GuidSubsystem->GetActorByGUID(BoundEventInfo.LocationGuid));
						PlaceActor->SetPlaceTexture(Texture);
					}
				}
			}
		}
		
		// 下面是存文件
		// 将纹理转换为2D纹理资源
		FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource());
		if (TextureResource != nullptr)
		{
			FTexture2DRHIRef TextureRHI;
			TArray<FColor> SurfData;
			ENQUEUE_RENDER_COMMAND(CheckTextureRHICommand)(
				[TextureResource, &TextureRHI, &SurfData](FRHICommandListImmediate& RHICmdList)
				{
					if (TextureResource != nullptr)
					{
						// 可选的额外检查，确认RHI资源是否已经创建。
						if (TextureResource->IsInitialized())
						{
							// 在这里，您处于渲染线程上下文中
							TextureRHI = TextureResource->TextureRHI;
							if (TextureRHI.IsValid())
							{
								// 读取像素数据
								if (TextureRHI.IsValid())
								{
									RHICmdList.ReadSurfaceData(
										TextureRHI,
										FIntRect(0, 0, TextureRHI->GetSizeX(), TextureRHI->GetSizeY()),
										SurfData,
										FReadSurfaceDataFlags()
									);
								}
							}
						}
					}
				}
			);

			// 确保渲染命令被执行
			FlushRenderingCommands();

			// 检查是否已经读取了数据
			if (SurfData.Num() > 0)
			{
				// 将纹理数据保存到PNG文件
				int32 Width = TextureRHI->GetSizeX();
				int32 Height = TextureRHI->GetSizeY();
				FString SavePath = FPaths::ProjectSavedDir() / TEXT("DownloadedTexture/Texture");
				FString DataTimeStr = FDateTime::UtcNow().ToIso8601();
				DataTimeStr.ReplaceInline(TEXT("-"), TEXT(""));
				DataTimeStr.ReplaceInline(TEXT(":"), TEXT(""));
				DataTimeStr.ReplaceInline(TEXT("."), TEXT(""));
				SavePath = SavePath + DataTimeStr + TEXT(".png"); // 添加文件扩展名为.png

				// 使用ImageWrapper模块中的功能来保存PNG
				IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
				TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

				if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(SurfData.GetData(), SurfData.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8))
				{
					const TArray64<uint8>& PngData = ImageWrapper->GetCompressed();
					FFileHelper::SaveArrayToFile(PngData, *SavePath);
				}

				if (FPaths::FileExists(SavePath))
				{
					UE_LOG(LogTemp, Log, TEXT("Image saved to %s"), *SavePath);
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Image saved failed to %s"), *SavePath);
				}
			}
		}
	}
}

void UTAImageGenerator::OnDownloadFailed(UTexture2DDynamic* Texture)
{
	UE_LOG(LogTemp, Log, TEXT("Texture DownloadFailed"));
}