// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScannerPrimitiveCheck.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UHitScannerPrimitiveCheck::UHitScannerPrimitiveCheck()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHitScannerPrimitiveCheck::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHitScannerPrimitiveCheck::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
/**
 * HitPrimitiveName - BreakHitResult
 *      needed because blueprint does not include name of hit component (assigned in primitive collision)
 * @param OtherComp - The other primitive component that was hit
 * @param HitCompID - The index of the hit component
 * 
 * @returns The name of the hit component
 * @param Center - The center of the hit component
 * @param Radius - The radius of the hit component
 */
FString UHitScannerPrimitiveCheck::HitPrimitiveName(UPrimitiveComponent* OtherComp, int32 HitCompID, FVector& Center, float& Radius)
{
    FString Name = "none";
    Center = FVector::ZeroVector;
    Radius = 0.f;

    if (!OtherComp || HitCompID < 0)
    {
        return Name;
    }

    if (UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(OtherComp))
    {
        UStaticMesh* Mesh = MeshComp->GetStaticMesh();
        if (!Mesh || !Mesh->GetBodySetup())
        {
            return Name;
        }

        FKShapeElem* El = Mesh->GetBodySetup()->AggGeom.GetElement(HitCompID);
        if (!El)
        {
            return Name;
        }

        if (FKSphereElem* SphereEl = static_cast<FKSphereElem*>(El))
		{
            Center = SphereEl->Center;
            Radius = SphereEl->Radius;
        }

        Name = El->GetName().ToString();
    }

    return Name;
}
/**
 * HitTomatoCheck
 *      checks if tomato is hit and returns state
 * @param SimpleHitList - List of simple hit results
 * @param ComplexHitList - List of complex hit results 
 * 
 * @returns
 * @param State - State of tomato hit scanner
 * @param ComplexHitListID - ID of complex hit result
 */
