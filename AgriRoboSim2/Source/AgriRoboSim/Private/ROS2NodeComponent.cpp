// Fill out your copyright notice in the Description page of Project Settings.


#include "ROS2NodeComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UROS2NodeComponent::UROS2NodeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UROS2NodeComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterGameInstance();
}


// Called every frame
void UROS2NodeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UROS2NodeComponent::RegisterGameInstance()
{
	rosinst = Cast<UROSIntegrationGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	/*for (auto Element : rosinst->ConnectedToROSBridge)
	{
		UE_LOG(LogTemp, Log, TEXT("%d"),Element)
	}*/
}

