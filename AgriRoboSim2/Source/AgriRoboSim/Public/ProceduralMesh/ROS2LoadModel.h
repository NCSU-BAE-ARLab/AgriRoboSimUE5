// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicPlantMesh.h"
#include "ROS2NodeComponent.h"

#include "ROS2LoadModel.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGRIROBOSIM_API UROS2LoadModel : public UDynamicPlantMesh
{
	GENERATED_BODY()

public:	
	UROS2NodeComponent* ROS2Node;
	// Sets default values for this component's properties
	UROS2LoadModel();
	UPROPERTY()
	UTopic* ROS2Model_Topic;
	UFUNCTION(BlueprintCallable, Category = "ROS2LoadModel")
	void PublishState(FString ResponseMSG);
	UPROPERTY()
	int PacketID;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
