// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


#include "Common/TASystemLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

FString UTASystemLibrary::CurrentGameLanguage;

void UTASystemLibrary::SetGlobalProxyAddress(const FString& ProxyAddress, int32 ProxyPort)
{
	// Construct the full proxy address by combining the IP address and port
	const FString FullProxyAddress = FString::Printf(TEXT("%s:%d"), *ProxyAddress, ProxyPort);

	// Set the proxy address in the FHttpModule
	FHttpModule::Get().SetProxyAddress(FullProxyAddress);
}

void UTASystemLibrary::ClearGlobalProxy()
{
	// Set the proxy address to an empty string to clear the proxy
	FHttpModule::Get().SetProxyAddress(TEXT(""));
}

void UTASystemLibrary::DownloadTextFile(const FString& URL, const FOnTextFileDownloaded& OnDownloadComplete, const FOnTextFileDownloaded& OnDownloadFailed)
{
	const TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	// Use Lambda to handle callbacks
	HttpRequest->OnProcessRequestComplete().BindLambda([OnDownloadComplete, OnDownloadFailed](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		OnResponseReceived(Request, Response, bWasSuccessful, OnDownloadComplete, OnDownloadFailed);
	});

	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));

	HttpRequest->ProcessRequest();
}

void UTASystemLibrary::ClipboardActionCopy(const FString& Context)
{
	FPlatformApplicationMisc::ClipboardCopy(*Context);
}

void UTASystemLibrary::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful,
                                      const FOnTextFileDownloaded& OnDownloadComplete, const FOnTextFileDownloaded& OnDownloadFailed)
{
	if (bWasSuccessful && Response.IsValid())
	{
		FString DownloadedText = Response->GetContentAsString();
		OnDownloadComplete.ExecuteIfBound(DownloadedText);
	}
	else
	{
		OnDownloadFailed.ExecuteIfBound("");
	}
}

void UTASystemLibrary::SetGameLanguage(const FString& Language)
{
	CurrentGameLanguage = Language;
}

FString UTASystemLibrary::GetGameLanguage()
{
	return CurrentGameLanguage;
}

void UTASystemLibrary::TASetCurrentCulture(FString Culture)
{
	FInternationalization& I18N = FInternationalization::Get();
	I18N.SetCurrentCulture(Culture);
}
