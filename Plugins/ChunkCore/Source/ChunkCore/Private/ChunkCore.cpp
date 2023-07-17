// Copyright (C) 2017-2021 | eelDev (Dry Eel Development)

#include "ChunkCore.h"
#include "ChunkCoreModule.h"
#include "ChunkCoreLogging.h"
#include <ChunkDownloader.h>
#include <Misc/CoreDelegates.h>
#include <IPlatformFilePak.h>
#include <HAL/PlatformFilemanager.h>

UChunkCore* UChunkCore::m_ChunkCoreSubsystem = nullptr;

void UChunkCore::InitChunkCore(FString platformName)
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetOrCreate();

	ChunkLoader->Initialize(platformName, 8);

	ChunkLoader->OnChunkMounted.AddLambda([](uint32 ChunkId, bool bSuccess)
		{
			m_ChunkCoreSubsystem->OnChunkMounted.Broadcast(ChunkId, bSuccess);
		});

	auto LambdaCallback = [](const FString& FileName, const FString& Url, uint64 SizeBytes, const FTimespan& DownloadTime, int32 HttpStatus)
	{
		m_ChunkCoreSubsystem->OnDownloadAnalytics.Broadcast(FileName, Url, LexToString(SizeBytes), DownloadTime, HttpStatus);
	};

	ChunkLoader->OnDownloadAnalytics = LambdaCallback;
}

void UChunkCore::Finalize()
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	ChunkLoader->Finalize();
}

bool UChunkCore::LoadCachedBuild(FString DeploymentName)
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->LoadCachedBuild(DeploymentName);
}

void UChunkCore::UpdateBuild(FString DeploymentName, FString ContentBuildId, const FOnUpdateBuildCallback& Callback)
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaFunc = [Callback](bool bSuccess)
	{
		LogVerbose("Callback bSuccess: %d", bSuccess);

		Callback.ExecuteIfBound(bSuccess);
	};

	ChunkLoader->UpdateBuild(DeploymentName, ContentBuildId, LambdaFunc);
}

void UChunkCore::MountChunks(TArray<int32> ChunkIds, const FOnMountChunksCallback& Callback)
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaCallback = [Callback](bool bSuccess)
	{
		Callback.ExecuteIfBound(bSuccess);
	};

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (!FCoreDelegates::OnMountPak.IsBound())
	{
		FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)(FPlatformFileManager::Get().GetPlatformFile(FPakPlatformFile::GetTypeName()));
		PakPlatformFile->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT(""));
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	ChunkLoader->MountChunks(ChunkIds, LambdaCallback);
}

void UChunkCore::MountChunk(int32 ChunkId, const FOnMountChunkCallback& Callback)
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaCallback = [Callback](bool bSuccess)
	{
		Callback.ExecuteIfBound(bSuccess);
	};

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (!FCoreDelegates::OnMountPak.IsBound())
	{
		FPakPlatformFile* PakPlatformFile = (FPakPlatformFile*)(FPlatformFileManager::Get().GetPlatformFile(FPakPlatformFile::GetTypeName()));
		PakPlatformFile->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT(""));
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	ChunkLoader->MountChunk(ChunkId, LambdaCallback);
}

void UChunkCore::DownloadChunks(TArray<int32> ChunkIds, const FOnDownloadChunksCallback& Callback, int32 Priority /*= 0*/)
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaCallback = [Callback](bool bSuccess)
	{
		Callback.ExecuteIfBound(bSuccess);
	};

	ChunkLoader->DownloadChunks(ChunkIds, LambdaCallback, Priority);
}

void UChunkCore::DownloadChunk(int32 ChunkId, const FOnDownloadChunkCallback& Callback, int32 Priority /*= 0*/)
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaCallback = [Callback](bool bSuccess)
	{
		Callback.ExecuteIfBound(bSuccess);
	};

	ChunkLoader->DownloadChunk(ChunkId, LambdaCallback, Priority);
}

int32 UChunkCore::FlushCache()
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->FlushCache();
}

int32 UChunkCore::ValidateCache()
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->ValidateCache();
}

void UChunkCore::BeginLoadingMode(const FOnBeginLoadingModeCallback& Callback)
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	auto LambdaCallback = [Callback](bool bSuccess)
	{
		Callback.ExecuteIfBound(bSuccess);
	};

	ChunkLoader->BeginLoadingMode(LambdaCallback);
}

FString UChunkCore::GetContentBuildId() 
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->GetContentBuildId();
}

FString UChunkCore::GetDeploymentName() 
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->GetDeploymentName();
}

EChunkCoreStatus UChunkCore::GetChunkStatus(int32 ChunkId)
{
	LogVerbose("");

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return static_cast<EChunkCoreStatus>(ChunkLoader->GetChunkStatus(ChunkId));
}

void UChunkCore::GetAllChunkIds(TArray<int32>& ChunkIds)
{
	LogVerbose("");

	ChunkIds.Empty();

	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();
	ChunkLoader->GetAllChunkIds(ChunkIds);
}

FChunkCoreStats UChunkCore::GetLoadingStats() 
{
	TSharedRef<FChunkDownloader> ChunkLoader = FChunkDownloader::GetChecked();

	return ChunkLoader->GetLoadingStats();
}

void UChunkCore::Initialize(FSubsystemCollectionBase& Collection)
{
	LogVerbose("");

	m_ChunkCoreSubsystem = this;
}

void UChunkCore::Deinitialize()
{
	LogVerbose("");

	FChunkDownloader::Shutdown();
}

