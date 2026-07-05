// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/DefaultValueHelper.h"
#include "IndexTypes.h"
#include "DynamicMesh/MeshNormals.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicPlantMesh.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGRIROBOSIM_API UDynamicPlantMesh : public UActorComponent
{
    GENERATED_BODY()

public:	
    // Sets default values for this component's properties
    UDynamicPlantMesh();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    UDynamicMeshComponent* DynMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    UDynamicMeshComponent* DynMeshSeg;

    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    void CreateMesh();
    
    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    void CreateMeshFromOBJString(
        const FString& OBJData, const FString& MTLData, const bool bFlipNormal);
    
    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    bool AsyncConstructMesh(int NumOfLines);
    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    void FinalizeMesh();
    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    void ConfigureMeshObj(const FString& OBJData, const FString& MTLData);
    UFUNCTION(BlueprintCallable, Category = "DynamicPlantMesh")
    float CurrentProgress();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    bool CompletedLoad;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    TArray<FString> MTLMaterialDef;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    TArray<FString> MTLMaterialName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    TArray<FString> OBJMaterialName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicPlantMesh")
    int AsyncLoadLine;
protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

    TArray<FVector3d> Vertices;
    TArray<FVector3d> Normals;
    TArray<FVector2f> UVs;
    FDynamicMesh3 Mesh;

    TArray<FString> MTLLines;
    TArray<FString> OBJLines;
    
    int32 CurrentMaterialSection = -1;
};
