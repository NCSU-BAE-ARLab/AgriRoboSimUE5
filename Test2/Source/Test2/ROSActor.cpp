// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSActor.h"
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ROS Actor Created!"));
	UE_LOG(LogTemp, Log, TEXT("ROS Actor Created!"));
	UROSIntegrationGameInstance* rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());

	// Initialize a topic
	UTopic* Sim2ROS_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
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

	UTopic* ROS2Sim_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Str_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Str_Topic_STR, TEXT("std_msgs/String"));
	ROS2Sim_Str_Topic->Advertise();

	//UTopic* ROS2Sim_Float_Topic = NewObject<UTopic>(UTopic::StaticClass());
	//ROS2Sim_Float_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Float_Topic_STR, TEXT("std_msgs/Float32"));
	//ROS2Sim_Float_Topic->Advertise();
	auto Vec3_1_Topic_Name = ROS2Sim_Vec3_0_1_Topic_STR;
	auto Vec3_2_Topic_Name = ROS2Sim_Vec3_0_2_Topic_STR;
	auto Vec3_WorldPos_Topic_Name = Sim2ROS_Vec3_0_WorldPos_Topic_STR;
	auto Bool_Topic_Name = ROS2Sim_Bool_0_Topic_STR;
	
	if (ArmID == 1) {
		Vec3_1_Topic_Name = ROS2Sim_Vec3_1_1_Topic_STR;
		Vec3_2_Topic_Name = ROS2Sim_Vec3_1_2_Topic_STR;
		Vec3_WorldPos_Topic_Name = Sim2ROS_Vec3_1_WorldPos_Topic_STR;
		Bool_Topic_Name = ROS2Sim_Bool_1_Topic_STR;
	}
	UTopic* ROS2Sim_Vec3_1_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_1_Topic->Init(rosinst->ROSIntegrationCore, Vec3_1_Topic_Name, TEXT("geometry_msgs/Vector3"));
	ROS2Sim_Vec3_1_Topic->Advertise();

	UTopic* ROS2Sim_Vec3_2_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_2_Topic->Init(rosinst->ROSIntegrationCore, Vec3_2_Topic_Name, TEXT("geometry_msgs/Vector3"));
	ROS2Sim_Vec3_2_Topic->Advertise();

	UTopic* Sim2ROS_Vec3_0_WorldPos_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Vec3_0_WorldPos_Topic->Init(rosinst->ROSIntegrationCore, Vec3_WorldPos_Topic_Name, TEXT("geometry_msgs/Vector3"));
	Sim2ROS_Vec3_0_WorldPos_Topic->Advertise();

	UTopic* ROS2Sim_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Bool_Topic->Init(rosinst->ROSIntegrationCore, Bool_Topic_Name, TEXT("std_msgs/Bool"));
	ROS2Sim_Bool_Topic->Advertise();
}

