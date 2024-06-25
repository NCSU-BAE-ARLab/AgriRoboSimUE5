// Fill out your copyright notice in the Description page of Project Settings.


#include "ROS2NodeComponent.h"

#include "Kismet/GameplayStatics.h"
#include "std_msgs/Bool.h"

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

	/*if(!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}
	WebSocket = FWebSocketsModule::Get().CreateWebSocket("ws://127.0.0.1:9090/", TEXT("ws"));
	WebSocket->Connect();*/
	
	rosinst = Cast<UROSIntegrationGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	for (auto Element : rosinst->ConnectedToROSBridge)
	{
		UE_LOG(LogTemp, Log, TEXT("%d"),Element)
	}
	
	Sim2ROS_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	Sim2ROS_Bool_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/s2r"), TEXT("std_msgs/Bool"));
	Sim2ROS_Bool_Topic->Advertise();
	
	ROS2Sim_Bool_Topic = NewObject<UTopic>(UTopic::StaticClass());
	ROS2Sim_Bool_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/r2s"), TEXT("std_msgs/Bool"));

	// ...
	Bool_SubscribeCallback = [s2r_bool = Sim2ROS_Bool_Topic](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::Bool>(msg);
		if (Concrete.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("received message"));
			TSharedPtr<ROSMessages::std_msgs::Bool> BoolMessage(new ROSMessages::std_msgs::Bool(true));
			s2r_bool->Publish(BoolMessage);
		}
		return;
	};
	ROS2Sim_Bool_Topic->Subscribe(Bool_SubscribeCallback);
	
}


// Called every frame
void UROS2NodeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//UE_LOG(LogTemp, Log, TEXT("%d"), WebSocket->IsConnected());
	// ...
	//WebSocket->Send(TEXT("1"));
}

