// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TcpSocketConnection.h"
#include "CameraTcpSocketConnection.generated.h"

/**
 * 
 */
UCLASS()
class TEST2_API ACameraTcpSocketConnection : public ATcpSocketConnection
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void OnConnected(int32 connection);
	
	UFUNCTION()
	void OnDisconnected(int32 ConId);

	UFUNCTION()
	void OnMessageReceived(int32 ConId, TArray<uint8>& Message);
  
	UFUNCTION(BlueprintCallable)
	void ConnectToGameServer();
	UFUNCTION(BlueprintImplementableEvent)
	void ImplementableEvent();
	UFUNCTION(BlueprintNativeEvent)
	void NativeEvent();
	UPROPERTY()
	int32 ConnectionIdGameServer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IPAddr = "127.0.0.1";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Port = 1234;

	UPROPERTY(BlueprintReadOnly)
	TArray<uint8> MessageBytes;
	UPROPERTY(BlueprintReadOnly)
	FString MessageString;
};
