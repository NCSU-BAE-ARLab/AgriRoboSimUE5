// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralMesh/ProceduralPlantMesh.h"





// Sets default values for this component's properties
UProceduralPlantMesh::UProceduralPlantMesh()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UProceduralPlantMesh::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UProceduralPlantMesh::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (StreamingTask && !StreamingTask->bFinished)
    {
        TickStreamingOBJ(50); // 50 faces per frame, adjust for performance
    }
	// ...
}

void UProceduralPlantMesh::CreateMesh()
{
    if (!ProcMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcMesh is null! Assign a ProceduralMeshComponent first."));
        return;
    }

    // Clear any previous mesh
    ProcMesh->ClearAllMeshSections();

    // Define cube size
    const float CubeSize = 100.0f;

    // Define 8 cube vertices
    TArray<FVector> Vertices;
    Vertices.Add(FVector(0, 0, 0));                    // 0 - bottom front left
    Vertices.Add(FVector(CubeSize, 0, 0));             // 1 - bottom front right
    Vertices.Add(FVector(CubeSize, CubeSize, 0));      // 2 - bottom back right
    Vertices.Add(FVector(0, CubeSize, 0));             // 3 - bottom back left
    Vertices.Add(FVector(0, 0, CubeSize));             // 4 - top front left
    Vertices.Add(FVector(CubeSize, 0, CubeSize));      // 5 - top front right
    Vertices.Add(FVector(CubeSize, CubeSize, CubeSize)); // 6 - top back right
    Vertices.Add(FVector(0, CubeSize, CubeSize));      // 7 - top back left

    // Define triangles (each face = 2 triangles = 6 indices)
    TArray<int32> Triangles = {
        // Bottom face (0,1,2,3)
        0, 2, 1,
        0, 3, 2,

        // Top face (4,5,6,7)
        4, 5, 6,
        4, 6, 7,

        // Front face (0,1,5,4)
        0, 1, 5,
        0, 5, 4,

        // Back face (3,2,6,7)
        3, 6, 2,
        3, 7, 6,

        // Left face (0,4,7,3)
        0, 7, 4,
        0, 3, 7,

        // Right face (1,2,6,5)
        1, 6, 2,
        1, 5, 6
    };

    // Generate normals
    TArray<FVector> Normals;
    Normals.Init(FVector(0, 0, 1), Vertices.Num());

    // UV mapping
    TArray<FVector2D> UVs;
    UVs.Init(FVector2D(0, 0), Vertices.Num());
    for (int32 i = 0; i < UVs.Num(); i++)
    {
        UVs[i] = FVector2D((Vertices[i].X / CubeSize), (Vertices[i].Y / CubeSize));
    }

    // Empty color/tangent arrays
    TArray<FLinearColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    // Create the cube mesh section
    ProcMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
}

