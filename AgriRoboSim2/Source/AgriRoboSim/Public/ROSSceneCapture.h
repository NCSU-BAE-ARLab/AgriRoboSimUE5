// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "sensor_msgs/Image.h"
#include "ROSSceneCapture.generated.h"

UENUM(BlueprintType)
enum class ECaptureType : uint8
{
	Unset,
	ColorCapture,
	SegmentationCapture,
	DepthCapture
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGRIROBOSIM_API UROSSceneCapture : public UObject
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UROSSceneCapture();
	UPROPERTY()
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
	void Initialize(
		UROSIntegrationGameInstance* rosinst,
		FString& topic_name,
		USceneCaptureComponent2D* SceneCapture_,
		ECaptureType CaptureType_
	);
protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

public:
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	//                           FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	void RefreshImageTopicSize();
	UFUNCTION()
	void Publish();
	template<typename T>
	UFUNCTION()
	void Publish(TArray<T>* Image);
	template<typename T>
	UFUNCTION()
	bool UpdateImageMsg(TArray<T>* Image, uint8* data);
	UFUNCTION()
	FString CheckROSEncoding();
	UFUNCTION()
	void ReadRenderTargetPerRHIBlock();
	UFUNCTION()
	void ReadRenderTargetPerRHIStream();
	UFUNCTION()
	void UpdateFrameID(bool force = false);
	UFUNCTION()
	void UpdateSceneCaptureCameraParameters(UCameraComponent* Camera, UWorld* WorldContext);

	int ROSStepMultiplier;
	TSharedPtr<ROSMessages::sensor_msgs::Image> ImageMSG;
	std::shared_ptr<uint8[]> img;
	ROSMessages::std_msgs::Header msg_header;
	FString topic_name;
	FString frame_id;
	UPROPERTY()
	TArray<AActor*> IDSources;

	bool is_publishing;
	bool is_streaming;
};
