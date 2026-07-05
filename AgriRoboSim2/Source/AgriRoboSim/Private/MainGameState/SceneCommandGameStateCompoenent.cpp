// Fill out your copyright notice in the Description page of Project Settings.


// Features to add

// 1. adjust PCG random seed
// 2. adjust PCG plant types
// 3. adjust robot parameters

#include "MainGameState/SceneCommandGameStateCompoenent.h"

#include "std_msgs/String.h"


// Sets default values for this component's properties
USceneCommandGameStateCompoenent::USceneCommandGameStateCompoenent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// messages will be as follows (for now), <c1_head>:<c1_data>:<c2_head>:<c2_data>:...
// commas reserved for data
TMap<FString, FString> USceneCommandGameStateCompoenent::ReadEntryPairsFromLastMSG()
{
	TMap<FString, FString> result;
	
	if (game_command.Len() > 1)
	{
		TArray<FString> cmds;
		//UE_LOG(LogTemp, Log, TEXT("%s"), *game_command);
		game_command.ParseIntoArray(cmds, TEXT(":"),false);
		GEngine->AddOnScreenDebugMessage(5,30.0f,FColor::Yellow,TEXT("Process Game Cmd"));
		if (cmds.Num() > 0 && cmds.Num() % 2 == 0)
		{
			for (int i = 0; i < cmds.Num(); i += 2)
			{
				result.Add(cmds[i],cmds[i + 1]);
			}
		}
		else
		{
		GEngine->AddOnScreenDebugMessage(
			6, 
			30.0f, 
			FColor::Red, 
			FString::Printf(TEXT("Incorrect Game Cmd Format: %d"), cmds.Num())
		);
			// unexpected command packet
		}
		game_command = FString("");
	}
	
	return result;
}
void USceneCommandGameStateCompoenent::SendGameResponse(FString& ResponseMSG)
{
	if (GameResponseTopic)
	{
		GameResponseMSG = MakeShareable(new ROSMessages::std_msgs::String());
		GameResponseMSG->_Data = ResponseMSG;
		GameResponseTopic->Publish(GameResponseMSG);
	}
}
// Called when the game starts
void USceneCommandGameStateCompoenent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	game_command = FString("");
	if (!rosinst->ROSIntegrationCore){return;}
	GameCommandTopic = NewObject<UTopic>(UTopic::StaticClass());
	GameCommandTopic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/game_commands"), TEXT("std_msgs/String"));	

	GameCommand_SubscribeCallback = [this](TSharedPtr<FROSBaseMsg> msg) -> void
	{
		auto Concrete = StaticCastSharedPtr<ROSMessages::std_msgs::String>(msg);
		if (Concrete.IsValid())
		{
			this->game_command = Concrete->_Data;
		}
	};
	GameCommandTopic->Subscribe(GameCommand_SubscribeCallback);

	GameResponseTopic = NewObject<UTopic>(UTopic::StaticClass());
	GameResponseTopic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/game_response"), TEXT("std_msgs/String"));
	GameResponseTopic->Advertise();
}


// Called every frame
void USceneCommandGameStateCompoenent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

