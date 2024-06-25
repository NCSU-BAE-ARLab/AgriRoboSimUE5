// Fill out your copyright notice in the Description page of Project Settings.


#include "ROSCameraControl.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

void FROSSceneCapture::RefreshImageSize()
{
	ROSEncoding = SceneCapture->TextureTarget->GetFormat();
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

void FROSSceneCapture::Publish()
{
	switch (ROSEncoding)
	{
	case PF_B8G8R8A8:
	case PF_R8G8B8A8:
	case PF_A8R8G8B8:
		ReadRenderTargetPerRHI();
		Publish(&ImageData8Bit);
		break;
	case PF_FloatRGBA:
        ReadRenderTargetPerRHI();
        Publish(&ImageData16Bit);
		break;
	case PF_A32B32G32R32F:
		ReadRenderTargetPerRHI();
		Publish(&ImageData32Bit);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unimplemented Format"))
	}
}

template<typename T>
void FROSSceneCapture::Publish(TArray<T>* Image)
{
	
	//TSharedPtr<ROSMessages::sensor_msgs::Image> const ImageMSG(new ROSMessages::sensor_msgs::Image());
	
	
	//uint8* img = new uint8[Image->Num()*ROSStepMultiplier];
	//std::unique_ptr<uint8[]> img(new uint8[Image->Num() * ROSStepMultiplier]);
	if (UpdateImageMsg(Image, img.get()))
	{
		UE_LOG(LogTemp, Log, TEXT("publishing, %d, %p, %d"), Image->Num(), img.get(), img.get()!=nullptr)

		Topic->Publish(ImageMSG);

		UE_LOG(LogTemp, Log, TEXT("published"))
	}
}

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

void FROSSceneCapture::ReadRenderTargetPerRHI()
{
	auto RenderTarget = SceneCapture->TextureTarget;
	FTextureRenderTargetResource *RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	//UE_LOG(LogTemp, Log, TEXT("RT DIM: {%d}"), RenderTarget->bGPUSharedFlag)
	//UE_LOG(LogTemp, Log, TEXT("Attemp Read RT: {%d}"),RenderTargetResource->GetRenderTargetTexture()->GetDesc().Format)
	//FImage Image_Read;
	//FImageUtils::GetRenderTargetImage(RenderTarget2D, Image_Read);
	
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


FString FROSSceneCapture::CheckROSEncoding()
{
	switch (ROSEncoding)
	{
	case PF_B8G8R8A8:
	case PF_R8G8B8A8:
	case PF_A8R8G8B8:
		ROSStepMultiplier = 3;
		return "rgb8";
	case PF_FloatRGBA:
		ROSStepMultiplier = 2;
        return "16SC1";
	case PF_A32B32G32R32F:
		ROSStepMultiplier = 4;
		return "32FC1";
	default:
		ROSStepMultiplier = 1;
		return "undefined(UE5_Encoding)";
	}
}

void UROSCameraControl::InitROSTopics()
{
	InitSceneCaptures();
	for (auto Element : SceneCaptures)
	{
		UE_LOG(LogTemp, Log, TEXT("Started Image Topic on: /ue5/%s/%s"),*TopicPrefix,*Element->SceneCapture->GetName())
		if (Element->SceneCapture->GetName().Contains("Color"))
		{
			Color_Topic = NewObject<UTopic>(UTopic::StaticClass());
            Color_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+Element->SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
            Color_Topic->Advertise();
			Element->Topic = Color_Topic;
		}
		if (Element->SceneCapture->GetName().Contains("Depth"))
        {
			Depth_Topic = NewObject<UTopic>(UTopic::StaticClass());
			Depth_Topic->Init(rosinst->ROSIntegrationCore, TEXT("/ue5/")+TopicPrefix+TEXT("/")+Element->SceneCapture->GetName(), TEXT("sensor_msgs/Image"));
			Depth_Topic->Advertise();
        	Element->Topic = Depth_Topic;
        }
		if (Element->SceneCapture->TextureTarget)
		{
			UE_LOG(LogTemp, Log, TEXT("Getting Texture Target Params"))
			Element->RefreshImageSize();
		} else
		{
			UE_LOG(LogTemp, Log, TEXT("NO Texture Target"))
		}
		
	}
}

void UROSCameraControl::InitSceneCaptures()
{
	SceneCaptures.Empty();
	TArray<USceneCaptureComponent2D*> L_SceneCaptures;
	this->GetOwner()->GetComponents<USceneCaptureComponent2D>(L_SceneCaptures);
	UE_LOG(LogTemp, Log, TEXT("Scene Capture Counts: %d"),L_SceneCaptures.Num())
	for (int i = 0; i < L_SceneCaptures.Num(); ++i)
	{
		TSharedPtr<FROSSceneCapture> NewStruct = MakeShared<FROSSceneCapture>();
		NewStruct->SceneCapture = L_SceneCaptures[i];
		SceneCaptures.Add(NewStruct);
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

void UROSCameraControl::PublishSceneCaptureToTopic()
{
}


// Called every frame
void UROSCameraControl::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//PublishAllTopic();
	// ...
}

void UROSCameraControl::ReadRenderTargetPerRHI(UTextureRenderTarget2D* RenderTarget)
{
	FTextureRenderTargetResource *RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	UE_LOG(LogTemp, Log, TEXT("RT DIM: {%d}"), RenderTarget->bGPUSharedFlag)
	UE_LOG(LogTemp, Log, TEXT("Attemp Read RT: {%d}"),RenderTargetResource->GetRenderTargetTexture()->GetDesc().Format)
	//FImage Image_Read;
	//FImageUtils::GetRenderTargetImage(RenderTarget2D, Image_Read);
	
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
			UE_LOG(LogTemp, Log, TEXT("RT READ: {%d}"), this->ImageData8Bit.Num())
			break;
		case PF_FloatRGBA:
			//RenderTargetResource->ReadFloat16Pixels(this->ImageData16Bit);
			ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
			[RenderTarget_RT = RenderTargetResource,
				SrcRect_RT = FIntRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY),
				OutData_RT = &ImageData16Bit,
				Flags_RT = FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)]
			(FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.ReadSurfaceFloatData(RenderTarget_RT->GetShaderResourceTexture(), SrcRect_RT, *OutData_RT, Flags_RT);
			});
			//UE_LOG(LogTemp, Log, TEXT("RT READ: {%d}"), this->ImageData16Bit.Num())
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
			//UE_LOG(LogTemp, Log, TEXT("RT READ: {%d}"), this->ImageData32Bit.Num())
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("NO RT READ: unclear pixel format"))
		}
			
	});
	FlushRenderingCommands();
}

void UROSCameraControl::ReadRenderTargetPerUtil(UTextureRenderTarget2D* RenderTarget)
{
	FImageUtils::GetRenderTargetImage(RenderTarget, ImageDataUtil);
	UE_LOG(LogTemp, Log, TEXT("%lld, %lld"), ImageDataUtil.GetWidth(), ImageDataUtil.GetHeight())
}

UTextureRenderTarget2D* UROSCameraControl::CreateRenderTarget(int Width, int Height, ETextureRenderTargetFormat Format)
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
		//ReadRenderTargetPerRHI(SceneCapture->SceneCapture->TextureTarget);
		SceneCapture->Publish();
		//SceneCapture->Publish();
	}
}

