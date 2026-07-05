// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSSceneCapture.h"

#include "AssetDefinitionAssetInfo.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtils.h"
#include "SNegativeActionButton.h"

// Sets default values for this component's properties
UROSSceneCapture::UROSSceneCapture()
{
	is_streaming = true;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// // Called when the game starts
// void UROSSceneCapture::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	// ...
// 	
// }
//
//
// // Called every frame
// void UROSSceneCapture::TickComponent(float DeltaTime, ELevelTick TickType,
//                                      FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
// 	UpdateFrameID();
// 	// ...
// }

void UROSSceneCapture::Initialize(
	UROSIntegrationGameInstance* rosinst,
	FString& topic_name_,
	USceneCaptureComponent2D* SceneCapture_,
	ECaptureType CaptureType_
)
{
	topic_name = topic_name_;
	UE_LOG(LogTemp, Log, TEXT("A %s"),*topic_name)
	SceneCapture = SceneCapture_;
	CaptureType = CaptureType_;
	switch (CaptureType)
	{
	case ECaptureType::ColorCapture:
		RenderTargetFormat = RTF_RGBA8;
		ROSStepMultiplier = 3;
	case ECaptureType::SegmentationCapture:
		RenderTargetFormat = RTF_RGBA8;
		ROSStepMultiplier = 3;
		// RenderTargetFormat = RTF_RGBA32f;
		// ROSStepMultiplier = 6;
		break;
	case ECaptureType::DepthCapture:
		RenderTargetFormat = RTF_RGBA32f;
		ROSStepMultiplier = 4;
		break;
	default:
		break;
	}
	if (!rosinst->ROSIntegrationCore) {return;}
	Topic = NewObject<UTopic>(UTopic::StaticClass());
	Topic->Init(rosinst->ROSIntegrationCore, topic_name, TEXT("sensor_msgs/Image"));
	Topic->Advertise();
}

/**
 * Compute the size of the ROS topic message size
 */
void UROSSceneCapture::RefreshImageTopicSize()
{
	//ROSEncoding = SceneCapture->TextureTarget->GetFormat();
	ImageMSG = MakeShareable(new ROSMessages::sensor_msgs::Image());
	ImageMSG->encoding = CheckROSEncoding();
	img = std::make_shared<uint8[]>(
		SceneCapture->TextureTarget->SizeX *
		SceneCapture->TextureTarget->SizeY *
		ROSStepMultiplier);
	ImageMSG->data = img.get();
	ImageMSG->header = ROSMessages::std_msgs::Header(0,FROSTime(0,0).Now(),"map");
	ImageMSG->width = SceneCapture->TextureTarget->SizeX;
	ImageMSG->height = SceneCapture->TextureTarget->SizeY;
	ImageMSG->is_bigendian = false;
	ImageMSG->step = SceneCapture->TextureTarget->SizeX * ROSStepMultiplier;
}

/**
 * read from current scene capture texture and publish it according to the datatype
 */
void UROSSceneCapture::Publish()
{
	is_publishing = true;
	if (is_streaming) {ReadRenderTargetPerRHIStream();} // fewer stutters but images may be outdated to the frame_id
	else {ReadRenderTargetPerRHIBlock();}	// accurate image to the frame_id
	
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		Publish(&ImageData8Bit);
		break;
	case RTF_R16f:
	case RTF_RG16f:
	case RTF_RGBA16f:
		Publish(&ImageData16Bit);
		break;
	case RTF_R32f:
	case RTF_RG32f:
	case RTF_RGBA32f:
		Publish(&ImageData32Bit);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unimplemented Format"))
	}
	is_publishing = false;
}

/**
 * converts the ue5 TArray image to ROS image datatype then publish it over the topic
 * @tparam T datatype for the image
 * @param Image TArray holding the ue5 texture 
 */
template<typename T>
void UROSSceneCapture::Publish(TArray<T>* Image)
{
	//UpdateFrameID();
	is_publishing = true;
	if (Topic && UpdateImageMsg(Image, img.get()))
	{
		//UE_LOG(LogTemp, Log, TEXT("publishing, %d, %p, %d"), Image->Num(), img.get(), img.get()!=nullptr)
		Topic->Publish(ImageMSG);
		//UE_LOG(LogTemp, Log, TEXT("published"))
		return;
	}
	is_publishing = false;
	//UE_LOG(LogTemp, Log, TEXT("failed publish: %s"), *topic_name)
}

/**
 * fill the ROS buffer with UE5 TArray information based on the TArray datatype
 * @tparam T datatype for the image
 * @param Image UE5 image in form of TArray
 * @param data ROS image buffer
 * @return whether the buffer was filled
 */
