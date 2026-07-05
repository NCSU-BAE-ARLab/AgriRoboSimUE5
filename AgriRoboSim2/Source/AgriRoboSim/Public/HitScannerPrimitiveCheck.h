// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitScannerPrimitiveCheck.generated.h"

UENUM(BlueprintType)
enum class ETomatoHitScannerState : uint8
{
	Unknown = 0 	UMETA(DisplayName = "Unknown", Description = "Initial state"),
	NotInTrace = 1		UMETA(DisplayName = "NotInTrace", Description = "No tomato in trace"),
	ExistVisible = 2	UMETA(DisplayName = "Exist-Visible", Description = "Tomato exist and visible"),
	ExistObstructed = 3 UMETA(DisplayName = "Exist-Obstructed", Description = "Tomato exist but obstructed"),
	UnknownObstructed = 4 UMETA(DisplayName = "Unknown-Obstructed", Description = "Tomato unknown but will not be visible"),
	SimpleNoComplex = 5 UMETA(DisplayName = "SimpleNoComplex", Description = "Hit a simple object but no mesh hit (likely boundary)")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGRIROBOSIM_API UHitScannerPrimitiveCheck : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitScannerPrimitiveCheck();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FString HitPrimitiveName(UPrimitiveComponent* OtherComp, int32 HitCompID, FVector& Center, float& Radius);
	UFUNCTION(BlueprintCallable)
	void HitTomatoCheck(TArray<FHitResult> SimpleHitList, TArray<FHitResult> ComplexHitList, 
    	UTextureRenderTarget2D* HitTexture,
		ETomatoHitScannerState& State, int32&ComplexHitListID, int32&SimpleHitListID, FString& DebugString);
};
