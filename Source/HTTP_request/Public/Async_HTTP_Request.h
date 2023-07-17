// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "Http.h"
#include "HttpModule.h"
//#include "RuntimeArchiverBase.h"
#include "Async_HTTP_Request.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMyHTTPRequestCompletedSignature, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMyHTTPRequestProgressSignature, int32, BytesSent, int32, BytesReceived);

UENUM(BlueprintType)
enum class EHTTPRequestVerb : uint8
{
    GET,
    POST,
    PUT,
    DELETE
};


USTRUCT(BlueprintType)
struct FUnzipData
{
    GENERATED_BODY()

        //UPROPERTY(EditAnywhere, BlueprintReadWrite)
        //TSubclassOf<URuntimeArchiverBase> ArchiverClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bForceOverwrite = true;
};

USTRUCT(BlueprintType)
struct FDirectory
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString UnarchiveDirectoryPath = "cookies/";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString DownloadArchivePath = "cookies/";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString EntryName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bAddParentDirectory;
};

USTRUCT(BlueprintType)
struct FHttpRequestData
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        EHTTPRequestVerb Verb;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString URL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Content;
};


UCLASS()
class HTTP_REQUEST_API UAsync_HTTP_Request : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

        UFUNCTION(BlueprintCallable, Category = "HTTP")
        static UAsync_HTTP_Request* DownloadAndUnzipContent(FHttpRequestData HttpRequestData, FUnzipData UnzipData, FDirectory Directory);

    UPROPERTY(BlueprintAssignable)
        FMyHTTPRequestCompletedSignature OnRequestCompleted;

    UPROPERTY(BlueprintAssignable)
        FMyHTTPRequestProgressSignature OnRequestProgress;

    virtual void Activate() override;

private:
    void HandleHTTPRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void HandleRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);


    FString LongUnarchiveDirectoryPath;
    FString LongDownloadArchivePath;
    FHttpRequestData HttpRequestData;
    FUnzipData UnzipData;
    FDirectory Directory;

    void ExtractAssets(const FString& ArchivePath, const FString& DestinationPath);
    //FString SavePath = FPaths::ProjectContentDir() + "cookies/" + "001.zip";

    /** Start unarchiving directory operation */
    void MyUnarchivingToStorage();

    FString GetAndroidContentDirectory();

    /** Used archiver */
/*
    UPROPERTY()
#if ENGINE_MAJOR_VERSION >= 5
        TObjectPtr<URuntimeArchiverBase> MyArchiver;
#else
        URuntimeArchiverBase* Archiver;
#endif
*/  
  /** Operation result delegate */
    //FRuntimeArchiverAsyncOperationResult UnarchiveOperationResult;

    /** Operation in progress delegate */
    //FRuntimeArchiverAsyncOperationProgress UnarchiveOperationProgress;

    UFUNCTION()
        void UnarchiveOnResult_Callback(bool bSuccess);
};