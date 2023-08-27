// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSActor.h"

#include <string>

#include "GameFramework/Actor.h"
#include "ROSIntegration/Classes/RI/Topic.h"
#include "ROSIntegration/Classes/ROSIntegrationGameInstance.h"
#include "ROSIntegration/Public/std_msgs/String.h"
#include "ROSIntegration/Public/std_msgs/Float32.h"
#include "ROSIntegration/Public/std_msgs/Bool.h"
#include "ROSIntegration/Public/geometry_msgs/Vector3.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
// Sets default values
AROSActor::AROSActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AROSActor::BeginPlay()
{
	Super::BeginPlay();
	ImageSize = ImageHeight * ImageWidth;
	ImageData.AddUninitialized(ImageSize);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ROS Actor Created!"));
	UE_LOG(LogTemp, Log, TEXT("ROS Actor Created!"));
	rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());
	// Initialize a topic
	Sim2ROS_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Str_Topic->Init(rosinst->ROSIntegrationCore, Sim2ROS_Str_Topic_STR, TEXT("std_msgs/String"));
	Sim2ROS_Str_Topic->Advertise();
	
	// Publish a string to the topic
	TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("ROS Actor in Simulation Started"));
	Sim2ROS_Str_Topic->Publish(StringMessage);

	// Create a std::function callback object
	std::function<void(TSharedPtr<FROSBaseMsg>)> SubscribeCallback = [](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::String>(msg);
		if (Concrete.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Incoming string was: %s"), (*(Concrete->_Data)));
		}
		return;
	};

	// Subscribe to the topic
	Sim2ROS_Str_Topic->Subscribe(SubscribeCallback);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("3"));

	ROS2Sim_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Str_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Str_Topic_STR, TEXT("std_msgs/String"));
	ROS2Sim_Str_Topic->Advertise();

	auto Vec3_1_Topic_Name = FString((UE5_TOPIC_PREFIX + std::string("/") +
											std::to_string(ArmID) + BOTTOM_JOINTS_TOPIC_ID).c_str());
	auto Vec3_2_Topic_Name = FString((UE5_TOPIC_PREFIX + std::string("/") +
											std::to_string(ArmID) + TOP_JOINTS_TOPIC_ID).c_str());
	auto Vec3_WorldPos_Topic_Name =FString((UE5_TOPIC_PREFIX + std::string("/") +
											std::to_string(ArmID) + WORLD_POS_TOPIC).c_str());
	auto Bool_Topic_Name = FString((UE5_TOPIC_PREFIX + std::string("/") +
											std::to_string(ArmID) + TAKE_IMAGE_TOPIC).c_str());

	ROS2Sim_Vec3_1_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_1_Topic->Init(rosinst->ROSIntegrationCore, Vec3_1_Topic_Name, TEXT("geometry_msgs/Vector3"));
	ROS2Sim_Vec3_1_Topic->Advertise();

	ROS2Sim_Vec3_2_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_2_Topic->Init(rosinst->ROSIntegrationCore, Vec3_2_Topic_Name, TEXT("geometry_msgs/Vector3"));
	ROS2Sim_Vec3_2_Topic->Advertise();

	Sim2ROS_Vec3_0_WorldPos_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Vec3_0_WorldPos_Topic->Init(rosinst->ROSIntegrationCore, Vec3_WorldPos_Topic_Name, TEXT("geometry_msgs/Vector3"));
	
	ROS2Sim_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Bool_Topic->Init(rosinst->ROSIntegrationCore, Bool_Topic_Name, TEXT("std_msgs/Bool"));
	ROS2Sim_Bool_Topic->Advertise();
	STRSubscribeCallback = [str_topic = Sim2ROS_Str_Topic, rm = &received_msg](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::String>(msg);
		if (Concrete.IsValid())
		{
			TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim"));
			//str_topic->Publish(StringMessage);
			*rm = true;
		}
		*rm = false;
		return *rm;
	};
	// Subscribe callback to joint angles
	// first 3 angles
	Vec3_1SubscribeCallback = [str_topic = Sim2ROS_Str_Topic, fs = &FirstSet](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::geometry_msgs::Vector3>(msg);
		if (Concrete.IsValid())
		{
			(fs->X) = Concrete->x;
			(fs->Y) = Concrete->y;
			(fs->Z) = Concrete->z;
			return true;
		}
		return false;
	};
	// second 3 angles
	Vec3_2SubscribeCallback = [str_topic = Sim2ROS_Str_Topic, ss = &SecondSet](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::geometry_msgs::Vector3>(msg);
		if (Concrete.IsValid())
		{
			(ss->X) = Concrete->x;
			(ss->Y) = Concrete->y;
			(ss->Z) = Concrete->z;
			return true;
		}
		return false;
	};
	// Subscriber callback to data collection signal
	Bool_SubscribeCallback = [str_topic = Sim2ROS_Str_Topic, take = &TakeData](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("Incoming data was: %f"), ((Concrete->_Data)));
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Received a Bool from ROS"));

			TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim (bool)"));
			//str_topic->Publish(StringMessage);
			*take = Concrete->_Data;
			return true;
		}
		return false;
	};
	received_msg |= ROS2Sim_Str_Topic->Subscribe(STRSubscribeCallback);
	received_msg |= ROS2Sim_Vec3_1_Topic->Subscribe(Vec3_1SubscribeCallback);
	received_msg |= ROS2Sim_Vec3_2_Topic->Subscribe(Vec3_2SubscribeCallback);
	received_msg |= ROS2Sim_Bool_Topic->Subscribe(Bool_SubscribeCallback);
	TSharedPtr<ROSMessages::geometry_msgs::Vector3> RobotPos(new ROSMessages::geometry_msgs::Vector3(EEFJointPos));
	Sim2ROS_Vec3_0_WorldPos_Topic->Publish(RobotPos);
}

// Called every frame
void AROSActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TSharedPtr<ROSMessages::geometry_msgs::Vector3> RobotPos(new ROSMessages::geometry_msgs::Vector3(EEFJointPos));
	
	Sim2ROS_Vec3_0_WorldPos_Topic->Publish(RobotPos);
}

void AROSActor::saveCamData(FString filePath, TArray<FString> savedData) {
	FString finalString = "";
	for (FString& Each : savedData) {
		finalString += Each;
		finalString += LINE_TERMINATOR;
	}
	if (!FFileHelper::SaveStringToFile(finalString, *filePath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Failed to Save Cam Location"));
	}
}
void AROSActor::ReadRTPixels(UTextureRenderTarget2D* SceneCaptureComp)
{
	FTextureRenderTargetResource* RenderTargetResource = SceneCaptureComp->GameThread_GetRenderTargetResource();
	bool bReadSuccess = RenderTargetResource->ReadPixels(ImageData);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("%d"), bReadSuccess);
}