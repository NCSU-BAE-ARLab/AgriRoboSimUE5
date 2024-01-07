// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "ROSIntegration/Classes/RI/Topic.h"
#include "ROSIntegration/Classes/ROSIntegrationGameInstance.h"
#include "Engine/TextureRenderTarget2D.h"

#include "ROSActor.generated.h"

#define ROS2Sim_Str_Topic_STR TEXT("test_str_in")
#define Sim2ROS_Str_Topic_STR TEXT("test_str_out")
#define ROS2Sim_Float_Topic_STR TEXT("test_float")

#define UE5_TOPIC_PREFIX std::string("/unreal")
#define BOTTOM_JOINTS_TOPIC_ID std::string("/vec3_1")
#define TOP_JOINTS_TOPIC_ID std::string("/vec3_2")
#define WORLD_POS_TOPIC std::string("/reachedgoal")
#define TAKE_IMAGE_TOPIC std::string("/takedata")


UCLASS()

class AGRIROBOSIM_API AROSActor : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AROSActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
		void saveCamData(FString filePath, TArray<FString> savedData);
	//UFUNCTION(BlueprintCallable)
	//	void ReadRTPixels(UTextureRenderTarget2D* SceneCaptureComp);
	float time_accumulation = 0;
	float reset_time = 0.1f;

	UPROPERTY(BlueprintReadWrite)
		bool received_msg = false;
	//UPROPERTY(BlueprintReadWrite)
	//	float set_arm_pos = 0.0;
	//UPROPERTY(BlueprintReadWrite)
	//	float set_wrist_pos = 0.0;
	UPROPERTY(BlueprintReadWrite)
		FVector FirstSet;
	UPROPERTY(BlueprintReadWrite)
		FVector SecondSet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 ArmID;
	UPROPERTY(BlueprintReadWrite)
		FVector EEFJointPos;
	UPROPERTY(BlueprintReadWrite)
		bool TakeData;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UTextureRenderTarget2D *CamImage;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int ImageHeight = 1080;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int ImageWidth = 1920;
private:
	UPROPERTY()
		UROSIntegrationGameInstance* rosinst;
	UPROPERTY()
		UTopic* Sim2ROS_Str_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Str_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Vec3_1_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Vec3_2_Topic;
	UPROPERTY()
		UTopic* Sim2ROS_Vec3_0_WorldPos_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Bool_Topic;
	std::function<void(TSharedPtr<FROSBaseMsg>)> STRSubscribeCallback;
	std::function<void(TSharedPtr<FROSBaseMsg>)> Vec3_1SubscribeCallback;
	std::function<void(TSharedPtr<FROSBaseMsg>)> Vec3_2SubscribeCallback;
	std::function<void(TSharedPtr<FROSBaseMsg>)> Bool_SubscribeCallback;
	int ImageSize;
	TArray<FColor> ImageData;
};