void UProceduralPlantMesh::CreateMeshFromOBJString(
	const FString& OBJData, const FString& MTLData, const bool bFlipNormal,
	TArray<FString>& MTLMaterialName, TArray<FString>& MTLMaterialDef, TArray<FString>& OBJMaterialName)
{
    if (!ProcMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcMesh is null! Assign a ProceduralMeshComponent first."));
        return;
    }
	MTLMaterialName.Empty();
	MTLMaterialDef.Empty();
	OBJMaterialName.Empty();
	TArray<FString> MTLLines;
    MTLData.ParseIntoArrayLines(MTLLines);
	for (const FString& Line : MTLLines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.StartsWith(TEXT("newmtl ")))
		{
			MTLMaterialName.Add(TrimmedLine.RightChop(7));
		}
		else if (TrimmedLine.StartsWith(TEXT("map_Kd ")))
		{
			MTLMaterialDef.Add(TrimmedLine.RightChop(7));
		}
		else if (TrimmedLine.StartsWith(TEXT("Kd ")))
		{
			MTLMaterialDef.Add(TrimmedLine.RightChop(3));
		}
	}

    // Clear any previous mesh
    ProcMesh->ClearAllMeshSections();
	
    // Arrays for mesh data
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Temporary arrays to store parsed OBJ data
    TArray<FVector> TempVertices;
    TArray<FVector> TempNormals;
    TArray<FVector2D> TempUVs;

    // Split string into lines
    TArray<FString> Lines;
    OBJData.ParseIntoArrayLines(Lines);

	int32 CurrentIndex = 0;
    for (const FString& Line : Lines)
    {
        FString TrimmedLine = Line.TrimStartAndEnd();
		// section (needs new material)
		if (TrimmedLine.StartsWith(TEXT("usemtl "))) 
		{
			// If no normals were provided, generate simple normals
            Normals.Empty();
			if (Normals.Num() != Vertices.Num())
			{
				Normals.Init(FVector::ZeroVector, Vertices.Num());
				for (int32 i = 0; i < Triangles.Num(); i += 3)
				{
					const int32 I0 = Triangles[i];
					const int32 I1 = Triangles[i + 1];
					const int32 I2 = Triangles[i + 2];
					const FVector FaceNormal = FVector::CrossProduct(Vertices[I2] - Vertices[I0], Vertices[I1] - Vertices[I0]).GetSafeNormal();
					Normals[I0] += FaceNormal;
					Normals[I1] += FaceNormal;
					Normals[I2] += FaceNormal;
				}
				for (FVector& N : Normals)
				{
					N.Normalize();
				}
			}
			OBJMaterialName.Add(TrimmedLine.RightChop(7));
			ProcMesh->CreateMeshSection_LinearColor(CurrentIndex, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
			CurrentIndex++;
			Vertices.Empty();
			Triangles.Empty();
			Normals.Empty();
			UVs.Empty();
		}

        if (TrimmedLine.StartsWith(TEXT("v ")))
        {
            // Vertex
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
            {
                FVector Vertex(FCString::Atof(*Tokens[1])*100, FCString::Atof(*Tokens[2])*100, FCString::Atof(*Tokens[3])*100);
                TempVertices.Add(Vertex);
            }
        }
        else if (TrimmedLine.StartsWith(TEXT("vt ")))
        {
            // UV
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 3)
            {
                FVector2D UV(FCString::Atof(*Tokens[1]), FCString::Atof(*Tokens[2]));
                TempUVs.Add(UV);
            }
        }
        else if (TrimmedLine.StartsWith(TEXT("vn ")))
        {
            // Normal
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
            {
                FVector Normal(FCString::Atof(*Tokens[1]), FCString::Atof(*Tokens[2]), FCString::Atof(*Tokens[3]));
                TempNormals.Add(Normal);
            }
        }
        else if (TrimmedLine.StartsWith(TEXT("f ")))
        {
            // Face
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);

            for (int32 i = 1; i + 2 < Tokens.Num(); i++)
            {
                TArray<FString> Vertex0, Vertex1, Vertex2;
                Tokens[1].ParseIntoArray(Vertex0, TEXT("/"));
                Tokens[i + 1].ParseIntoArray(Vertex1, TEXT("/"));
                Tokens[i + 2].ParseIntoArray(Vertex2, TEXT("/"));

                auto AddIndex = [&](const TArray<FString>& VertexToken)
                {
                    int32 VertexIndex = FCString::Atoi(*VertexToken[0]) - 1;
                    Vertices.Add(TempVertices[VertexIndex]);

                    if (VertexToken.Num() > 1 && !VertexToken[1].IsEmpty())
                    {
                        int32 UVIndex = FCString::Atoi(*VertexToken[1]) - 1;
                        UVs.Add(TempUVs.IsValidIndex(UVIndex) ? TempUVs[UVIndex] : FVector2D(0, 0));
                    }
                    else
                    {
                        UVs.Add(FVector2D(0, 0));
                    }

                    if (VertexToken.Num() > 2 && !VertexToken[2].IsEmpty())
                    {
                        int32 NormalIndex = FCString::Atoi(*VertexToken[2]) - 1;
                        Normals.Add(TempNormals.IsValidIndex(NormalIndex) ? TempNormals[NormalIndex] : FVector(0, 0, 1));
                    }
                    else
                    {
                        Normals.Add(FVector(0, 0, 1));
                    }

                    Triangles.Add(Vertices.Num() - 1);
                };
                if (bFlipNormal)
                {
                    AddIndex(Vertex2);
                    AddIndex(Vertex1);
                    AddIndex(Vertex0);
                } else {
                    AddIndex(Vertex0);
                    AddIndex(Vertex1);
                    AddIndex(Vertex2);
                }
                
            }
        }
    }



    // Create last mesh section
    // If no normals were provided, generate simple normals
	if (Normals.Num() != Vertices.Num())
	{
		Normals.Init(FVector::ZeroVector, Vertices.Num());
		for (int32 i = 0; i < Triangles.Num(); i += 3)
		{
			const int32 I0 = Triangles[i];
			const int32 I1 = Triangles[i + 1];
			const int32 I2 = Triangles[i + 2];
			const FVector FaceNormal = FVector::CrossProduct(Vertices[I2] - Vertices[I0], Vertices[I1] - Vertices[I0]).GetSafeNormal();
			Normals[I0] += FaceNormal;
			Normals[I1] += FaceNormal;
			Normals[I2] += FaceNormal;
		}
		for (FVector& N : Normals)
		{
			N.Normalize();
		}
	}
	ProcMesh->CreateMeshSection_LinearColor(CurrentIndex, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
	
	
}