template <typename T>
bool UROSSceneCapture::UpdateImageMsg(TArray<T>* Image, uint8* data)
{
	if (Image->Num() < 100)
	{
		//UE_LOG(LogTemp, Log, TEXT("no image msg"))
		return false;
	}
	ImageMSG->header.time = FROSTime().Now();
	ImageMSG->header.frame_id = frame_id;
	CheckROSEncoding();
	if constexpr (std::is_same_v<T, FColor>)
	{
		for (int i = 0; i<Image->Num(); i++)
		{
			data[i*ROSStepMultiplier] = Image->GetData()[i].R;
			data[i*ROSStepMultiplier+1] = Image->GetData()[i].G;
			data[i*ROSStepMultiplier+2] = Image->GetData()[i].B;
			// if (CaptureType==ECaptureType::SegmentationCapture && Image->GetData()[i].B > 0)
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("%d, %d, %d"), Image->GetData()[i].R,Image->GetData()[i].G,Image->GetData()[i].B);
			// }
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
			
			float FloatVal;
			uint16 Value16;
			FloatVal = Image->GetData()[i].B * 255.0f;
			Value16 = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FloatVal), 0, 255));
			if (Value16 > 0 && CaptureType==ECaptureType::SegmentationCapture)
			{
				UE_LOG(LogTemp, Warning, TEXT("%d, %d: %d: %f, %f, %f, %f"), Image->Num(), i,Value16, FloatVal, Image->GetData()[i].B,Image->GetData()[i].R,Image->GetData()[i].G);
			}
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
			// Float16Bytes floatBytes;
   //          floatBytes.floatVal = Image->GetData()[i].R;
			// data[i*ROSStepMultiplier] = floatBytes.bytes[0];
			// data[i*ROSStepMultiplier+1] = floatBytes.bytes[1];
			//
			// floatBytes.floatVal = Image->GetData()[i].G;
			// data[i*ROSStepMultiplier+2] = floatBytes.bytes[0];
			// data[i*ROSStepMultiplier+3] = floatBytes.bytes[1];
			// floatBytes.floatVal = Image->GetData()[i].B;
			// data[i*ROSStepMultiplier+4] = floatBytes.bytes[0];
			// data[i*ROSStepMultiplier+5] = floatBytes.bytes[1];
			// floatBytes.floatVal = Image->GetData()[i].A;
			// data[i*ROSStepMultiplier+6] = floatBytes.bytes[0];
			// data[i*ROSStepMultiplier+7] = floatBytes.bytes[1];
			//data[i*4+3] = Image->GetData()[i].A;
			float FloatVal;
			uint16 Value16;
			FloatVal = Image->GetData()[i].R.GetFloat() * 255.0f;
			Value16 = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FloatVal), 0, 255));
			data[i*ROSStepMultiplier] = static_cast<uint8>(Value16 & 0xFF);
			data[i*ROSStepMultiplier+1] = static_cast<uint8>((Value16 >> 8) & 0xFF);
			FloatVal = Image->GetData()[i].G.GetFloat() * 255.0f;
			Value16 = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FloatVal), 0, 255));
			data[i*ROSStepMultiplier+2] = static_cast<uint8>(Value16 & 0xFF);
			data[i*ROSStepMultiplier+3] = static_cast<uint8>((Value16 >> 8) & 0xFF);
			FloatVal = Image->GetData()[i].B.GetFloat() * 255.0f;
			Value16 = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(FloatVal), 0, 255));
			data[i*ROSStepMultiplier+4] = static_cast<uint8>(Value16 & 0xFF);
			data[i*ROSStepMultiplier+5] = static_cast<uint8>((Value16 >> 8) & 0xFF);
			// if (Value16 > 70 && Value16 < 80)
			// {
			// 	// The color has drifted and is no longer a perfect 0-255 integer step
			// 	UE_LOG(LogTemp, Warning, TEXT("%d, %d: %d: %f, %f"), Image->Num(), i,Value16, FloatVal, Image->GetData()[i].B.GetFloat());
			// }
			// if (Value16 != 0)
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("%d: %f, %f"),Value16, FloatVal, Image->GetData()[i].B.GetFloat());
			// }
		}
	}
	return true;
}

/**
 * block the game to read render target texture on GPU and put it into CPU for publishing,
 * accurate to the game object positions, but blocks the game with stutters
 */
void UROSSceneCapture::ReadRenderTargetPerRHIBlock()
{
    auto RenderTarget = SceneCapture->TextureTarget;
    FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

    if (!RenderTargetResource) return;

    FTextureRHIRef TextureRHI = RenderTargetResource->GetRenderTargetTexture();
    if (!TextureRHI) return;

    // 1. Enqueue the read command directly
    ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
        [RenderTarget_RT = RenderTargetResource,
         SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
         OutData8 = &ImageData8Bit,
         OutData16 = &ImageData16Bit,
         OutData32 = &ImageData32Bit,
         Format = TextureRHI->GetDesc().Format,
		this]
        (FRHICommandListImmediate& RHICmdList)
        {
            if (Format == PF_B8G8R8A8) {
                RHICmdList.ReadSurfaceData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData8, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
            } else if (Format == PF_FloatRGBA) {
                RHICmdList.ReadSurfaceFloatData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData16, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX));
            } else if (Format == PF_A32B32G32R32F) {
                RHICmdList.ReadSurfaceData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData32, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX));
            }
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				UpdateFrameID(true);
			});
        });

    // 2. Create a fence and wait for the Render Thread to catch up
    FRenderCommandFence ReadFence;
    ReadFence.BeginFence();
    ReadFence.Wait(); // This blocks the Game Thread until the pixels are in the TArray
	// You'll need the "ImageWriteQueue" module or "ExrImageWrapper"
	// TArray<uint8> ExrData;
	// FImageUtils::CompressImageArray(RenderTarget->SizeX, RenderTarget->SizeY, ImageData16Bit, ExrData); // ImageUtils has overloads for Float16
	// FFileHelper::SaveArrayToFile(ExrData, *FilePath);
}
/**
 * async read render target texture on GPU and put it into CPU for publishing,
 * slightly higher FPS than alternatives
 */
