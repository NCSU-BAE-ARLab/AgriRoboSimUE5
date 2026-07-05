// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMesh/ROS2LoadModel.h"
#include "std_msgs/String.h"

// Sets default values for this component's properties
UROS2LoadModel::UROS2LoadModel()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UROS2LoadModel::BeginPlay()
{
	Super::BeginPlay();
	ROS2Node = NewObject<UROS2NodeComponent>(this);
	ROS2Node->RegisterComponent();
	ROS2Node->Activate(true);
	PacketID = 0;
	// ...
	if (!ROS2Node->rosinst){return;}
	ROS2Model_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Model_Topic->Init(ROS2Node->rosinst->ROSIntegrationCore, TEXT("/ue5/LoadModel"), TEXT("std_msgs/String"));
	ROS2Model_Topic->Advertise();
}

void UROS2LoadModel::PublishState(FString ResponseMSG)
{
	if (ROS2Model_Topic)
	{
		TSharedPtr<ROSMessages::std_msgs::String> msg = MakeShareable(new ROSMessages::std_msgs::String());
		msg->_Data = ResponseMSG;
		ROS2Model_Topic->Publish(msg);
		PacketID += 1;
	}
}

// Called every frame
void UROS2LoadModel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

