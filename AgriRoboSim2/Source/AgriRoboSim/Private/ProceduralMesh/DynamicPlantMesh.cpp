#include "ProceduralMesh/DynamicPlantMesh.h"
#include "RenderCommandFence.h"
// Sets default values for this component's properties
UDynamicPlantMesh::UDynamicPlantMesh()
{
	PrimaryComponentTick.bCanEverTick = true;
}
// Called when the game starts
void UDynamicPlantMesh::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UDynamicPlantMesh::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Test mesh
void UDynamicPlantMesh::CreateMesh()
{
	using namespace UE::Geometry;
	const FString CubeOBJ = TEXT(
        "v -50 -50 -50\n"
        "v 50 -50 -50\n"
        "v 50 50 -50\n"
        "v -50 50 -50\n"
        "v -50 -50 50\n"
        "v 50 -50 50\n"
        "v 50 50 50\n"
        "v -50 50 50\n"
        "f 1 2 3 4\n"
        "f 5 6 7 8\n"
        "f 1 5 8 4\n"
        "f 2 6 7 3\n"
        "f 4 3 7 8\n"
        "f 1 2 6 5\n"
    );
	Mesh = FDynamicMesh3();
    Mesh.EnableAttributes();  // enable normals/UV support (optional)
	
    Vertices.Empty();
    TArray<FIndex3i> Triangles;

    TArray<FString> Lines;
	CubeOBJ.ParseIntoArrayLines(Lines);

    for (const FString& Line : Lines)
    {
        FString Trimmed = Line.TrimStartAndEnd();

        // Parse vertex positions
        if (Trimmed.StartsWith(TEXT("v ")))
        {
            TArray<FString> Parts;
            Trimmed.RightChop(2).TrimStartAndEnd().ParseIntoArray(Parts, TEXT(" "), true);
            if (Parts.Num() >= 3)
            {
                double X, Y, Z;
                FDefaultValueHelper::ParseDouble(Parts[0], X);
                FDefaultValueHelper::ParseDouble(Parts[1], Y);
                FDefaultValueHelper::ParseDouble(Parts[2], Z);
                Vertices.Add(FVector3d(X, Y, Z));
            }
        }

        // Parse faces (tri or quad)
        else if (Trimmed.StartsWith(TEXT("f ")))
        {
            TArray<FString> Parts;
            Trimmed.RightChop(2).TrimStartAndEnd().ParseIntoArray(Parts, TEXT(" "), true);
            if (Parts.Num() == 3)
            {
                int32 A = FCString::Atoi(*Parts[0]) - 1;
                int32 B = FCString::Atoi(*Parts[1]) - 1;
                int32 C = FCString::Atoi(*Parts[2]) - 1;
                Triangles.Add(FIndex3i(A, B, C));
            }
            else if (Parts.Num() == 4) // split quad into two tris
            {
                int32 A = FCString::Atoi(*Parts[0]) - 1;
                int32 B = FCString::Atoi(*Parts[1]) - 1;
                int32 C = FCString::Atoi(*Parts[2]) - 1;
                int32 D = FCString::Atoi(*Parts[3]) - 1;
                Triangles.Add(FIndex3i(A, B, C));
                Triangles.Add(FIndex3i(A, C, D));
            }
        }
    }

    // Build mesh
    for (const FVector3d& V : Vertices)
        Mesh.AppendVertex(V);
    for (const FIndex3i& T : Triangles)
        Mesh.AppendTriangle(T);

    // Upload to component
    DynMesh->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
    {
        EditMesh = Mesh;
    }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown);
}

