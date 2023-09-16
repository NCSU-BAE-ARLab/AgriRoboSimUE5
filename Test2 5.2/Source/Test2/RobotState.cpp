// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotState.h"

// Sets default values for this component's properties
URobotState::URobotState()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	
}


// Called when the game starts
void URobotState::BeginPlay()
{
	Super::BeginPlay();

	// ...
	GetOwner()->GetComponents(allJoints, true);
}


// Called every frame
void URobotState::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(GEngine)
	{
		AROSActor* parent = (AROSActor*)this->GetOwner();
		
		auto name = allJoints[0]->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::SanitizeFloat(parent->ArmID));
	}
	// ...
}

