// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSArmControl.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "sensor_msgs/JointState.h"
#include "geometry_msgs/Transform.h"

void UROSArmControl::InitRobotArm(USkeletalMeshComponent* arm, FName JointProfileName, FName JointCommonBoneName, FString RobotTopicPrefix)
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

	RJointPosition.Init(10,0);
	if (!rosinst->ROSIntegrationCore) {return;}
	R2S_JointState_Topic = NewObject<UTopic>(UTopic::StaticClass());
	R2S_JointState_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/joint_states"), TEXT("sensor_msgs/JointState"));
	// ...
	JointState_SubscribeCallback = [_pos = &RJointPosition, _vel = &RJointVelocity, _name = &RJointNames](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::sensor_msgs::JointState>(msg);
		if (Concrete.IsValid())
		{
			R2S_TArray_Helper(Concrete->position, _pos);
			R2S_TArray_Helper(Concrete->name, _name);
		}
	};
	R2S_JointState_Topic->Subscribe(JointState_SubscribeCallback);

	S2R_RobotState_Topic = NewObject<UTopic>(UTopic::StaticClass());
	S2R_RobotState_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+RobotTopicPrefix+TEXT("/robot_state"), TEXT("tf2_msgs/TFMessage"));
	S2R_RobotState_Topic->Advertise();
	
	PlatformTransformsTopic = NewObject<UTopic>(UTopic::StaticClass());
	FString planar_topic_name = TEXT("/ue5/")+RobotTopicPrefix+TEXT("/planar_robot_tf");
	UE_LOG(LogTemp, Log, TEXT("%s"),*planar_topic_name)
	PlatformTransformsTopic->Init(rosinst->ROSIntegrationCore, planar_topic_name, TEXT("geometry_msgs/Transform"));
	PlatformRobot_SubscribeCallback = [_transform = &PlatformTransform, _enabled = &bFollowPlatformTopic](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::geometry_msgs::Transform>(msg);
		if (Concrete.IsValid())
		{
			*_enabled = true;
			_transform->SetLocation(
				FVector(
					Concrete->translation.x,
					Concrete->translation.y,
					Concrete->translation.z
				)
			);
			_transform->SetRotation(
				FQuat(
					Concrete->rotation.x,
					Concrete->rotation.y,
					Concrete->rotation.z,
					Concrete->rotation.w
				)
			);

			
		}
	};
	PlatformTransformsTopic->Subscribe(PlatformRobot_SubscribeCallback);
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
	robot_state_msg = MakeShareable(new ROSMessages::tf2_msgs::TFMessage());
	
}



// Called every frame
void UROSArmControl::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UROSArmControl::SetJointsTargets()
{
	#if WITH_EDITOR
		//UE_LOG(LogTemp, Log, TEXT("Set Joint Targets %d, %d, %d"),RobotJointMapping.Num(), RJointPosition.Num(), RJointNames.Num())
	#endif
	
	RobotArm->WakeAllRigidBodies();
	if (bFollowPlatformTopic)
	{
		//RobotArm->SetWorldTransform(PlatformTransform, false,nullptr,ETeleportType::TeleportPhysics);
		RobotArm->SetWorldLocationAndRotation(PlatformTransform.GetLocation(), PlatformTransform.GetRotation(),
						false,nullptr,ETeleportType::TeleportPhysics);
	}
	
	int _RJointPositionNum = RJointPosition.Num();
	if (_RJointPositionNum <= 0)
	{
		// joint position not received yet from ros
		return;
	}
	
	for (int ue5_ind = 0; ue5_ind < JointNames_ROS.Num(); ue5_ind++)
	{
		int ros_ind = RJointNames.Find(JointNames_ROS[ue5_ind]);
		//UE_LOG(LogTemp, Log, TEXT("%d number joints, %d"), RJointNames.Num(), JointNames_ROS.Num())
		// the joint name is found in the topic message
		if (ros_ind == INDEX_NONE)
		{
			continue;
		}
			// set the target
		for (int i = 0; i < RobotJoints.Num(); i++)
		{
			FString joint_name = RobotJoints[i].Get()->JointName.ToString();
			if (joint_name != JointNames_UE[ue5_ind])
			{
				continue;
			}
			// UE_LOG(LogTemp, Log, TEXT("%s, %s: %f"), *joint_name, *JointNames_ROS[ue5_ind], RJointPosition[ros_ind])
            // get the target joint position (angular or linear)
            if (bLinearJointType[ue5_ind])
            {
                FVector target = FVector(RJointPosition[ros_ind], RJointPosition[ros_ind],RJointPosition[ros_ind]);
                RobotJoints[i].Get()->SetLinearPositionTarget(target);
                break;
            }
            FQuat target = FQuat::MakeFromEuler(FVector(RJointPosition[ros_ind]*180.0f/PI,0,0));
            RobotJoints[i].Get()->SetAngularOrientationTarget(target);
            break;
		}
	}
}
void UROSArmControl::PubRobotState()
{
	ROSMessages::geometry_msgs::TransformStamped base_transform;
	FVector base_transforms_loc = FVector(0, 0, 0);
	FQuat base_transforms_rot = FQuat(0, 0, 0, 1);
	if (RobotArm->DoesSocketExist("camera_socket")) // check socket existence
	{
		RobotArm->GetSocketWorldLocationAndRotation("base_socket", base_transforms_loc, base_transforms_rot);
	}
	else
	{
		base_transforms_loc = RobotArm->GetComponentLocation();
		base_transforms_rot = RobotArm->GetComponentQuat();
	}
	FVector cam_transforms_loc = FVector(0, 0, 0);
	FQuat cam_transforms_rot = FQuat(0, 0, 0, 1);
	if (RobotArm->DoesSocketExist("camera_socket"))
	{
		RobotArm->GetSocketWorldLocationAndRotation("camera_socket", cam_transforms_loc, cam_transforms_rot);
	}
	else
	{
		cam_transforms_loc = RobotArm->GetComponentLocation();
		cam_transforms_rot = RobotArm->GetComponentQuat();
	}
	//robot_state_msg->transforms.Reset();
	robot_state_msg->transforms.Init(base_transform,2);
	robot_state_msg->transforms[0].header.frame_id = "base_link";
	robot_state_msg->transforms[1].header.frame_id = "camera_link";
	robot_state_msg->transforms[0].header.time = FROSTime::Now();
	robot_state_msg->transforms[1].header.time = robot_state_msg->transforms[0].header.time;
	robot_state_msg->transforms[0].transform.translation = base_transforms_loc;
	robot_state_msg->transforms[0].transform.rotation = base_transforms_rot;
	robot_state_msg->transforms[1].transform.translation = cam_transforms_loc;
	robot_state_msg->transforms[1].transform.rotation = cam_transforms_rot;
	if (!S2R_RobotState_Topic)
	{
		return;
	}
	S2R_RobotState_Topic->Publish(robot_state_msg);

}
