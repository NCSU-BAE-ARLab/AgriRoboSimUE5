// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSArmControl.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "sensor_msgs/JointState.h"
void UROSArmControl::InitRobotArm(USkeletalMeshComponent* arm, FName JointProfileName, FName JointCommonBoneName)
{
	RobotArm = arm;
	RobotArm->SetConstraintProfileForAll(JointProfileName, false);
	
	// load which robot joints should be moved
	RobotJoints.Reset();
	RobotJointMapping.Reset();
	RobotArm->GetConstraints(false, RobotJoints);
	for (int i = 0; i < RobotJoints.Num(); i++)
	{
		FString joint_name = RobotJoints[i].Get()->JointName.ToString();
		if (joint_name.Contains(JointCommonBoneName.ToString()) && !joint_name.Contains("_end"))
		{
			UE_LOG(LogTemp, Log, TEXT("%s added to mapping at %d"), *joint_name, i)
			RobotJointMapping.Add(i);
		}
		if (joint_name.Contains("_end")){
			UE_LOG(LogTemp, Log, TEXT("%s added as end effector: %d"), *joint_name, i)
			EndEffectorJoint = i;
		}
	}
}

void UROSArmControl::Debug()
{
	for (auto Element : RobotJointMapping)
	{
		FString joint_name = RobotJoints[Element].Get()->JointName.ToString();
		UE_LOG(LogTemp, Log, TEXT("DEBUG: %s"), *joint_name)
	}
}

FVector UROSArmControl::GetEndEffectorTransform()
{
	if (RobotJoints.Num() < EndEffectorJoint) {return FVector();}
	return RobotArm->GetBoneLocation(RobotJoints[EndEffectorJoint].Get()->GetChildBoneName());
	//return Cast<USceneComponent>(RobotJoints[EndEffectorJoint].Get())->GetComponentTransform();
}

// Sets default values for this component's properties
UROSArmControl::UROSArmControl()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}


// Called when the game starts
void UROSArmControl::BeginPlay()
{
	Super::BeginPlay();
	RJointPosition.Init(10,0);
	
	R2S_JointState_Topic = NewObject<UTopic>(UTopic::StaticClass());
	R2S_JointState_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/joint_states"), TEXT("sensor_msgs/JointState"));
	// ...
	JointState_SubscribeCallback = [_pos = &RJointPosition, _vel = &RJointVelocity, _name = &RJointNames](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::sensor_msgs::JointState>(msg);
		if (Concrete.IsValid())
		{
			//UE_LOG(LogTemp, Log, TEXT("received joint message"));
			/*if (_pos->Num() < Concrete->position.Num())
			{
				_pos->Reserve(Concrete->position.Num());
			}
			for (int i = 0; i < Concrete->position.Num(); i++)
			{
				if (i < _pos->Num())
				{
					(*_pos)[i] = (Concrete->position)[i];
				} else
				{
					_pos->Add(Concrete->position[i]);
				}
			}
			if (_pos->Num() > Concrete->position.Num())
			{
				_pos->RemoveAt(Concrete->position.Num(), _pos->Num() - Concrete->position.Num());
			}*/
			//_vel->Reset();
			R2S_TArray_Helper(Concrete->position, _pos);
			R2S_TArray_Helper(Concrete->name, _name);
			//R2S_Helper(Concrete->position, _pos);
			//R2S_Helper(Concrete->name, _name);
			//_name->Reset();
			// _pos->Append(Concrete->position);
			//_vel->Append(Concrete->velocity);
			//_name->Append(Concrete->name);
		}
	};
	R2S_JointState_Topic->Subscribe(JointState_SubscribeCallback);

	
}



// Called every frame
void UROSArmControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UROSArmControl::SetJointsTargets()
{
	#if WITH_EDITOR
		//UE_LOG(LogTemp, Log, TEXT("%d, %d, %d"),RobotJointMapping.Num(), RJointPosition.Num(), RJointNames.Num())
	#endif
	int _RJointPositionNum = RJointPosition.Num();
	if (_RJointPositionNum <= 0)
	{
		// joint position not received yet from ros
		return;
	}
	int _RobotJointMappingNum = RobotJointMapping.Num();
	if (_RobotJointMappingNum > _RJointPositionNum ||
		_RobotJointMappingNum > RJointNames.Num()	||
		_RobotJointMappingNum != JointTopicOrder.Num()
		)
	{
		UE_LOG(LogTemp, Log, TEXT("Joint Count Mismatch, %d, %d, %d"),RobotJointMapping.Num(), RJointPosition.Num(), RJointNames.Num())
		return;
	}
	int maxMappingValue = 0;

	for (auto Element : JointTopicOrder)
	{
		if (Element > maxMappingValue)
		{
			maxMappingValue = Element;
		}
	}
	
	if (maxMappingValue >= _RJointPositionNum)
	{
		UE_LOG(LogTemp, Log, TEXT("Joint Mapping Out of Range, %d, %d"),maxMappingValue, RJointPosition.Num())
		return;
	}
	RobotArm->WakeAllRigidBodies();
	for (int i = 0; i < _RobotJointMappingNum; i++)
	{
		if (i >= JointTopicOrder.Num() || i >= RobotJointMapping.Num())
		{
			UE_LOG(LogTemp, Log, TEXT("i exceeds after checks"))
			return;
		}
		if (JointTopicOrder[i] >= RJointNames.Num() ||
			RobotJointMapping[i] >= RobotJoints.Num() ||
			JointTopicOrder[i] >= RJointPosition.Num())
		{
			UE_LOG(LogTemp, Log, TEXT("setjointtarget exceeds after checks"))
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("%s | %s : %f"),
			*RJointNames[JointTopicOrder[i]],
			*RobotJoints[RobotJointMapping[i]].Get()->JointName.ToString(),
			RJointPosition[JointTopicOrder[i]])
		FQuat target = FQuat::MakeFromEuler(FVector(RJointPosition[JointTopicOrder[i]]*180.0f/PI,0,0));
		RobotJoints[RobotJointMapping[i]].Get()->SetAngularOrientationTarget(target);
	}
}

