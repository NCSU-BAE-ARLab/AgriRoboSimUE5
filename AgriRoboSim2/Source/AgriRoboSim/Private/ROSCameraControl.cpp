// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSCameraControl.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"

/**
 * Compute the size of the ROS topic message size
 */
void FROSSceneCapture::RefreshImageTopicSize()
{
	//ROSEncoding = SceneCapture->TextureTarget->GetFormat();
	ImageMSG = MakeShareable(new ROSMessages::sensor_msgs::Image());
	ImageMSG->encoding = CheckROSEncoding();
	img = std::make_shared<uint8[]>(
		SceneCapture->TextureTarget->SizeX *
		SceneCapture->TextureTarget->SizeY *
		ROSStepMultiplier);
	ImageMSG->data = img.get();
	//ImageMSG->header = ROSMessages::std_msgs::Header(0,FROSTime(0,0),"map");
	ImageMSG->width = SceneCapture->TextureTarget->SizeX;
	ImageMSG->height = SceneCapture->TextureTarget->SizeY;
	ImageMSG->is_bigendian = false;
	ImageMSG->step = SceneCapture->TextureTarget->SizeX * ROSStepMultiplier;
}

/**
 * read from current scene capture texture and publish it according to the datatype
 */
void FROSSceneCapture::Publish()
{
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		ReadRenderTargetPerRHI();
		Publish(&ImageData8Bit);
		break;
	case RTF_R16f:
	case RTF_RG16f:
	case RTF_RGBA16f:
		ReadRenderTargetPerRHI();
		Publish(&ImageData16Bit);
		break;
	case RTF_R32f:
	case RTF_RG32f:
	case RTF_RGBA32f:
		ReadRenderTargetPerRHI();
		Publish(&ImageData32Bit);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unimplemented Format"))
	}
}

/**
 * converts the ue5 TArray image to ROS image datatype then publish it over the topic
 * @tparam T datatype for the image
 * @param Image TArray holding the ue5 texture 
 */
template<typename T>
void FROSSceneCapture::Publish(TArray<T>* Image)
{
	if (UpdateImageMsg(Image, img.get()))
	{
		//UE_LOG(LogTemp, Log, TEXT("publishing, %d, %p, %d"), Image->Num(), img.get(), img.get()!=nullptr)
		Topic->Publish(ImageMSG);
		//UE_LOG(LogTemp, Log, TEXT("published"))
	}
}

/**
 * fill the ROS buffer with UE5 TArray information based on the TArray datatype
 * @tparam T datatype for the image
 * @param Image UE5 image in form of TArray
 * @param data ROS image buffer
 * @return whether the buffer was filled
 */
template <typename T>
bool FROSSceneCapture::UpdateImageMsg(TArray<T>* Image, uint8* data)
{
	if (Image->Num() < 100)
	{
		UE_LOG(LogTemp, Log, TEXT("no image msg"))
		return false;
	}
	CheckROSEncoding();
	if constexpr (std::is_same_v<T, FColor>)
	{
		for (int i = 0; i<Image->Num(); i++)
		{
			data[i*ROSStepMultiplier] = Image->GetData()[i].R;
			data[i*ROSStepMultiplier+1] = Image->GetData()[i].G;
			data[i*ROSStepMultiplier+2] = Image->GetData()[i].B;
			//data[i*4+3] = Image->GetData()[i].A;
		}
	} else if constexpr (std::is_same_v<T, FLinearColor>)
	{
		union FloatBytes
		{
			float floatVal;
			uint8_t bytes[4];
		};
		for (int i = 0; i<Image->Num(); i++)
		{
			FloatBytes floatBytes;
			floatBytes.floatVal = Image->GetData()[i].R;
			data[i*ROSStepMultiplier]   = floatBytes.bytes[0];
			data[i*ROSStepMultiplier+1] = floatBytes.bytes[1];
			data[i*ROSStepMultiplier+2] = floatBytes.bytes[2];
			data[i*ROSStepMultiplier+3] = floatBytes.bytes[3];
			
			//data[i*4+3] = Image->GetData()[i].A;
		}
	}else if constexpr (std::is_same_v<T, FFloat16Color>)
	{
		union Float16Bytes
		{
			uint16 floatVal;
			uint8_t bytes[2];
		};
		for (int i = 0; i<Image->Num(); i++)
		{
			Float16Bytes floatBytes;
            floatBytes.floatVal = Image->GetData()[i].R;
			data[i*ROSStepMultiplier] = floatBytes.bytes[0];
			data[i*ROSStepMultiplier] = floatBytes.bytes[1];
			//data[i*4+3] = Image->GetData()[i].A;
		}
	}
	return true;
}

/**
 * read render target texture on GPU and put it into CPU for publishing,
 * slightly higher FPS than alternatives
 */
