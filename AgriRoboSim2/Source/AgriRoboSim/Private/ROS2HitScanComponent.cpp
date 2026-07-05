// Fill out your copyright notice in the Description page of Project Settings.


#include "ROS2HitScanComponent.h"
#include "geometry_msgs/Transform.h"

void UROS2HitScanComponent::InitHitScan(FString RobotTopicPrefix)
{
	if (!rosinst->ROSIntegrationCore) {return;}
	Check = false;
	R2S_Received_Topic = NewObject<UTopic>(UTopic::StaticClass());
	R2S_Received_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/RayDest"), TEXT("std_msgs/Float32MultiArray"));
	ReturnMessage = "";
	//UE_LOG(LogTemp, Log, TEXT("%s"), 
	// 	*(FString(TEXT("/ue5/")) + RobotTopicPrefix + TEXT("/RayDest"))
	// );
	// ...
	TargetLocation_SubscribeCallback = [_pos = &TargetLocations, _pix = &TargetImagePos, _check = &Check, _returnmsg = &ReturnMessage](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::Float32MultiArray>(msg);
		if (Concrete.IsValid())
		{
			*_check = true;
			_pos->Empty();
			_pix->Empty();
			*_returnmsg = "";
			size_t num_target = Concrete->layout.dim[0].size;
			size_t stride = Concrete->layout.dim[1].stride;
			//UE_LOG(LogTemp, Log, TEXT("num_target: %d stride: %d"), num_target, stride);
			for (size_t i = 0; i < num_target; i+=1)
			{
				_pos->Add(FVector(Concrete->data[i*stride], Concrete->data[i*stride+1], Concrete->data[i*stride+2]));
				//_pix->Add((FVector(Concrete->data[i*stride+3],Concrete->data[i*stride+4],0)));
			}
			
			//R2S_TArray_Helper(Concrete->position, _pos);
			//R2S_TArray_Helper(Concrete->name, _name);
		}
	};
	R2S_Received_Topic->Subscribe(TargetLocation_SubscribeCallback);

	S2R_Return_Topic = NewObject<UTopic>(UTopic::StaticClass());
	S2R_Return_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/RayInfo"), TEXT("std_msgs/Float32MultiArray"));
	S2R_Return_Topic->Advertise();
}

void UROS2HitScanComponent::PublishHitScan(const FString& data)
{
	if (!rosinst->ROSIntegrationCore) {return;}
	
	return_msg = MakeShareable(new ROSMessages::std_msgs::Float32MultiArray());
	ROSMessages::std_msgs::MultiArrayDimension dim1;
	TArray<FString> Tokens;
	data.ParseIntoArray(Tokens, TEXT(","), false);
	dim1.size = 3;
	dim1.stride = Tokens.Num()-1;
	
	if (dim1.stride / dim1.size != TargetLocations.Num()) {
		ReturnMessage = "";
		Check = true;
		return;
	}
	return_msg->layout.data_offset = 0;
	return_msg->layout.dim.Add(dim1);
	//UE_LOG(LogTemp, Log, TEXT("return size: %d stride: %d"), dim1.size, dim1.stride);
	for (int32 i = 0; i < Tokens.Num()-1; i++)
	{
		int32 StartIndex = 0;
		//UE_LOG(LogTemp, Log, TEXT("T: %s"), *Tokens[i]);
		// Find the index of the first digit
		for (int32 j = 0; j < Tokens[i].Len(); j++)
		{
			if (FChar::IsDigit(Tokens[i][j]))
			{
				StartIndex = j;
				break;
			}
		}

		// Convert from that index onwards
		float Value = FCString::Atof(*Tokens[i] + StartIndex);
		return_msg->data.Add(Value);
	}
	S2R_Return_Topic->Publish(return_msg);
}

// Called when the game starts
void UROS2HitScanComponent::BeginPlay()
{
	Super::BeginPlay();
	received_msg = MakeShareable(new ROSMessages::std_msgs::Float32MultiArray());
	return_msg = MakeShareable(new ROSMessages::std_msgs::Float32MultiArray());
}