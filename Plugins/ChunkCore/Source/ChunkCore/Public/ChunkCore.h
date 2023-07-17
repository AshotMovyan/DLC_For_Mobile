// Copyright (C) 2017-2021 | eelDev (Dry Eel Development)

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <ChunkDownloader.h>
#include "ChunkCore.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnUpdateBuildCallback, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMountChunksCallback, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMountChunkCallback, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDownloadChunksCallback, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDownloadChunkCallback, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBeginLoadingModeCallback, bool, bWasSuccessful);

// Called whenever a chunk mounts (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN FOR MOUNTS (otherwise use the proper request Callback on MountChunk)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChunkMounted, int32, ChunkId, bool, bSuccess);

// Called each time a download attempt finishes (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN. Downloads retry until successful.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnDownloadAnalytics, const FString&, FileName, const FString&, URL, FString, SizeBytes, const FTimespan&, DownloadTime, int32, HttpStatus);

USTRUCT(BlueprintType)
struct FChunkCoreStats
{
	GENERATED_BODY()
public:
	FChunkCoreStats()
		: FilesDownloaded(0)
		  , TotalFilesToDownload(0)
		  , ChunksMounted(0)
		  , TotalChunksToMount(0)
		  , LoadingStartTime(FDateTime::MinValue())
	{
	}

	FChunkCoreStats(const FChunkDownloader::FStats& data)
		: FilesDownloaded(data.FilesDownloaded)
		  , TotalFilesToDownload(data.TotalFilesToDownload)
		  , BytesDownloaded(LexToString(data.BytesDownloaded))
		  , TotalBytesToDownload(LexToString(data.TotalBytesToDownload))
		  , ChunksMounted(data.ChunksMounted)
		  , TotalChunksToMount(data.TotalChunksToMount)
		  , LoadingStartTime(data.LoadingStartTime)
		  , LastError(data.LastError)
	{
	}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	int32 FilesDownloaded;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	int32 TotalFilesToDownload;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FString BytesDownloaded;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FString TotalBytesToDownload;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	int32 ChunksMounted;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	int32 TotalChunksToMount;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FDateTime LoadingStartTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
	FText LastError;
};

UENUM(BlueprintType)
enum class EChunkCoreStatus : uint8
{
	Mounted,
	// chunk is cached locally and mounted in RAM
	Cached,
	// chunk is fully cached locally but not mounted
	Downloading,
	// chunk is partially cached locally, not mounted, download in progress
	Partial,
	// chunk is partially cached locally, not mounted, download NOT in progress
	Remote,
	// no local caching has started
	Unknown,
	// no paks are included in this chunk, can consider it either an error or fully mounted depending
};

UCLASS()
class CHUNKCORE_API UChunkCore : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/*
	* Called whenever a chunk mounts (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN FOR MOUNTS (otherwise use the proper request Callback on MountChunk)
	*/
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnChunkMounted OnChunkMounted;
	/*
	* Called each time a download attempt finishes (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN. Downloads retry until successful.
	*/
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnDownloadAnalytics OnDownloadAnalytics;
public:
	/*
	* Initialize the download manager (populates the list of cached pak files from disk). Call only once.
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void InitChunkCore(FString PlatformName);

	/* 
	* Unmount all chunks and cancel any downloads in progress (preserving partial downloads). 
	* Call only once, don't reuse this object, make a new one.
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void Finalize();

	/*
	* Try to load a cached build ID from disk (good to do before updating build so it can possibly no-op)
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static bool LoadCachedBuild(FString DeploymentName);

	/*
	 * Set the the content build id
	 * If the content build id has changed, we pull the new BuildManifest from CDN and load it.
	 * The client should compare ContentBuildId with its current embedded build id to determine if this content is
	 * Even compatible BEFORE calling this function. e.g. ContentBuildId="v1.4.22-r23928293" we might consider BUILD_VERSION="1.4.1"
	 * Compatible but BUILD_VERSION="1.3.223" incompatible (needing an update)
	 */
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void UpdateBuild(FString DeploymentName, FString ContentBuildId, const FOnUpdateBuildCallback& Callback);

	/*
	* Download and mount all chunks then fire the Callback (convenience wrapper managing multiple MountChunk calls)
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void MountChunks(TArray<int32> ChunkIds, const FOnMountChunksCallback& Callback);

	/*
	* Download all pak files, then asynchronously mount them in order (in order among themselves, async with game thread). 
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void MountChunk(int32 ChunkId, const FOnMountChunkCallback& Callback);

	/*
	* Download (Cache) all pak files in these chunks then fire the Callback (convenience wrapper managing multiple DownloadChunk calls)
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void DownloadChunks(TArray<int32> ChunkIds, const FOnDownloadChunksCallback& Callback, int32 Priority = 0);

	/*
	* Download all pak files in the chunk, but don't mount.
	* 
	* Callback is fired when all paks have finished caching 
	* (whether success or failure). Downloads will retry forever, but might fail due to space issues.
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void DownloadChunk(int32 ChunkId, const FOnDownloadChunkCallback& Callback, int32 Priority = 0);

	/*
	* Flush any cached files (on disk) that are not currently being downloaded to or mounting (does not unmount the corresponding pak files).
	* This will include full and partial downloads, but not active downloads.
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static int32 FlushCache();

	/*
	* Validate all fully cached files (blocking) by attempting to read them and check their Version hash.
	* This automatically deletes any files that don't match. Returns the number of files deleted.
	* In this case best to return to a simple update map and reinitialize ChunkDownloader (or restart).
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static int32 ValidateCache();

	/*
	* Snapshot stats and enter into loading screen mode (pauses all background downloads). 
	* Fires Callback when all non-background downloads have completed. 
	* If no downloads/mounts are currently queued by the end of the frame, Callback will fire next frame.
	*/
	UFUNCTION(BlueprintCallable, Category = "ChunkCore")
	static void BeginLoadingMode(const FOnBeginLoadingModeCallback& Callback);

	/*
	* Get the current content build ID
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ChunkCore")
	static FString GetContentBuildId();

	/*
	* Get the most recent deployment name
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ChunkCore")
	static FString GetDeploymentName();

	/* 
	* Get the current status of the specified chunk
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ChunkCore")
	static EChunkCoreStatus GetChunkStatus(int32 ChunkId);

	/*
	* Return a list of all chunk IDs in the current manifest
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ChunkCore")
	static void GetAllChunkIds(TArray<int32>& ChunkIds);

	/*
	* get the current loading stats (generally only useful if you're in loading mode see BeginLoadingMode)
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ChunkCore")
	static FChunkCoreStats GetLoadingStats();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	static UChunkCore* m_ChunkCoreSubsystem;
};