void FROSSceneCapture::ReadRenderTargetPerRHI()
{
	auto RenderTarget = SceneCapture->TextureTarget;
	FTextureRenderTargetResource *RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[RenderTargetResource, this, RenderTarget]()
	{
		//UE_LOG(LogTemp, Log, TEXT("AsyncTask"))
		switch (RenderTargetResource->GetRenderTargetTexture()->GetDesc().Format)
		{
		case PF_B8G8R8A8:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData8Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadPixels(this->ImageData8Bit);
			break;
		case PF_FloatRGBA:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData16Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceFloatData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadFloat16Pixels(this->ImageData16Bit);
			break;
		case PF_A32B32G32R32F:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData32Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//RenderTargetResource->ReadLinearColorPixels(this->ImageData32Bit);
			//UE_LOG(LogTemp, Log, TEXT("RT READ: {%d}"), ImageData32Bit.Num())
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("NO RT READ: unclear pixel format"))
		}
	});
	
	//FlushRenderingCommands();
}

/**
 * update camera parameters using the Camera variable, and then filters for the actors that should be rendered
 * based on the capture type
 * @param Camera CameraComponent with desired parameters such as FOV, post process
 * @param WorldContext GetWorld()
 */
void FROSSceneCapture::UpdateSceneCaptureCameraParameters(
		UCameraComponent* Camera,
		UWorld* WorldContext
		)
{
	FMinimalViewInfo MinimalViewInfo;
	Camera->GetCameraView(0.0, MinimalViewInfo);
	SceneCapture->FOVAngle = MinimalViewInfo.FOV;
	SceneCapture->PostProcessSettings = MinimalViewInfo.PostProcessSettings;
	SceneCapture->PostProcessBlendWeight = MinimalViewInfo.PostProcessBlendWeight;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	TArray<AActor*> ShowOnlyActors_L;
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "ColoredImageGen", ShowOnlyActors_L);
		break;
	case ECaptureType::SegmentationCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "SegmentImageGen", ShowOnlyActors_L);
		break;
	case ECaptureType::DepthCapture:
		UGameplayStatics::GetAllActorsWithTag(WorldContext, "ColoredImageGen", ShowOnlyActors_L);
		break;
	default:
		break;
	}
	SceneCapture->ShowOnlyActors = ShowOnlyActors_L;
}

/**
 * 
 * @return the encoding string for the ROS Image Message
 */
FString FROSSceneCapture::CheckROSEncoding()
{
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		ROSStepMultiplier = 3;
		return "rgb8";
	case RTF_R16f:
	case RTF_RG16f:
	case RTF_RGBA16f:
		ROSStepMultiplier = 2;
        return "16SC1";
	case RTF_R32f:
	case RTF_RG32f:
	case RTF_RGBA32f:
		ROSStepMultiplier = 4;
		return "32FC1";
	default:
		ROSStepMultiplier = 1;
		return "undefined(UE5_Encoding)";
	}
}

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
	TArray<USceneCaptureComponent2D*> L_SceneCaptures;
	this->GetOwner()->GetComponents<USceneCaptureComponent2D>(L_SceneCaptures);
	for (int i = 0; i < L_SceneCaptures.Num(); ++i)
	{
		TSharedPtr<FROSSceneCapture> NewStruct = MakeShared<FROSSceneCapture>();
		NewStruct->SceneCapture = L_SceneCaptures[i];
		SceneCaptures.Add(NewStruct);
	}
}

void UROSCameraControl::InitSceneCapture(USceneCaptureComponent2D* SceneCapture, ECaptureType CaptureType)
{
	TSharedPtr<FROSSceneCapture> NewStruct;
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
		Color_Topic = NewObject<UTopic>(UTopic::StaticClass());
		Color_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
		Color_Topic->Advertise();
		NewStruct = MakeShared<FROSSceneCapture>(Color_Topic, SceneCapture, CaptureType);
		SceneCaptures.Add(NewStruct);
		break;
	case ECaptureType::SegmentationCapture:
		Segment_Topic = NewObject<UTopic>(UTopic::StaticClass());
		Segment_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
		Segment_Topic->Advertise();
		NewStruct = MakeShared<FROSSceneCapture>(Segment_Topic, SceneCapture, CaptureType);
		SceneCaptures.Add(NewStruct);
		break;
	case ECaptureType::DepthCapture:
		Depth_Topic = NewObject<UTopic>(UTopic::StaticClass());
        Depth_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
        Depth_Topic->Advertise();
		NewStruct = MakeShared<FROSSceneCapture>(Depth_Topic, SceneCapture, CaptureType);
		SceneCaptures.Add(NewStruct);
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
	NewRenderTarget2D->RenderTargetFormat = Format;
	NewRenderTarget2D->ClearColor = FLinearColor();
	NewRenderTarget2D->bAutoGenerateMips = false;
	NewRenderTarget2D->bCanCreateUAV = true;
	NewRenderTarget2D->InitAutoFormat(Width, Height);
	
	NewRenderTarget2D->SRGB = false;
	NewRenderTarget2D->bGPUSharedFlag = true;
	NewRenderTarget2D->TargetGamma = 2.5;
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