void UHitScannerPrimitiveCheck::HitTomatoCheck(
    TArray<FHitResult> SimpleHitList, TArray<FHitResult> ComplexHitList,
    UTextureRenderTarget2D* HitTexture,
    ETomatoHitScannerState& State, int32&ComplexHitListID, int32&SimpleHitListID,
    FString& DebugString)
{
    State = ETomatoHitScannerState::Unknown;
    FLinearColor LeafColor = FLinearColor(0, 0, 0, 0);
    UMaterialInterface* LastHitMaterial = nullptr;
    AActor* LastHitActor = nullptr;
    DebugString = "NotInTrace,0.0";
    FString TomatoPrimName = "";
    float TomatoPrimRadius = 0.f;
    //TArray<float> distances = {};
    if (SimpleHitList.Num() == 0) {
        State = ETomatoHitScannerState::NotInTrace;
        DebugString = "NotInTrace,0.0";
        return;
    }
    for(int complex_idx = 0; complex_idx < ComplexHitList.Num(); complex_idx++)
    {
        if (State == ETomatoHitScannerState::NotInTrace ||
            State == ETomatoHitScannerState::ExistObstructed ||
            State == ETomatoHitScannerState::ExistVisible)
        {
            break;
        }
        FHitResult ComplexHit = ComplexHitList[complex_idx];
        int32 section_idx = 0;
        UMaterialInterface* TempHitMaterial = ComplexHit.Component.Get()->GetMaterialFromCollisionFaceIndex(ComplexHit.FaceIndex, section_idx);
        if (TempHitMaterial != LastHitMaterial) {
            LastHitMaterial = TempHitMaterial;
            LastHitActor = ComplexHit.Component.Get()->GetOwner();
            UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), HitTexture);
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(
                GetWorld(),
                HitTexture,
                LastHitMaterial
            );
        }
        FVector2D UV;
        UGameplayStatics::FindCollisionUV(ComplexHit, 0, UV);
        // get color in the texture TODO: optimize it
        if (!LastHitMaterial)
        {
            continue;
        }
        //UE_LOG(LogTemp, Warning, TEXT("LeafColor: %s"), *LastHitMaterial->GetName());
        bool hitLeaf = LastHitMaterial->GetName().Contains("leaf");
        if (hitLeaf)
        {
            LeafColor = UKismetRenderingLibrary::ReadRenderTargetRawUV(
                GetWorld(),
                HitTexture,
                UV.X,
                UV.Y,
                true
            );
            if (LeafColor.A > 50.f && State == ETomatoHitScannerState::Unknown) {
                //UE_LOG(LogTemp, Warning, TEXT("Transparent Leaf, check next, LeafColor: %s State: %d"), *LeafColor.ToString(), (int32)State);
                State = ETomatoHitScannerState::Unknown;
            } else {
                //UE_LOG(LogTemp, Warning, TEXT("Blocked by leaf, check next, LeafColor: %s State: %d"), *LeafColor.ToString(), (int32)State);
                State = ETomatoHitScannerState::UnknownObstructed;
            }
            continue;
        }
        // scan along hit tomato primitives
        TMap<FString, FVector> TomatoNameRadiusMinDistance;
        FHitResult SimpleHit;
        for(int simple_idx = 0; simple_idx < SimpleHitList.Num(); simple_idx++) {
            if (simple_idx != 0 &&SimpleHit.ElementIndex == SimpleHitList[simple_idx].ElementIndex) {
                continue;
            }
            SimpleHit = SimpleHitList[simple_idx];
            float radius = 0;
            FVector center = FVector::ZeroVector;
            FString Name = HitPrimitiveName(SimpleHit.Component.Get(), SimpleHit.ElementIndex, center, radius);
            FVector TomatoCenter = UKismetMathLibrary::Quat_RotateVector(
                SimpleHit.GetActor()->GetActorQuat(),
                center
            );
            FVector ScaledCenter;
            ScaledCenter = TomatoCenter * SimpleHit.GetActor()->GetActorScale();
            FVector TomatoCenterWorldLocation;
            TomatoCenterWorldLocation = SimpleHit.GetActor()->GetActorLocation() + ScaledCenter;
            // UE_LOG(LogTemp, Warning, TEXT("TomatoCenter: %s ScaledCenter: %s"),
            //     *TomatoCenter.ToString(), *ScaledCenter.ToString());
            // UE_LOG(LogTemp, Warning, TEXT("TomatoCenterWorldLocation: %s"),
            //     *TomatoCenterWorldLocation.ToString());
            float TomatoRadius = radius * SimpleHit.Component->GetComponentScale().GetMax();    // get the tomato primitive radius
            float distance = UKismetMathLibrary::Vector_Distance(ComplexHit.ImpactPoint, TomatoCenterWorldLocation);
            if (distance < TomatoRadius) {
                if (TomatoNameRadiusMinDistance.Contains(Name)) {
                    if (TomatoNameRadiusMinDistance[Name].Y > distance) {
                        TomatoNameRadiusMinDistance[Name] = FVector(TomatoRadius, distance, distance/TomatoRadius);
                    }
                }
                else 
                {
                    TomatoNameRadiusMinDistance.Add(Name, FVector(TomatoRadius, distance, distance/TomatoRadius));
                }
            }
        }

        float min_ratio = 1.f;
        for (const auto& Element : TomatoNameRadiusMinDistance)
        {
            if (Element.Value.Z < min_ratio) {
                min_ratio = Element.Value.Z;
                TomatoPrimName = Element.Key;
                TomatoPrimRadius = Element.Value.X;
            }
            //UE_LOG(LogTemp, Warning, TEXT("Element: %s, Radius: %f, Distance: %f, Ratio: %f"),
            //    *Element.Key, Element.Value.X, Element.Value.Y, Element.Value.Z);
        }
        if (State == ETomatoHitScannerState::Unknown && TomatoPrimName.Contains("t")) {
            State = ETomatoHitScannerState::ExistVisible;
            //UE_LOG(LogTemp, Warning, TEXT("Visible TomatoName: %s, State: %d"), *TomatoPrimName, (int32)State);
            break;
        } else if (State == ETomatoHitScannerState::UnknownObstructed && TomatoPrimName.Contains("t")) {
            State = ETomatoHitScannerState::ExistObstructed;
            //UE_LOG(LogTemp, Warning, TEXT("Obstructed TomatoName: %s, State: %d"), *TomatoPrimName, (int32)State);
            break;
        }
    }

    if (SimpleHitList.Num() > 0 && (State == ETomatoHitScannerState::Unknown || State == ETomatoHitScannerState::UnknownObstructed))
    {
        State = ETomatoHitScannerState::SimpleNoComplex;
        State = ETomatoHitScannerState::NotInTrace;
        for(int j = 0; j < SimpleHitList.Num(); j++) 
        {
            FHitResult SimpleHit = SimpleHitList[j];
            float radius = 0;
            FVector center = FVector::ZeroVector;
            FString Name = HitPrimitiveName(SimpleHit.Component.Get(), SimpleHit.ElementIndex, center, radius);
            TomatoPrimName = Name;
            TomatoPrimRadius = radius * SimpleHit.Component->GetComponentScale().GetMax();
        }
    }

    if (State == ETomatoHitScannerState::Unknown || State == ETomatoHitScannerState::UnknownObstructed)
    {
        //UE_LOG(LogTemp, Warning, TEXT("NotInTrace, terminate."));
        State = ETomatoHitScannerState::NotInTrace;
    }
    if (State != ETomatoHitScannerState::NotInTrace){
        DebugString = TomatoPrimName + "," + FString::SanitizeFloat(TomatoPrimRadius,2);
    } else {
        DebugString = "NotInTrace,0.0";
    }
}