void UROSSceneCapture::ReadRenderTargetPerRHIStream()
{
	auto RenderTarget = SceneCapture->TextureTarget;
	FTextureRenderTargetResource *RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[RenderTargetResource, this, RenderTarget]()
	{
		//UE_LOG(LogTemp, Log, TEXT("AsyncTask"))
		if (!RenderTargetResource)
		{
			return;
		}
		FTextureRHIRef TextureRHI = RenderTargetResource->GetRenderTargetTexture();
        if (!TextureRHI)
        {
            UE_LOG(LogTemp, Warning, TEXT("RenderTargetTexture is null"));
            return;
        }
		switch (TextureRHI->GetDesc().Format)
		{
		case PF_B8G8R8A8:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData8Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX),
				this]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					UpdateFrameID(true);
				});
			});
			//RenderTargetResource->ReadPixels(this->ImageData8Bit);
			break;
		case PF_FloatRGBA:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData16Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX),
				this]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceFloatData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					UpdateFrameID(true);
				});
			});
			//RenderTargetResource->ReadFloat16Pixels(this->ImageData16Bit);
			break;
		case PF_A32B32G32R32F:
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData32Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX),
				this]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceData(RenderTarget_RT->GetRenderTargetTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
				AsyncTask(ENamedThreads::GameThread, [this]()
				{
					UpdateFrameID(true);
				});
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

void UROSSceneCapture::UpdateFrameID(bool force)
{
	// uint32 FrameID = 0;
	// for (AActor* IDSource : IDSources){
	// 	if (IDSource) {
	// 		FVector Loc = IDSource->GetActorLocation();
	// 		FrameID = HashCombine(FrameID, GetTypeHash(Loc));
			
	// 	}
	// }
	// frame_id = LexToString(FrameID);
	if (is_publishing && !force) {return;}

	FString FrameIDString;

	for (AActor* IDSource : IDSources)
	{
		if (IDSource)
		{
			FVector Loc = IDSource->GetActorLocation();
			FrameIDString += FString::Printf(TEXT("%.2f_%.2f_%.2f;"), Loc.X, Loc.Y, Loc.Z);
		}
	}

	// Optional: remove the last semicolon
	if (FrameIDString.EndsWith(";"))
	{
		FrameIDString.LeftChopInline(1);
	}

	// Now frame_id is a string representation of all locations
	frame_id = FrameIDString;
}

/**
 * update camera parameters using the Camera variable, and then filters for the actors that should be rendered
 * based on the capture type
 * @param Camera CameraComponent with desired parameters such as FOV, post process
 * @param WorldContext GetWorld()
 */
void UROSSceneCapture::UpdateSceneCaptureCameraParameters(
		UCameraComponent* Camera,
		UWorld* WorldContext
		)
{
	FMinimalViewInfo MinimalViewInfo;
	if (!Camera) {return;}
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
	
	// actors that have a mix of rgb and segment components
	TArray<AActor*> MixedActors_L;
	UGameplayStatics::GetAllActorsWithTag(WorldContext, "Depends", MixedActors_L);
	SceneCapture->ShowOnlyComponents.Empty();
	for (AActor* Actor : MixedActors_L)
	{
		if (!Actor) continue;
		TArray<UActorComponent*> Components;
		switch (CaptureType)
		{
		case ECaptureType::ColorCapture:
		case ECaptureType::DepthCapture:
			Components = Actor->GetComponentsByTag(UPrimitiveComponent::StaticClass(), FName("RGBComponent"));
			break;
		case ECaptureType::SegmentationCapture:
			Components = Actor->GetComponentsByTag(UPrimitiveComponent::StaticClass(), FName("SegmentationComponent"));
			break;
		default:
			break;
		}
		for (UActorComponent* Component : Components)
		{
			if (!Component) continue;
			UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
			if (!PrimitiveComponent) continue;
			SceneCapture->ShowOnlyComponents.Add(PrimitiveComponent);
		}
	}
}

/**
 * 
 * @return the encoding string for the ROS Image Message
 */
FString UROSSceneCapture::CheckROSEncoding()
{
	switch (RenderTargetFormat)
	{
	case RTF_RGBA8:
		ROSStepMultiplier = 3;
		return "rgb8";
	case RTF_R16f:
	case RTF_RG16f:
		ROSStepMultiplier = 2;
		return "16SC1";
	case RTF_RGBA16f:
		ROSStepMultiplier = 6;
        return "rgb16";
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