void UProceduralPlantMesh::LoadOBJAsync(const FString& OBJData, const FString& MTLData, bool bFlipNormal)
{
    if (!ProcMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProcMesh is null! Assign a ProceduralMeshComponent first."));
        return;
    }

    // Delete any previous task
    if (StreamingTask)
    {
        delete StreamingTask;
        StreamingTask = nullptr;
    }

    // Initialize new streaming task
    StreamingTask = new FStreamingOBJTask();
    StreamingTask->OBJData = OBJData;
    StreamingTask->MTLData = MTLData;
    StreamingTask->bFlipNormal = bFlipNormal;
    StreamingTask->ProcMesh = ProcMesh;

    // Parse MTL lines upfront
    MTLData.ParseIntoArrayLines(StreamingTask->MTLLines);
    for (const FString& Line : StreamingTask->MTLLines)
    {
        FString TrimmedLine = Line.TrimStartAndEnd();
        if (TrimmedLine.StartsWith(TEXT("newmtl ")))
            StreamingTask->MTLMaterialName.Add(TrimmedLine.RightChop(7));
        else if (TrimmedLine.StartsWith(TEXT("map_Kd ")))
            StreamingTask->MTLMaterialDef.Add(TrimmedLine.RightChop(7));
        else if (TrimmedLine.StartsWith(TEXT("Kd ")))
            StreamingTask->MTLMaterialDef.Add(TrimmedLine.RightChop(3));
    }

    OBJData.ParseIntoArrayLines(StreamingTask->OBJLines);

    // Clear previous mesh sections
    ProcMesh->ClearAllMeshSections();
}


void UProceduralPlantMesh::TickStreamingOBJ(int32 FacesPerTick)
{
    if (!StreamingTask || StreamingTask->bFinished || !StreamingTask->ProcMesh)
        return;

    int32 FacesProcessed = 0;
    const int32 TotalLines = StreamingTask->OBJLines.Num();

    while (StreamingTask->CurrentLineIndex < TotalLines && FacesProcessed < FacesPerTick)
    {
        FString Line = StreamingTask->OBJLines[StreamingTask->CurrentLineIndex].TrimStartAndEnd();

        if (Line.StartsWith(TEXT("v ")))
        {
            TArray<FString> Tokens;
            Line.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
                StreamingTask->TempVertices.Add(FVector(FCString::Atof(*Tokens[1])*100, FCString::Atof(*Tokens[2])*100, FCString::Atof(*Tokens[3])*100));
        }
        else if (Line.StartsWith(TEXT("vt ")))
        {
            TArray<FString> Tokens;
            Line.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 3)
                StreamingTask->TempUVs.Add(FVector2D(FCString::Atof(*Tokens[1]), FCString::Atof(*Tokens[2])));
        }
        else if (Line.StartsWith(TEXT("vn ")))
        {
            TArray<FString> Tokens;
            Line.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
                StreamingTask->TempNormals.Add(FVector(FCString::Atof(*Tokens[1]), FCString::Atof(*Tokens[2]), FCString::Atof(*Tokens[3])));
        }
        else if (Line.StartsWith(TEXT("f ")))
        {
            TArray<FString> Tokens;
            Line.ParseIntoArray(Tokens, TEXT(" "), true);

            for (int32 i = 1; i + 2 < Tokens.Num(); i++)
            {
                TArray<FString> V0, V1, V2;
                Tokens[1].ParseIntoArray(V0, TEXT("/"));
                Tokens[i + 1].ParseIntoArray(V1, TEXT("/"));
                Tokens[i + 2].ParseIntoArray(V2, TEXT("/"));

                auto AddVertex = [&](const TArray<FString>& VT)
                {
                    int32 VIndex = FCString::Atoi(*VT[0]) - 1;
                    StreamingTask->Vertices.Add(StreamingTask->TempVertices[VIndex]);

                    if (VT.Num() > 1 && !VT[1].IsEmpty())
                        StreamingTask->UVs.Add(StreamingTask->TempUVs.IsValidIndex(FCString::Atoi(*VT[1])-1) ? StreamingTask->TempUVs[FCString::Atoi(*VT[1])-1] : FVector2D(0,0));
                    else StreamingTask->UVs.Add(FVector2D(0,0));

                    if (VT.Num() > 2 && !VT[2].IsEmpty())
                        StreamingTask->Normals.Add(StreamingTask->TempNormals.IsValidIndex(FCString::Atoi(*VT[2])-1) ? StreamingTask->TempNormals[FCString::Atoi(*VT[2])-1] : FVector(0,0,1));
                    else StreamingTask->Normals.Add(FVector(0,0,1));

                    StreamingTask->Triangles.Add(StreamingTask->Vertices.Num() - 1);
                };

                if (StreamingTask->bFlipNormal)
                {
                    AddVertex(V2);
                    AddVertex(V1);
                    AddVertex(V0);
                }
                else
                {
                    AddVertex(V0);
                    AddVertex(V1);
                    AddVertex(V2);
                }
            }

            FacesProcessed++;
        }

        StreamingTask->CurrentLineIndex++;
    }

    // Update mesh each tick
    if (StreamingTask->Vertices.Num() > 0)
    {
        StreamingTask->ProcMesh->CreateMeshSection_LinearColor(
            StreamingTask->CurrentMeshIndex,
            StreamingTask->Vertices,
            StreamingTask->Triangles,
            StreamingTask->Normals,
            StreamingTask->UVs,
            TArray<FLinearColor>(),
            TArray<FProcMeshTangent>(),
            true
        );
    }

    if (StreamingTask->CurrentLineIndex >= TotalLines)
        StreamingTask->bFinished = true;
}