void UDynamicPlantMesh::CreateMeshFromOBJString(
	const FString& OBJData, const FString& MTLData, const bool bFlipNormal)
{
    ConfigureMeshObj(OBJData, MTLData);
    AsyncConstructMesh(-1);
    FinalizeMesh();
    return;
	// using namespace UE::Geometry;
	// if (!DynMesh)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("DynMesh is null! Assign a DynamicMeshComponent first."));
    //     return;
    // }

    // MTLMaterialName.Empty();
    // MTLMaterialDef.Empty();
    // OBJMaterialName.Empty();
    // MTLLines.Empty();
    // OBJLines.Empty();
    // // Parse MTL
    // MTLData.ParseIntoArrayLines(MTLLines);
    // for (const FString& Line : MTLLines)
    // {
    //     FString TrimmedLine = Line.TrimStartAndEnd();
    //     if (TrimmedLine.StartsWith(TEXT("newmtl ")))
    //     {
    //         MTLMaterialName.Add(TrimmedLine.RightChop(7));
    //     }
    //     else if (TrimmedLine.StartsWith(TEXT("map_Kd ")))
    //     {
    //         MTLMaterialDef.Add(TrimmedLine.RightChop(7));
    //     }
    //     else if (TrimmedLine.StartsWith(TEXT("Kd ")))
    //     {
    //         MTLMaterialDef.Add(TrimmedLine.RightChop(3));
    //     }
    // }

    // // Clear previous mesh
    // Mesh = FDynamicMesh3();
    // Mesh.EnableAttributes(); // enables normals and UV overlays
	// Mesh.EnableTriangleGroups();
	// Mesh.Attributes()->EnableMaterialID();
	// Mesh.EnableVertexNormals(FVector3f(0,0,0));
	
    // Vertices.Empty();
    // Normals.Empty();
    // UVs.Empty();
    // // TArray<FVector3d> TempVertices;
    // // TArray<FVector3d> TempNormals;
    // // TArray<FVector2f> TempUVs;
    // // TArray<FIndex3i> Triangles;

    // OBJData.ParseIntoArrayLines(OBJLines);

    // CurrentMaterialSection = -1; // will be 0 after usemtl

    // for (const FString& Line : OBJLines)
    // {
    //     FString TrimmedLine = Line.TrimStartAndEnd();

    //     // Material change
    //     if (TrimmedLine.StartsWith(TEXT("usemtl ")))
    //     {
    //         OBJMaterialName.Add(TrimmedLine.RightChop(7));
    //         CurrentMaterialSection++;
			
    //     }

    //     // Vertex
    //     else if (TrimmedLine.StartsWith(TEXT("v ")))
    //     {
    //         TArray<FString> Tokens;
    //         TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
    //         if (Tokens.Num() >= 4)
    //         {
    //             FVector3d V(
    //                 FCString::Atod(*Tokens[1]) * 100,
    //                 FCString::Atod(*Tokens[2]) * 100,
    //                 FCString::Atod(*Tokens[3]) * 100
    //             );
    //             Vertices.Add(V);
	// 			int vid = Mesh.AppendVertex(V);
	// 			//UE_LOG(LogTemp, Warning, TEXT("VID: %d"), vid);
    //         }
    //     }

    //     // UV
    //     else if (TrimmedLine.StartsWith(TEXT("vt ")))
    //     {
    //         TArray<FString> Tokens;
    //         TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
    //         if (Tokens.Num() >= 3)
    //         {
    //             UVs.Add(FVector2f(FCString::Atod(*Tokens[1]), FCString::Atod(*Tokens[2])));
    //         }
    //     }

    //     // Normal
    //     else if (TrimmedLine.StartsWith(TEXT("vn ")))
    //     {
    //         TArray<FString> Tokens;
    //         TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
    //         if (Tokens.Num() >= 4)
    //         {
    //             Normals.Add(FVector3d(FCString::Atod(*Tokens[1]), FCString::Atod(*Tokens[2]), FCString::Atod(*Tokens[3])));
    //         }
    //     }

    //     // Face
    //     else if (TrimmedLine.StartsWith(TEXT("f ")))
    //     {
    //         TArray<FString> Tokens;
    //         TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
    //         for (int32 i = 1; i + 2 < Tokens.Num(); ++i)
    //         {
    //             TArray<FString> V0, V1, V2;
    //             Tokens[1].ParseIntoArray(V0, TEXT("/"));
    //             Tokens[i + 1].ParseIntoArray(V1, TEXT("/"));
    //             Tokens[i + 2].ParseIntoArray(V2, TEXT("/"));
				
    //             auto AddTriangle = [&](const TArray<FString>& A, const TArray<FString>& B, const TArray<FString>& C)
    //             {
	// 				int32 Idx0 = FCString::Atoi(*A[0]) - 1;
	// 				int32 Idx1 = FCString::Atoi(*B[0]) - 1;
	// 				int32 Idx2 = FCString::Atoi(*C[0]) - 1;
	// 				if (Idx0 != Idx1 && Idx0 != Idx2 && Idx1 != Idx2) {
	// 					int TriID = Mesh.AppendTriangle(FIndex3i(Idx0, Idx1, Idx2), CurrentMaterialSection);
    //                     if (TriID < 0)
    //                     {
    //                         UE_LOG(LogTemp, Warning, TEXT("Skipping triangle: already exists or invalid: %d %d %d"), Idx0, Idx1, Idx2);
    //                         return;
    //                     }
	// 					// UV overlay
	// 					if (Mesh.Attributes() && Mesh.Attributes()->PrimaryUV() && A.Num() > 1 && !A[1].IsEmpty())
	// 					{
	// 						FDynamicMeshUVOverlay* UVOverlay = Mesh.Attributes()->PrimaryUV();
	// 						int32 UVIndex0 = FCString::Atoi(*A[1]) - 1;
	// 						int32 UVIndex1 = FCString::Atoi(*B[1]) - 1;
	// 						int32 UVIndex2 = FCString::Atoi(*C[1]) - 1;
    //                         // UE_LOG(LogTemp, Warning, TEXT("ABCIdx: %d %d %d"), FCString::Atoi(*A[0]), FCString::Atoi(*B[0]), FCString::Atoi(*C[0]));
    //                         // UE_LOG(LogTemp, Warning, TEXT("Idx: %d %d %d"), Idx0, Idx1, Idx2);
    //                         // UE_LOG(LogTemp, Warning, TEXT("UVIndex: %d %d %d"), UVIndex0, UVIndex1, UVIndex2);

	// 						int32 UVElem0 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex0) ? UVs[UVIndex0] : FVector2f(0,0));
	// 						int32 UVElem1 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex1) ? UVs[UVIndex1] : FVector2f(0,0));
	// 						int32 UVElem2 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex2) ? UVs[UVIndex2] : FVector2f(0,0));

	// 						// Associate UV elements with triangle
                        
    //                         //UE_LOG(LogTemp, Warning, TEXT("TriID: %d UV: %d %d %d"), TriID, UVElem0, UVElem1, UVElem2);
	// 						UVOverlay->SetTriangle(TriID, FIndex3i(UVElem0, UVElem1, UVElem2));
	// 					}
    //                     UE_LOG(LogTemp, Warning, TEXT("Triangle: %d %d %d"), Idx0, Idx1, Idx2);
	// 					FDynamicMeshMaterialAttribute* MaterialAttr = Mesh.Attributes()->GetMaterialID();
	// 					if (MaterialAttr)
	// 					{
	// 						MaterialAttr->SetValue(TriID, CurrentMaterialSection);
	// 					}
	// 				} else {
	// 					UE_LOG(LogTemp, Warning, TEXT("Invalid triangle: %s %s %s"), *A[0], *B[0], *C[0]);
	// 				}
    //             };
	//             AddTriangle(V0, V1, V2);
    //         }
    //     }
    // }
	
	// int NumGroups = Mesh.MaxGroupID();
	// FMeshNormals NormalMesh(&Mesh);
	// TArray<int> NormalElementIDs;
	// NormalElementIDs.SetNum(Mesh.MaxVertexID());
	// FDynamicMeshNormalOverlay* NormalOverlay = Mesh.Attributes()->PrimaryNormals();
	// NormalMesh.ComputeTriangleNormals();
	// NormalMesh.ComputeVertexNormals(true, true);
	// NormalOverlay->ClearElements();
	// for (int vid : Mesh.VertexIndicesItr())
	// {
	// 	FVector3d N = NormalMesh[vid];
	// 	NormalElementIDs[vid] = NormalOverlay->AppendElement(FVector3f(N.X, N.Y, N.Z));
	// 	NormalOverlay->SetElement(vid, FVector3f(N.X, N.Y, N.Z));
	// }
	// for (int tid : Mesh.TriangleIndicesItr())
	// {
	// 	FIndex3i tri = Mesh.GetTriangle(tid);
	// 	FIndex3i elemTri(NormalElementIDs[tri.A], NormalElementIDs[tri.B], NormalElementIDs[tri.C]);
	// 	NormalOverlay->SetTriangle(tid, elemTri);
		
	// }
	// //UE_LOG(LogTemp, Warning, TEXT("Normals: %d"), NormalOverlay->MaxElementID());
	// // Upload mesh to component
	
    // DynMesh->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
    // {
    //     EditMesh = Mesh;
    // }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown);

	// DynMesh->SetNumMaterials(NumGroups);
	// UMaterialInterface* Placeholder = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Assets/Plants/procedural/main/ProceduralMeshMat.ProceduralMeshMat"));
	// for (int32 i = 0; i < NumGroups; ++i)
	// {
	// 	DynMesh->SetMaterial(i, Placeholder);
	// }
    // CompletedLoad = true;
}
void UDynamicPlantMesh::FinalizeMesh()
{
    using namespace UE::Geometry;
    int NumGroups = Mesh.MaxGroupID();
	FMeshNormals NormalMesh(&Mesh);
	TArray<int> NormalElementIDs;
	NormalElementIDs.SetNum(Mesh.MaxVertexID());
	FDynamicMeshNormalOverlay* NormalOverlay = Mesh.Attributes()->PrimaryNormals();
	NormalMesh.ComputeTriangleNormals();
	NormalMesh.ComputeVertexNormals(true, true);
	NormalOverlay->ClearElements();
	for (int vid : Mesh.VertexIndicesItr())
	{
		FVector3d N = NormalMesh[vid];
		NormalElementIDs[vid] = NormalOverlay->AppendElement(FVector3f(N.X, N.Y, N.Z));
		NormalOverlay->SetElement(vid, FVector3f(N.X, N.Y, N.Z));
	}
	for (int tid : Mesh.TriangleIndicesItr())
	{
		FIndex3i tri = Mesh.GetTriangle(tid);
		FIndex3i elemTri(NormalElementIDs[tri.A], NormalElementIDs[tri.B], NormalElementIDs[tri.C]);
		NormalOverlay->SetTriangle(tid, elemTri);
		
	}
	//UE_LOG(LogTemp, Warning, TEXT("Normals: %d"), NormalOverlay->MaxElementID());
	// Upload mesh to component
	
    DynMesh->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
    {
        EditMesh = Mesh;
    }, EDynamicMeshChangeType::GeneralEdit, EDynamicMeshAttributeChangeFlags::Unknown);
    DynMeshSeg->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
    {
        EditMesh = Mesh; // deep copy
    }, EDynamicMeshChangeType::GeneralEdit);

	DynMesh->SetNumMaterials(NumGroups);
    DynMeshSeg->SetNumMaterials(NumGroups);
	UMaterialInterface* Placeholder = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Assets/Plants/procedural/main/ProceduralMeshMat.ProceduralMeshMat"));
	for (int32 i = 0; i < NumGroups; ++i)
	{
		DynMesh->SetMaterial(i, Placeholder);
	}
    for (int32 i = 0; i < NumGroups; ++i)
	{
		DynMeshSeg->SetMaterial(i, Placeholder);
	}
    CompletedLoad = true;
}
bool UDynamicPlantMesh::AsyncConstructMesh(int NumOfLines)
{
    using namespace UE::Geometry;
    //FRenderCommandFence RenderFence;
    //RenderFence.BeginFence();
    if (NumOfLines <= 0) {NumOfLines = OBJLines.Num();}
    int32 LoadUntilLine = AsyncLoadLine + NumOfLines;
    //UE_LOG(LogTemp, Log, TEXT("Line %d %d"), AsyncLoadLine, NumOfLines);
    for (int32 lineIdx = AsyncLoadLine; lineIdx < LoadUntilLine; ++lineIdx)
    {
        
        // if (RenderFence.IsFenceComplete())
        // {
        //     // Render thread has finished everything up to the fence
        //     // You can exit the loop or continue
        //     AsyncLoadLine = lineIdx;
        //     return;
        // }
        if (lineIdx >= OBJLines.Num()) {return true;}
        const FString& Line = OBJLines[lineIdx];
        FString TrimmedLine = Line.TrimStartAndEnd();
        //UE_LOG(LogTemp, Log, TEXT("Line %d: %s"), lineIdx, *TrimmedLine);
        // Material change
        if (TrimmedLine.StartsWith(TEXT("usemtl ")))
        {
            OBJMaterialName.Add(TrimmedLine.RightChop(7));
            CurrentMaterialSection++;
			
        }

        // Vertex
        else if (TrimmedLine.StartsWith(TEXT("v ")))
        {
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
            {
                FVector3d V(
                    FCString::Atod(*Tokens[1]) * 100,
                    FCString::Atod(*Tokens[2]) * 100,
                    FCString::Atod(*Tokens[3]) * 100
                );
                Vertices.Add(V);
				int vid = Mesh.AppendVertex(V);
                //UE_LOG(LogTemp, Warning, TEXT("VID: %d %d"), vid, Vertices.Num());
				//UE_LOG(LogTemp, Warning, TEXT("VID: %d"), vid);
            } 
        	else
            {
	            UE_LOG(LogTemp, Warning, TEXT("Invalid v ID"));
            }
        }

        // UV
        else if (TrimmedLine.StartsWith(TEXT("vt ")))
        {
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 3)
            {
                UVs.Add(FVector2f(FCString::Atod(*Tokens[1]), FCString::Atod(*Tokens[2])));
            }
            else
            {
	            UE_LOG(LogTemp, Warning, TEXT("Invalid vt ID"));
            }
        }

        // Normal
        else if (TrimmedLine.StartsWith(TEXT("vn ")))
        {
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            if (Tokens.Num() >= 4)
            {
                Normals.Add(FVector3d(FCString::Atod(*Tokens[1]), FCString::Atod(*Tokens[2]), FCString::Atod(*Tokens[3])));
            }
            else
            {
	            UE_LOG(LogTemp, Warning, TEXT("Invalid vn ID"));
            }
        }

        // Face
        else if (TrimmedLine.StartsWith(TEXT("f ")))
        {
            TArray<FString> Tokens;
            TrimmedLine.ParseIntoArray(Tokens, TEXT(" "), true);
            for (int32 i = 1; i + 2 < Tokens.Num(); ++i)
            {
                TArray<FString> V0, V1, V2;
                Tokens[1].ParseIntoArray(V0, TEXT("/"));
                Tokens[i + 1].ParseIntoArray(V1, TEXT("/"));
                Tokens[i + 2].ParseIntoArray(V2, TEXT("/"));
				
                auto AddTriangle = [&](const TArray<FString>& A, const TArray<FString>& B, const TArray<FString>& C)
                {
					int32 Idx0 = FCString::Atoi(*A[0]) - 1;
					int32 Idx1 = FCString::Atoi(*B[0]) - 1;
					int32 Idx2 = FCString::Atoi(*C[0]) - 1;
                	if (Idx0 == Idx1 || Idx0 == Idx2 || Idx1 == Idx2)
                	{
                		UE_LOG(LogTemp, Error,
							TEXT("Degenerate face at line %d: %s"),
							lineIdx + 1,
							*Line);

                		return;
                	}
					if (Idx0 != Idx1 && Idx0 != Idx2 && Idx1 != Idx2) {
						int TriID = Mesh.AppendTriangle(FIndex3i(Idx0, Idx1, Idx2), CurrentMaterialSection);
                        if (TriID < 0)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Skipping triangle: already exists or invalid %d: %d %d %d"), TriID, Idx0, Idx1, Idx2);
                            UE_LOG(LogTemp, Warning, TEXT("%d %d"), Vertices.Num(), Mesh.MaxVertexID());

                            return;
                        }
						// UV overlay
						if (Mesh.Attributes() && Mesh.Attributes()->PrimaryUV() && A.Num() > 1 && !A[1].IsEmpty())
						{
							FDynamicMeshUVOverlay* UVOverlay = Mesh.Attributes()->PrimaryUV();
							int32 UVIndex0 = FCString::Atoi(*A[1]) - 1;
							int32 UVIndex1 = FCString::Atoi(*B[1]) - 1;
							int32 UVIndex2 = FCString::Atoi(*C[1]) - 1;
                            // UE_LOG(LogTemp, Warning, TEXT("ABCIdx: %d %d %d"), FCString::Atoi(*A[0]), FCString::Atoi(*B[0]), FCString::Atoi(*C[0]));
                            // UE_LOG(LogTemp, Warning, TEXT("Idx: %d %d %d"), Idx0, Idx1, Idx2);
                            // UE_LOG(LogTemp, Warning, TEXT("UVIndex: %d %d %d"), UVIndex0, UVIndex1, UVIndex2);

							int32 UVElem0 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex0) ? UVs[UVIndex0] : FVector2f(0,0));
							int32 UVElem1 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex1) ? UVs[UVIndex1] : FVector2f(0,0));
							int32 UVElem2 = UVOverlay->AppendElement(UVs.IsValidIndex(UVIndex2) ? UVs[UVIndex2] : FVector2f(0,0));

							// Associate UV elements with triangle
                        
                            //UE_LOG(LogTemp, Warning, TEXT("TriID: %d UV: %d %d %d"), TriID, UVElem0, UVElem1, UVElem2);
							UVOverlay->SetTriangle(TriID, FIndex3i(UVElem0, UVElem1, UVElem2));
						}
                        //UE_LOG(LogTemp, Warning, TEXT("Triangle: %d %d %d"), Idx0, Idx1, Idx2);
						FDynamicMeshMaterialAttribute* MaterialAttr = Mesh.Attributes()->GetMaterialID();
						if (MaterialAttr)
						{
							MaterialAttr->SetValue(TriID, CurrentMaterialSection);
						}
					} else {
						UE_LOG(LogTemp, Warning, TEXT("Invalid triangle: %s %s %s"), *A[0], *B[0], *C[0]);
					}
                };
	            AddTriangle(V0, V1, V2);
            }
        }
    }
    AsyncLoadLine = LoadUntilLine;
    return false;
}
void UDynamicPlantMesh::ConfigureMeshObj(const FString& OBJData, const FString& MTLData)
{
    using namespace UE::Geometry;
	if (!DynMesh || !DynMeshSeg)
    {
        UE_LOG(LogTemp, Warning, TEXT("DynMesh is null! Assign a DynamicMeshComponent first."));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("ConfigureMeshObj"));
    MTLMaterialName.Empty();
    MTLMaterialDef.Empty();
    OBJMaterialName.Empty();
    MTLLines.Empty();
    OBJLines.Empty();
    // Parse MTL
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

    // Clear previous mesh
    Mesh = FDynamicMesh3();
    Mesh.EnableAttributes(); // enables normals and UV overlays
	Mesh.EnableTriangleGroups();
	Mesh.Attributes()->EnableMaterialID();
	Mesh.EnableVertexNormals(FVector3f(0,0,0));
	
    Vertices.Empty();
    Normals.Empty();
    UVs.Empty();

    OBJData.ParseIntoArrayLines(OBJLines);

    CurrentMaterialSection = -1;
    AsyncLoadLine = 0;
    CompletedLoad = false;
}

float UDynamicPlantMesh::CurrentProgress()
{
    if (OBJLines.Num() == 0) return 0;
    return float(AsyncLoadLine) / OBJLines.Num();
}
