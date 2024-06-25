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
#include "sensor_msgs/CameraInfo.h"
#include "sensor_msgs/JointState.h"
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ROS Actor Created!"));
	UE_LOG(LogTemp, Log, TEXT("ROS Actor Created!"));

	rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());
	
	
	// Subscribe to the topic
	auto const Vec3_WorldPos_Topic_Name =FString((UE5_TOPIC_PREFIX + std::string("/") +
												std::to_string(ArmID) + WORLD_POS_TOPIC).c_str());
	auto const Bool_Topic_Name = FString((UE5_TOPIC_PREFIX + std::string("/") +
												std::to_string(ArmID) + TAKE_IMAGE_TOPIC).c_str());
	auto const Sim2ROS_Joints_Topic_Name = FString((UE5_TOPIC_PREFIX + std::string("/") +
												std::to_string(ArmID) + JOINTS_TOPIC_ID).c_str());
	auto const ROS2Sim_Joints_Topic_Name = FString((JOINTS_TOPIC_ID).c_str());
	// Initialize topics
	Sim2ROS_Vec3_0_WorldPos_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Vec3_0_WorldPos_Topic->Init(rosinst->ROSIntegrationCore, Vec3_WorldPos_Topic_Name, TEXT("geometry_msgs/Vector3"));

	Sim2ROS_Joints_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Joints_Topic->Init(rosinst->ROSIntegrationCore, Sim2ROS_Joints_Topic_Name, TEXT("sensor_msgs/JointState"));

	ROS2Sim_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Bool_Topic->Init(rosinst->ROSIntegrationCore, Bool_Topic_Name, TEXT("std_msgs/Bool"));
	ROS2Sim_Bool_Topic->Advertise();

	ROS2Sim_Joints_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Joints_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Joints_Topic_Name, TEXT("sensor_msgs/JointState"));
	ROS2Sim_Joints_Topic->Advertise();

	// Subscribe callback to joint angles
	JointsSubscribeCallback = [jn = &TopicName, pos = &TopicPosition, vel = &TopicVelocity, effort = &TopicEffort](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::sensor_msgs::JointState>(msg);
		if (Concrete.IsValid())
		{
			jn->Reset();
			pos->Reset();
			vel->Reset();
			effort->Reset();
			jn->Append(Concrete->name);
			pos->Append(Concrete->position);
			vel->Append(Concrete->velocity);
			effort->Append(Concrete->effort);

			/*pos->Add(Concrete->position[id*3]);
			pos->Add(Concrete->position[id*3+1]);
			pos->Add(Concrete->position[id*3+2]);
			pos->Add(Concrete->position[id*3+3]);
			pos->Add(Concrete->position[id*3+4]);
			pos->Add(Concrete->position[id*3+5]);*/
			
			return true;
		}
		return false;
	};
	// Subscriber callback to data collection signal
	Bool_SubscribeCallback = [take = &TakeData](TSharedPtr<FROSBaseMsg> msg) -> bool
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

	received_msg |= ROS2Sim_Joints_Topic->Subscribe(JointsSubscribeCallback);
	received_msg |= ROS2Sim_Bool_Topic->Subscribe(Bool_SubscribeCallback);

	JointPosMSG = MakeShareable(new ROSMessages::sensor_msgs::JointState());
	TSharedPtr<ROSMessages::geometry_msgs::Vector3> const RobotPos(new ROSMessages::geometry_msgs::Vector3(EEFJointPos));
	Sim2ROS_Vec3_0_WorldPos_Topic->Publish(RobotPos);
}

// Called every frame
void AROSActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TSharedPtr<ROSMessages::geometry_msgs::Vector3> const RobotPosMSG(new ROSMessages::geometry_msgs::Vector3(EEFJointPos));
	Sim2ROS_Vec3_0_WorldPos_Topic->Publish(RobotPosMSG); // Publish the robot eef world position every tick

	
	JointPosMSG->name = UE5JointName;
	JointPosMSG->position = UE5JointPosition;
	JointPosMSG->velocity = UE5JointPositionErr;
	JointPosMSG->effort = TopicPosition;
	Sim2ROS_Joints_Topic->Publish(JointPosMSG);
}

void AROSActor::SaveCamData(FString FilePath, TArray<FString> SavedData) {
	FString FinalString = "";
	for (FString& Each : SavedData) {
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}
	if (!FFileHelper::SaveStringToFile(FinalString, *FilePath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Failed to Save Cam Location"));
	}
}
/**
void AROSActor::ReadRTPixels(UTextureRenderTarget2D* SceneCaptureComp)
{
	FTextureRenderTargetResource* RenderTargetResource = SceneCaptureComp->GameThread_GetRenderTargetResource();
	bool bReadSuccess = RenderTargetResource->ReadPixels(ImageData);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("%d"), bReadSuccess);
}
**/