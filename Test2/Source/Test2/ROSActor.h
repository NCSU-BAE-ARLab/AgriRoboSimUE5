// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "ROSActor.generated.h"

#define ROS2Sim_Str_Topic_STR TEXT("test_str_in")
#define Sim2ROS_Str_Topic_STR TEXT("test_str_out")
#define ROS2Sim_Float_Topic_STR TEXT("test_float")
#define ROS2Sim_Vec3_0_1_Topic_STR TEXT("/unreal/0/vec3_1")
#define ROS2Sim_Vec3_0_2_Topic_STR TEXT("/unreal/0/vec3_2")
#define ROS2Sim_Vec3_1_1_Topic_STR TEXT("/unreal/1/vec3_1")
#define ROS2Sim_Vec3_1_2_Topic_STR TEXT("/unreal/1/vec3_2")

#define Sim2ROS_Vec3_0_WorldPos_Topic_STR TEXT("/unreal/0/reachedgoal")
#define Sim2ROS_Vec3_1_WorldPos_Topic_STR TEXT("/unreal/1/reachedgoal")

#define ROS2Sim_Bool_0_Topic_STR TEXT("/unreal/0/takedata")
#define ROS2Sim_Bool_1_Topic_STR TEXT("/unreal/1/takedata")


UCLASS()

class TEST2_API AROSActor : public AActor
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

	float time_accumulation = 0;
	float reset_time = 0.1f;

	UPROPERTY(BlueprintReadWrite)
		bool received_msg = false;
	//UPROPERTY(BlueprintReadWrite)
	//	float set_arm_pos = 0.0;
	//UPROPERTY(BlueprintReadWrite)
	//	float set_wrist_pos = 0.0;
	UPROPERTY(BlueprintReadWrite)
		FVector firstSet;
	UPROPERTY(BlueprintReadWrite)
		FVector secondSet;
	UPROPERTY(BlueprintReadWrite)
		int32 ArmID;
	UPROPERTY(BlueprintReadWrite)
		FVector eefJointPos;
	UPROPERTY(BlueprintReadWrite)
		bool takeData;
};
