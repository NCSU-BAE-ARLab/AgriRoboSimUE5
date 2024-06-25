// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "ROSIntegration/Classes/RI/Topic.h"
#include "ROSIntegration/Classes/ROSIntegrationGameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "sensor_msgs/JointState.h"
#include "TextureResource.h"
#include "ROSActor.generated.h"
/**
#define ROS2Sim_Str_Topic_STR TEXT("test_str_in")
#define Sim2ROS_Str_Topic_STR TEXT("test_str_out")
#define ROS2Sim_Float_Topic_STR TEXT("test_float")

#define BOTTOM_JOINTS_TOPIC_ID std::string("/vec3_1")
#define TOP_JOINTS_TOPIC_ID std::string("/vec3_2")
**/

#define UE5_TOPIC_PREFIX std::string("/unreal")
#define WORLD_POS_TOPIC std::string("/reachedgoal")
#define TAKE_IMAGE_TOPIC std::string("/takedata")
#define JOINTS_TOPIC_ID std::string("/joint_states")

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
		void SaveCamData(FString FilePath, TArray<FString> SavedData);
	//UFUNCTION(BlueprintCallable)
	//	void ReadRTPixels(UTextureRenderTarget2D* SceneCaptureComp);

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
	// current joint positions for ros publishing
	UPROPERTY(BlueprintReadWrite)
		TArray<FString> UE5JointName;
	UPROPERTY(BlueprintReadWrite)
		TArray<double> UE5JointPosition;
	UPROPERTY(BlueprintReadWrite)
		TArray<double> UE5JointPositionErr;
	// Store joint state message from call back
	UPROPERTY(BlueprintReadWrite)
		TArray<FString> TopicName;		// the joint name
	UPROPERTY(BlueprintReadWrite)
		TArray<double> TopicPosition;	// the position of the joint (rad or m),
	UPROPERTY(BlueprintReadWrite)
		TArray<double> TopicVelocity;	// the velocity of the joint (rad/s or m/s)
	UPROPERTY(BlueprintReadWrite)
		TArray<double> TopicEffort;		// the effort that is applied in the joint (Nm or N)
private:
	UPROPERTY()
		UROSIntegrationGameInstance* rosinst;
	UPROPERTY()
		UTopic* Sim2ROS_Vec3_0_WorldPos_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Bool_Topic;
	UPROPERTY()
		UTopic* ROS2Sim_Joints_Topic;
	UPROPERTY()
		UTopic* Sim2ROS_Joints_Topic;
	//UPROPERTY()
	TSharedPtr<ROSMessages::sensor_msgs::JointState> JointPosMSG;
	std::function<void(TSharedPtr<FROSBaseMsg>)> JointsSubscribeCallback;
	std::function<void(TSharedPtr<FROSBaseMsg>)> Bool_SubscribeCallback;
};