// Called every frame
void AROSActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UROSIntegrationGameInstance* rosinst = Cast<UROSIntegrationGameInstance>(GetGameInstance());

	UTopic* Sim2ROS_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Str_Topic->Init(rosinst->ROSIntegrationCore, Sim2ROS_Str_Topic_STR, TEXT("std_msgs/String"));

	UTopic* ROS2Sim_Str_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Str_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Str_Topic_STR, TEXT("std_msgs/String"));

	//UTopic* ROS2Sim_Float_Topic = NewObject<UTopic>(UTopic::StaticClass());
	//ROS2Sim_Float_Topic->Init(rosinst->ROSIntegrationCore, ROS2Sim_Float_Topic_STR, TEXT("std_msgs/Float32"));
	auto Vec3_1_Topic_Name = ROS2Sim_Vec3_0_1_Topic_STR;
	auto Vec3_2_Topic_Name = ROS2Sim_Vec3_0_2_Topic_STR;
	auto Vec3_WorldPos_Topic_Name = Sim2ROS_Vec3_0_WorldPos_Topic_STR;
	auto Bool_Topic_Name = ROS2Sim_Bool_0_Topic_STR;

	if (ArmID == 1) {
		Vec3_1_Topic_Name = ROS2Sim_Vec3_1_1_Topic_STR;
		Vec3_2_Topic_Name = ROS2Sim_Vec3_1_2_Topic_STR;
		Vec3_WorldPos_Topic_Name = Sim2ROS_Vec3_1_WorldPos_Topic_STR;
		Bool_Topic_Name = ROS2Sim_Bool_1_Topic_STR;

	}
	// subscribers to the robot joint angles
	UTopic* ROS2Sim_Vec3_1_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_1_Topic->Init(rosinst->ROSIntegrationCore, Vec3_1_Topic_Name, TEXT("geometry_msgs/Vector3"));
	UTopic* ROS2Sim_Vec3_2_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Vec3_2_Topic->Init(rosinst->ROSIntegrationCore, Vec3_2_Topic_Name, TEXT("geometry_msgs/Vector3"));

	// subscribe to data collection signal
	UTopic* ROS2Sim_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Bool_Topic->Init(rosinst->ROSIntegrationCore, Bool_Topic_Name, TEXT("std_msgs/Bool"));

	// publish robot position
	UTopic* Sim2ROS_Vec3_0_WorldPos_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Vec3_0_WorldPos_Topic->Init(rosinst->ROSIntegrationCore, Vec3_WorldPos_Topic_Name, TEXT("geometry_msgs/Vector3"));
	TSharedPtr<ROSMessages::geometry_msgs::Vector3> RobotPos(new ROSMessages::geometry_msgs::Vector3(eefJointPos));
	Sim2ROS_Vec3_0_WorldPos_Topic->Publish(RobotPos);

	// Create a std::function callback object
	std::function<void(TSharedPtr<FROSBaseMsg>)> STRSubscribeCallback = [Sim2ROS_Str_Topic, rm = &received_msg](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::String>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("Incoming string was: %s"), (*(Concrete->_Data)));
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Received a String from ROS"));

			TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim"));
			Sim2ROS_Str_Topic->Publish(StringMessage);
			*rm = true;
		}
		*rm = false;
		return *rm;
	};
	// Subscribe callback to joint angles
	// first 3 angles
	std::function<void(TSharedPtr<FROSBaseMsg>)> Vec3_1SubscribeCallback = [Sim2ROS_Str_Topic, fs = &firstSet](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::geometry_msgs::Vector3>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("Incoming x was: %f"), ((Concrete->x)));
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Received a Vector from ROS"));

			//TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim (first set)"));
			//Sim2ROS_Str_Topic->Publish(StringMessage);
			(fs->X) = Concrete->x;
			(fs->Y) = Concrete->y;
			(fs->Z) = Concrete->z;
			return true;
		}
		return false;
	};
	// second 3 angles
	std::function<void(TSharedPtr<FROSBaseMsg>)> Vec3_2SubscribeCallback = [Sim2ROS_Str_Topic, ss = &secondSet](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::geometry_msgs::Vector3>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("Incoming x was: %f"), ((Concrete->x)));
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Received a Vector from ROS"));

			//TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim (second set)"));
			//Sim2ROS_Str_Topic->Publish(StringMessage);
			(ss->X) = Concrete->x;
			(ss->Y) = Concrete->y;
			(ss->Z) = Concrete->z;
			return true;
		}
		return false;
	};
	// Subscriber callback to data collection signal
	std::function<void(TSharedPtr<FROSBaseMsg>)> Bool_SubscribeCallback = [Sim2ROS_Str_Topic, take = &takeData](TSharedPtr<FROSBaseMsg> msg) -> bool
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("Incoming data was: %f"), ((Concrete->_Data)));
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Received a Bool from ROS"));

			TSharedPtr<ROSMessages::std_msgs::String> StringMessage(new ROSMessages::std_msgs::String("Sent from Sim (bool)"));
			Sim2ROS_Str_Topic->Publish(StringMessage);
			*take = Concrete->_Data;
			return true;
		}
		return false;
	};
	// Subscribe to the topic
	if (!received_msg) {
		received_msg |= ROS2Sim_Str_Topic->Subscribe(STRSubscribeCallback);
		received_msg |= ROS2Sim_Vec3_1_Topic->Subscribe(Vec3_1SubscribeCallback);
		received_msg |= ROS2Sim_Vec3_2_Topic->Subscribe(Vec3_2SubscribeCallback);
		received_msg |= ROS2Sim_Bool_Topic->Subscribe(Bool_SubscribeCallback);
		//received_msg |= ROS2Sim_Float_Topic->Subscribe(FloatSubscriberCallback);
		//UE_LOG(LogTemp, Log, TEXT("5 Seconds Timer"));
	}
	time_accumulation += DeltaTime;
	if (time_accumulation > reset_time) {
		received_msg = false;
		time_accumulation = 0;
		//set_arm_pos += 10;
		UE_LOG(LogTemp, Log, TEXT("Parent Tick"));
		//UE_LOG(LogTemp, Log, TEXT("Angle2: %f"), (set_arm_pos));
	}
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