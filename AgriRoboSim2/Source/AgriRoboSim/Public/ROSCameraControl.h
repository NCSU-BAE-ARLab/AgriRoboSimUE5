// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "ImageUtils.h"
#include "ImageCore.h"
#include "Camera/CameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "sensor_msgs/Image.h"
#include "ROSCameraControl.generated.h"

UENUM(BlueprintType)
enum class ECaptureType : uint8
{
	Unset,
	ColorCapture,
	SegmentationCapture,
	DepthCapture
};

USTRUCT()
struct FROSSceneCapture
{
	GENERATED_BODY()
	//UPROPERTY()
	UTopic* Topic;
	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;
	ECaptureType CaptureType;
	//EPixelFormat ROSEncoding;
	ETextureRenderTargetFormat RenderTargetFormat;
	UPROPERTY()
	TArray<FColor> ImageData8Bit;
	TArray<FFloat16Color> ImageData16Bit;
	UPROPERTY()
	TArray<FLinearColor> ImageData32Bit;
	// Default constructor
	FROSSceneCapture()
		: Topic(nullptr)
		, SceneCapture(nullptr)
		, CaptureType(ECaptureType::Unset)
		, RenderTargetFormat(RTF_RGBA8)
		//, ROSEncoding()
		, ROSStepMultiplier(-1)
		, ImageMSG(nullptr)
		, img(nullptr)
		
	{
	}

	FROSSceneCapture(
		UTopic* Topic_,
		USceneCaptureComponent2D* SceneCapture_,
		ECaptureType CaptureType_
		)
		: ImageMSG(nullptr)
		, img(nullptr)
	{
		Topic = Topic_;
		SceneCapture = SceneCapture_;
		CaptureType = CaptureType_;
		switch (CaptureType)
		{
		case ECaptureType::ColorCapture:
		case ECaptureType::SegmentationCapture:
			RenderTargetFormat = RTF_RGBA8;
			ROSStepMultiplier = 3;
			break;
		case ECaptureType::DepthCapture:
			RenderTargetFormat = RTF_RGBA32f;
			ROSStepMultiplier = 4;
			break;
		default:
			break;
		}
	}
	
	void RefreshImageTopicSize();
	void Publish();
	template<typename T>
	void Publish(TArray<T>* Image);/*
	bool UpdateImageMsgColor(TArray<FColor>* Image, uint8* data);
	bool UpdateImageMsgLinearColor(TArray<FLinearColor>* Image, uint8* data);
	bool UpdateImageMsg16BitColor(TArray<FFloat16Color>* Image, uint8* data);*/
	template<typename T>
	bool UpdateImageMsg(TArray<T>* Image, uint8* data);
	
	FString CheckROSEncoding();
	
	void ReadRenderTargetPerRHI();

	void UpdateSceneCaptureCameraParameters(UCameraComponent* Camera, UWorld* WorldContext);
private:
	int ROSStepMultiplier;
	TSharedPtr<ROSMessages::sensor_msgs::Image> ImageMSG;
	std::shared_ptr<uint8[]> img;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGRIROBOSIM_API UROSCameraControl : public UROS2NodeComponent
{
	GENERATED_BODY()
private:
	UFUNCTION()
    void InitSceneCaptures();
	UFUNCTION()
    void InitSceneCapture(USceneCaptureComponent2D* SceneCapture, ECaptureType CaptureType);
public:
	UFUNCTION(BlueprintCallable)
    void InitROSTopics(
    	TMap<USceneCaptureComponent2D*, ECaptureType> CameraTypePair,
    	UCameraComponent* CameraModel, int Width, int Height);
	// Sets default values for this component's properties
	UROSCameraControl();

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TSharedPtr<FROSSceneCapture>> SceneCaptures;

	UPROPERTY()
    UTopic* Color_Topic;
    UPROPERTY()
    UTopic* Depth_Topic;
	UPROPERTY()
	UTopic* Segment_Topic;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/*UFUNCTION(BlueprintCallable)
	void ReadRenderTargetPerRHI(UTextureRenderTarget2D *RenderTarget);

	UFUNCTION(BlueprintCallable)
	void ReadRenderTargetPerUtil(UTextureRenderTarget2D *RenderTarget);*/

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* CreateRenderTarget(int Width, int Height, ETextureRenderTargetFormat Format = RTF_RGBA16f);

	UFUNCTION(BlueprintCallable)
	void PublishSelectTopic(FName TopicName);
	UFUNCTION(BlueprintCallable)
	void PublishAllTopic();

	UFUNCTION(BlueprintCallable)
	void UpdateAllCameraParameters(UCameraComponent* Camera);
	UFUNCTION(BlueprintCallable)
	void UpdateAllCameraSize(int Width, int Height);
};

