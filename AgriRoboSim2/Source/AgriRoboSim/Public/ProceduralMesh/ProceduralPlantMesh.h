// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralPlantMesh.generated.h"

struct FStreamingOBJTask
{
    FString OBJData;
    FString MTLData;
    bool bFlipNormal = false;

    TArray<FString> MTLMaterialName;
    TArray<FString> MTLMaterialDef;
    TArray<FString> OBJMaterialName;

    TArray<FString> OBJLines;
    TArray<FString> MTLLines;

    int32 CurrentLineIndex = 0;
    int32 CurrentMeshIndex = 0;

    TArray<FVector> TempVertices;
    TArray<FVector> TempNormals;
    TArray<FVector2D> TempUVs;

    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<int32> Triangles;

    bool bFinished = false;
    UProceduralMeshComponent* ProcMesh = nullptr;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGRIROBOSIM_API UProceduralPlantMesh : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralPlantMesh();

 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProceduralMeshPlayground")
	UProceduralMeshComponent * ProcMesh;

	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshPlayground")
 	void CreateMesh();
	UFUNCTION(BlueprintCallable, Category = "ProceduralMeshPlayground")
	void CreateMeshFromOBJString(
		const FString& OBJData, const FString& MTLData, const bool bFlipNormal,
		TArray<FString>& MTLMaterialName, TArray<FString>& MTLMaterialDef, TArray<FString>& OBJMaterialName);
    void LoadOBJAsync(const FString& OBJData, const FString& MTLData, bool bFlipNormal = false);
	void TickStreamingOBJ(int32 FacesPerTick = 20);
	FStreamingOBJTask* StreamingTask = nullptr;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
