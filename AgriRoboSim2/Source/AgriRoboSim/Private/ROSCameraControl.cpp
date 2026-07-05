// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSCameraControl.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SceneCaptureComponent2D.h"

/**
 * All in one function for configuring this component
 * @param CameraTypePair SceneCapture and which information to render
 * @param CameraModel The CameraComponent with desired camera parameters (FOV, post process)
 * @param Width image width
 * @param Height image height
 */
void UROSCameraControl::InitROSTopics(
	TMap<USceneCaptureComponent2D*, ECaptureType> CameraTypePair,
	UCameraComponent* CameraModel, int Width, int Height)
{
	SceneCaptures.Empty();
	//if (!rosinst->ROSIntegrationCore) {return;}
	for (auto Element : CameraTypePair)
	{
		InitSceneCapture(Element.Key, Element.Value);
	}
	UpdateAllCameraParameters(CameraModel);
	UpdateAllCameraSize(Width, Height);
}

void UROSCameraControl::InitSceneCaptures()
{
	SceneCaptures.Empty();
	if (!rosinst->ROSIntegrationCore) {return;}
	TArray<USceneCaptureComponent2D*> L_SceneCaptures;
	this->GetOwner()->GetComponents<USceneCaptureComponent2D>(L_SceneCaptures);
	for (int i = 0; i < L_SceneCaptures.Num(); ++i)
	{
		//TSharedPtr<UROSSceneCapture> NewROSSceneCapture = MakeShared<UROSSceneCapture>();
		//NewROSSceneCapture->SceneCapture = L_SceneCaptures[i];
		//SceneCaptures.Add(NewROSSceneCapture);
	}
}

void UROSCameraControl::InitSceneCapture(USceneCaptureComponent2D* SceneCapture, ECaptureType CaptureType)
{
	
	FString topic_name = TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName();
	UE_LOG(LogTemp, Log, TEXT("%s"),*topic_name)
	UROSSceneCapture* NewROSSceneCapture = NewObject<UROSSceneCapture>(UROSSceneCapture::StaticClass());
	NewROSSceneCapture->Initialize(rosinst,topic_name, SceneCapture, CaptureType);
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
		//Color_Topic = NewObject<UTopic>(UTopic::StaticClass());
		//Color_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
		//Color_Topic->Advertise();
		//NewROSSceneCapture = UROSSceneCapture(rosinst, topic_name, SceneCapture, CaptureType);

		SceneCaptures.Add(NewROSSceneCapture);
		break;
	case ECaptureType::SegmentationCapture:
		//Segment_Topic = NewObject<UTopic>(UTopic::StaticClass());
		//Segment_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
		//Segment_Topic->Advertise();
		//NewROSSceneCapture = MakeShared<UROSSceneCapture>(rosinst, topic_name, SceneCapture, CaptureType);
		SceneCaptures.Add(NewROSSceneCapture);
		break;
	case ECaptureType::DepthCapture:
		//Depth_Topic = NewObject<UTopic>(UTopic::StaticClass());
        //Depth_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
        //Depth_Topic->Advertise();
		//NewROSSceneCapture = MakeShared<UROSSceneCapture>(rosinst, topic_name, SceneCapture, CaptureType);
		SceneCaptures.Add(NewROSSceneCapture);
		break;
	default:
		break;
	}
}

// Sets default values for this component's properties
UROSCameraControl::UROSCameraControl()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UROSCameraControl::BeginPlay()
{
	Super::BeginPlay();
	//InitROSTopics();
	// ...
}

// Called every frame
void UROSCameraControl::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//PublishAllTopic();
	// ...
}

UTextureRenderTarget2D* UROSCameraControl::CreateRenderTarget(
	int Width, int Height, ETextureRenderTargetFormat Format)
{
	UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>();
	check(NewRenderTarget2D);
	//NewRenderTarget2D->InitCustomFormat(Width,Height,GetPixelFormatFromRenderTargetFormat(Format),false);
	NewRenderTarget2D->RenderTargetFormat = Format;
	NewRenderTarget2D->ClearColor = FColor();
	NewRenderTarget2D->bAutoGenerateMips = false;
	NewRenderTarget2D->bCanCreateUAV = false;//true;
	NewRenderTarget2D->InitAutoFormat(Width, Height);
	NewRenderTarget2D->SRGB = true;
	NewRenderTarget2D->bGPUSharedFlag = true;
	NewRenderTarget2D->TargetGamma = 0.0;//2.2;
	NewRenderTarget2D->UpdateResourceImmediate(true);
	return NewRenderTarget2D; 
}
void UROSCameraControl::PublishSelectTopic(FName TopicName)
{
	
}
void UROSCameraControl::PublishAllTopic()
{
	for (auto SceneCapture : SceneCaptures)
	{
		if (SceneCapture->is_publishing) {return;}
		//UE_LOG(LogTemp, Log, TEXT("Publishing: %s, %s"), *SceneCapture->SceneCapture->GetName(),*SceneCapture->Topic->GetTopicName())
		
	}
	for (auto SceneCapture : SceneCaptures)
	{
		SceneCapture->Publish();
	}
}

void UROSCameraControl::UpdateAllCameraParameters(UCameraComponent* Camera)
{
	for (auto SceneCapture: SceneCaptures)
	{
		SceneCapture->UpdateSceneCaptureCameraParameters(Camera, GetWorld());
	}
}

void UROSCameraControl::SetCameraMode(EROSCameraMode Mode)
{
	for (auto SceneCapture: SceneCaptures)
	{
		switch (Mode)
		{
			case EROSCameraMode::Streaming:
				SceneCapture->is_streaming = true;
				break;
			case EROSCameraMode::Block:
				SceneCapture->is_streaming = false;
				break;
		}
	}
}

void UROSCameraControl::UpdateAllCameraSize(int Width, int Height)
{
	for (auto Element : SceneCaptures)
	{
		Element->SceneCapture->TextureTarget = CreateRenderTarget(Width, Height, Element->RenderTargetFormat);
		if (Element->SceneCapture->TextureTarget)
		{
			UE_LOG(LogTemp, Log, TEXT("Getting Texture Target Params"))
			Element->RefreshImageTopicSize();
		} else
		{
			UE_LOG(LogTemp, Log, TEXT("NO Texture Target"))
		}
		
	}
}

void UROSCameraControl::UpdateFrameIDSources(TArray<AActor*> FrameIDSources)
{
	for (auto Element : SceneCaptures)
	{
		if (Element && Element->SceneCapture && Element->SceneCapture->TextureTarget)
    	{
			Element->IDSources = FrameIDSources;
			//Element->UpdateFrameID();
			//UKismetRenderingLibrary::ClearRenderTarget2D(this, Element->SceneCapture->TextureTarget, FLinearColor::Transparent);

		}
	}
}



