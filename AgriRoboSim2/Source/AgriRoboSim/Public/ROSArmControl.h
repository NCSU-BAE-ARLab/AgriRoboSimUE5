// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "ROSArmControl.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGRIROBOSIM_API UROSArmControl : public UROS2NodeComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int> JointTopicOrder;	// ur10 is [0, 4, 1, 2, 3, 5]

	UFUNCTION(BlueprintCallable)
	void InitRobotArm(USkeletalMeshComponent* arm, FName JointProfileName, FName JointCommonBoneName);
	UFUNCTION(BlueprintCallable)
	void Debug();
	UFUNCTION(BlueprintCallable)
	FVector GetEndEffectorTransform();
	// Sets default values for this component's properties
	UROSArmControl();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
private:
	// for ROS
	UPROPERTY()
	UTopic* R2S_JointState_Topic;
	UPROPERTY()//BlueprintReadWrite, VisibleAnywhere)
	TArray<double> RJointPosition;
	UPROPERTY()//BlueprintReadWrite, VisibleAnywhere)
	TArray<double> RJointVelocity;
	UPROPERTY()//BlueprintReadWrite, VisibleAnywhere)
	TArray<double> RJointEffort;
	UPROPERTY()//BlueprintReadWrite, VisibleAnywhere)
	TArray<FString> RJointNames;
	std::function<void(TSharedPtr<FROSBaseMsg>)> JointState_SubscribeCallback;
	template<typename T>
	static void R2S_Helper(TArray<T> ROS, TArray<T>* Sim);
	// for UE5
	UPROPERTY()
	USkeletalMeshComponent* RobotArm;
	UPROPERTY()
	TArray<FConstraintInstanceAccessor> RobotJoints;
	UPROPERTY()
	int EndEffectorJoint;
	UPROPERTY()
	TArray<int8> RobotJointMapping;
	UFUNCTION(BlueprintCallable)
	void SetJointsTargets();
};

template <typename T>
void UROSArmControl::R2S_Helper(TArray<T> ROS, TArray<T>* Sim)
{
	if (Sim->Num() < ROS.Num())
	{
		Sim->Reserve(ROS.Num());
	}
	for (int i = 0; i < ROS.Num(); i++)
	{
		if (i < Sim->Num())
		{
			(*Sim)[i] = ROS[i];
		} else
		{
			Sim->Add(ROS[i]);
		}
	}
	if (Sim->Num() > ROS.Num())
	{
		Sim->RemoveAt(ROS.Num(), Sim->Num() - ROS.Num());
	}
			
}
