// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ROS2NodeComponent.h"
#include "std_msgs/String.h"
#include "SceneCommandGameStateCompoenent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGRIROBOSIM_API USceneCommandGameStateCompoenent : public UROS2NodeComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USceneCommandGameStateCompoenent();
	UPROPERTY()
	UTopic* GameCommandTopic;
	TSharedPtr<ROSMessages::std_msgs::String> GameCommandMSG;
	std::function<void(TSharedPtr<FROSBaseMsg>)> GameCommand_SubscribeCallback;
	
	UFUNCTION(BlueprintCallable)
	TMap<FString, FString> ReadEntryPairsFromLastMSG();

    UPROPERTY(BlueprintReadWrite)
	FString game_command;

	UPROPERTY()
	UTopic* GameResponseTopic;
	TSharedPtr<ROSMessages::std_msgs::String> GameResponseMSG;
	
	UFUNCTION(BlueprintCallable)
	void SendGameResponse(FString& ResponseMSG);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
