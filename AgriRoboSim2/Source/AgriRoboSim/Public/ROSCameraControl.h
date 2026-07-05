// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "ImageUtils.h"
#include "ImageCore.h"
#include "Camera/CameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "sensor_msgs/Image.h"
#include "ROSSceneCapture.h"
#include "ROSCameraControl.generated.h"


UENUM(BlueprintType)
enum class EROSCameraMode : uint8 {
     Streaming = 0 UMETA(DisplayName = "ROS Stream Images"),
     Block = 1  UMETA(DisplayName = "ROS Block and Publish Images")
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
	UPROPERTY()
	TArray<UROSSceneCapture*> SceneCaptures;

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
	void SetCameraMode(EROSCameraMode Mode);
	UFUNCTION(BlueprintCallable)
	void UpdateAllCameraSize(int Width, int Height);
	UFUNCTION(BlueprintCallable)
	void UpdateFrameIDSources(TArray<AActor*> FrameIDSources);
};

