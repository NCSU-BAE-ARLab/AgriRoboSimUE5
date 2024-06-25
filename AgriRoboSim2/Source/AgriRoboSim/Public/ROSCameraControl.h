// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "ImageUtils.h"
#include "ImageCore.h"
#include "Engine/TextureRenderTarget2D.h"
#include "sensor_msgs/Image.h"
#include "ROSCameraControl.generated.h"

USTRUCT()
struct FROSSceneCapture
{
	GENERATED_BODY()
	//UPROPERTY()
	UTopic* Topic;
	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;
	
	EPixelFormat ROSEncoding;
	UPROPERTY()
	TArray<FColor> ImageData8Bit;
	TArray<FFloat16Color> ImageData16Bit;
	UPROPERTY()
	TArray<FLinearColor> ImageData32Bit;
	// Default constructor
	FROSSceneCapture()
		: Topic(nullptr)
		, SceneCapture(nullptr)
		, ROSEncoding()
		, ROSStepMultiplier(-1)
		, ImageMSG(nullptr)
		, img(nullptr)
	{
	}

	
	void RefreshImageSize();
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
public:

	UFUNCTION(BlueprintCallable)
	void InitROSTopics();
	// Sets default values for this component's properties
	UROSCameraControl();

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TSharedPtr<FROSSceneCapture>> SceneCaptures;
	

	UPROPERTY(BlueprintReadOnly)
	TArray<FColor> ImageData8Bit;
	
	//UPROPERTY(BlueprintReadOnly)
	TArray<FFloat16Color> ImageData16Bit;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLinearColor> ImageData32Bit;

	//UPROPERTY(BlueprintReadOnly)
	FImage ImageDataUtil;
	
	UPROPERTY()
    UTopic* Color_Topic;
    UPROPERTY()
    UTopic* Depth_Topic;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void PublishSceneCaptureToTopic();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ReadRenderTargetPerRHI(UTextureRenderTarget2D *RenderTarget);

	UFUNCTION(BlueprintCallable)
	void ReadRenderTargetPerUtil(UTextureRenderTarget2D *RenderTarget);

	UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* CreateRenderTarget(int Width, int Height, ETextureRenderTargetFormat Format = RTF_RGBA16f);

	UFUNCTION(BlueprintCallable)
	void PublishSelectTopic(FName TopicName);
	UFUNCTION(BlueprintCallable)
	void PublishAllTopic();
	
};

