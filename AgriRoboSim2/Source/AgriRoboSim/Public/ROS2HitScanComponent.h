// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "std_msgs/Float32MultiArray.h"
#include "std_msgs/UInt8MultiArray.h"
#include "ROS2HitScanComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGRIROBOSIM_API UROS2HitScanComponent : public UROS2NodeComponent
{
	GENERATED_BODY()
	public:
	UFUNCTION(BlueprintCallable)
	void InitHitScan(FString RobotTopicPrefix);
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> TargetLocations;
	TArray<FVector> TargetImagePos;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Check;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ReturnMessage;
	UFUNCTION(BlueprintCallable)
	void PublishHitScan(const FString& data);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	// for ROS
	UPROPERTY()
	UTopic* R2S_Received_Topic;
	UPROPERTY()
	UTopic* S2R_Return_Topic;
	TSharedPtr<ROSMessages::std_msgs::Float32MultiArray> received_msg; // n * 3 (x,y,z)
	TSharedPtr<ROSMessages::std_msgs::Float32MultiArray> return_msg; // n * 4 (fruit id, distance, u, v)
	std::function<void(TSharedPtr<FROSBaseMsg>)> TargetLocation_SubscribeCallback;
};
