// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacementTool.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"

// for raycast into World
#include "CollisionQueryParams.h"
#include "PlacementToolInputBehavior.h"
#include "Engine/World.h"

#include "SceneManagement.h"
#include "Curves/CurveLinearColorAtlas.h"

#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

// localization namespace
#define LOCTEXT_NAMESPACE "UGridPlacerPlacementTool"

#pragma region Tool
void UPlacementTool::SetWorld(UWorld* World)
{
	check(World);
	this->TargetWorld = World;
}

void UPlacementTool::Setup()
{
	UInteractiveTool::Setup();

	// add default button input behaviors for devices
	UMultiClickSequenceInputBehavior* MouseBehavior = NewObject<UMultiClickSequenceInputBehavior>();
	MouseBehavior->Initialize(this);
	//MouseBehavior->Modifiers.RegisterModifier(IgnoreSnappingModifier, FInputDeviceState::IsShiftKeyDown);
	AddInputBehavior(MouseBehavior);

	// Create the property set and register it with the Tool
	Properties = NewObject<UPlacementToolProperties>(this, "Parameters");
	AddToolPropertySource(Properties);

	Properties->ObjectPalette.OnActivePaletteChanged.BindUFunction(this, FName("OnActivePaletteChanged"));
	
	PlacementToolBehavior = NewObject<UPlacementToolInputBehavior>(this);
	PlacementToolBehavior->Initialize(this);
	AddInputBehavior(PlacementToolBehavior.Get());

	Properties->RestoreProperties(this);
}

void UPlacementTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();

	//Grid
	DrawGrid(RenderAPI);
	//Rotation Axis
	DrawRotationAxis(RenderAPI);
	//Grid Origin
	PDI->DrawPoint(Properties->GridOrigin, FLinearColor::Red, 20.0f, SDPG_Foreground);
	//Raw Placement point
	PDI->DrawPoint(GridToWorldSpace(RawPlacementPoint), FLinearColor::Green, 10.0f, SDPG_Foreground);
	//Snapped placement point and height offset
	FVector HeightOffsetVector = GetHeightOffsetVector();
	PDI->DrawLine(GridToWorldSpace(SnappedPlacementPoint),
		GridToWorldSpace(SnappedPlacementPoint) + HeightOffsetVector,
		FLinearColor::Yellow, SDPG_Foreground, 2.0f);
	PDI->DrawPoint(GridToWorldSpace(SnappedPlacementPoint) + HeightOffsetVector,
		FLinearColor::Yellow, 20.0f, SDPG_Foreground);
}

void UPlacementTool::DrawGrid(IToolsContextRenderAPI* RenderAPI)
{
	FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();

	FVector GridPoint00 = GridToWorldSpace(CurrentGridCell);
	FVector GridPoint10 = GridToWorldSpace(CurrentGridCell + FVector::ForwardVector * Properties->GridSize.X);
	FVector GridPoint01 = GridToWorldSpace(CurrentGridCell + FVector::RightVector * Properties->GridSize.Y);
	FVector GridPoint11 = GridToWorldSpace(CurrentGridCell + FVector::ForwardVector * Properties->GridSize.X + FVector::RightVector * Properties->GridSize.Y);
	PDI->DrawLine(GridPoint00, GridPoint01, FLinearColor::Gray, SDPG_Foreground, 2.0f);
	PDI->DrawLine(GridPoint00, GridPoint10, FLinearColor::Gray, SDPG_Foreground, 2.0f);
	PDI->DrawLine(GridPoint11, GridPoint01, FLinearColor::Gray, SDPG_Foreground, 2.0f);
	PDI->DrawLine(GridPoint11, GridPoint10, FLinearColor::Gray, SDPG_Foreground, 2.0f);

	switch (Properties->SnappingMode)
	{
		case ESnappingMode::Center:
			PDI->DrawPoint(((GridPoint00 + GridPoint01) / 2.0f + (GridPoint10 + GridPoint11) / 2.0f) / 2.0f, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			break;
		case ESnappingMode::Edges:
			PDI->DrawPoint((GridPoint00 + GridPoint01) / 2.0f, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint((GridPoint00 + GridPoint10) / 2.0f, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint((GridPoint11 + GridPoint01) / 2.0f, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint((GridPoint11 + GridPoint10) / 2.0f, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			break;
		case ESnappingMode::Corners:
			PDI->DrawPoint(GridPoint00, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(GridPoint10, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(GridPoint01, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			PDI->DrawPoint(GridPoint11, FLinearColor::Yellow, 10.0f, SDPG_Foreground);
			break;
	}
}

void UPlacementTool::DrawRotationAxis(IToolsContextRenderAPI* RenderAPI)
{
	//Create world space axis at origin
	FVector AxisStart;
	FVector AxisEnd;
	const float HalfAxisLength = (FMath::Max(Properties->GridSize.X,Properties->GridSize.Y) + 50.0f) * 0.5f;
	switch(Properties->CurrentRotationAxis)
	{
		case ERotationAxis::X:
			AxisStart = FVector::ForwardVector * -HalfAxisLength;
			AxisEnd = FVector::ForwardVector * HalfAxisLength;
			break;
		case ERotationAxis::Y:
			AxisStart = FVector::RightVector * -HalfAxisLength;
			AxisEnd = FVector::RightVector * HalfAxisLength;
			break;
		case ERotationAxis::Z:
			AxisStart = FVector::UpVector * -HalfAxisLength;
			AxisEnd = FVector::UpVector * HalfAxisLength;
			break;
	}

	//Transform to current rotation space
	switch(Properties->CurrentRotationSpace)
	{
		case EGridSpaceMode::Global:
			AxisStart = GridToWorldSpace(SnappedPlacementPoint) + AxisStart;
			AxisEnd = GridToWorldSpace(SnappedPlacementPoint) + AxisEnd;
			break;
		case EGridSpaceMode::Local:
			AxisStart = GridToWorldSpace(SnappedPlacementPoint + AxisStart);
			AxisEnd = GridToWorldSpace(SnappedPlacementPoint + AxisEnd);
			break;
		case EGridSpaceMode::Custom:
			if(Properties->CustomRotationSpaceTarget)
			{
				const FQuat TargetRotation = Properties->CustomRotationSpaceTarget->GetActorRotation().Quaternion();
				AxisStart = GridToWorldSpace(SnappedPlacementPoint) + TargetRotation * AxisStart;
				AxisEnd = GridToWorldSpace(SnappedPlacementPoint) + TargetRotation * AxisEnd;
			}
			else
			{
				AxisStart = GridToWorldSpace(SnappedPlacementPoint) + AxisStart;
				AxisEnd = GridToWorldSpace(SnappedPlacementPoint) + AxisEnd;
			}
			break;
	}
	switch(Properties->CurrentRotationPivotMode)
	{
	case ERotationPivotMode::ObjectPivot:
		AxisStart += GetHeightOffsetVector();
		AxisEnd += GetHeightOffsetVector();
		break;
	case ERotationPivotMode::Grid:
		break;
	}
	
	//Draw the axis
	FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();
	switch(Properties->CurrentRotationAxis)
	{
	case ERotationAxis::X:
		PDI->DrawLine(AxisStart, AxisEnd, FLinearColor::Red, SDPG_Foreground, 3.0f);
		break;
	case ERotationAxis::Y:
		PDI->DrawLine(AxisStart, AxisEnd, FLinearColor::Green, SDPG_Foreground, 3.0f);
		break;
	case ERotationAxis::Z:
		PDI->DrawLine(AxisStart, AxisEnd, FLinearColor::Blue, SDPG_Foreground, 3.0f);
		break;
	}
}

AActor* UPlacementTool::SpawnPaletteObject(UPaletteObject* PaletteObject, const FName& Name)
{
	FActorSpawnParameters PreviewSpawnParams;
	PreviewSpawnParams.Name = MakeUniqueObjectName(nullptr, AActor::StaticClass(), Name);
	PreviewSpawnParams.bTemporaryEditorActor = true;
	if(PaletteObject->ObjectType == EPaletteObjectType::StaticMesh)
	{
		AStaticMeshActor* Preview = TargetWorld->SpawnActor<AStaticMeshActor>(PreviewSpawnParams);
		Preview->GetStaticMeshComponent()->SetStaticMesh(PaletteObject->StaticMesh);
		Preview->GetStaticMeshComponent()->SetCanEverAffectNavigation(false);
		return Preview;
	}
	if(PaletteObject->ObjectType == EPaletteObjectType::ActorClass)
	{
		return TargetWorld->SpawnActor(PaletteObject->ActorClass, 0, 0, PreviewSpawnParams);
	}
	return nullptr;
}

void UPlacementTool::SpawnPreviewActor(UPaletteObject* PaletteObject)
{
	DestroyPreviewActor();

	PreviewPaletteObject = PaletteObject;
	
	PreviewActor = SpawnPaletteObject(PaletteObject, FName("GridPlacerPreview"));
}

void UPlacementTool::DestroyPreviewActor()
{
	if(PreviewActor)
	{
		TargetWorld->DestroyActor(PreviewActor);
		PreviewPaletteObject = nullptr;
	}
}

void UPlacementTool::RealizePreview()
{
	if(!PreviewActor || !PreviewPaletteObject)
		return;

	if(GEditor) GEditor->BeginTransaction(FText::FromString("Place Object"));

	FName SpawnedObjectName;
	if(PreviewPaletteObject->ObjectType == EPaletteObjectType::StaticMesh)
		SpawnedObjectName = FName(PreviewPaletteObject->StaticMesh->GetName());
	else if(PreviewPaletteObject->ObjectType == EPaletteObjectType::ActorClass)
		SpawnedObjectName = FName(PreviewPaletteObject->ActorClass->GetClass()->GetName());
	AActor* SpawnedObject = SpawnPaletteObject(PreviewPaletteObject, SpawnedObjectName);
	SpawnedObject->SetActorLocation(PreviewActor->GetActorLocation());
	SpawnedObject->SetActorRotation(PreviewActor->GetActorRotation());
	SpawnedObject->SetActorScale3D(PreviewActor->GetActorScale3D());

	if(GEditor) GEditor->EndTransaction();
}

void UPlacementTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
}

void UPlacementTool::Shutdown(EToolShutdownType ShutdownType){
	Super::Shutdown(ShutdownType);
	
	DestroyPreviewActor();

	Properties->SaveProperties(this);
}

void UPlacementTool::ChangeHeightOffset(EPlacementParameterChangeMode ChangeMode)
{
	switch(ChangeMode)
	{
		case IncreaseMajor: Properties->CurrentPlacementHeightOffset += Properties->HeightOffsetMajorIncrement; break;
		case DecreaseMajor: Properties->CurrentPlacementHeightOffset -= Properties->HeightOffsetMajorIncrement; break;
		case IncreaseMinor: Properties->CurrentPlacementHeightOffset += Properties->HeightOffsetMinorIncrement; break;
		case DecreaseMinor: Properties->CurrentPlacementHeightOffset -= Properties->HeightOffsetMinorIncrement; break;
	}
}

void UPlacementTool::RandomizeHeightOffset()
{
	if(FMath::Abs(Properties->HeightOffsetRandomizationDivisions) > 0)
	{
		Properties->CurrentPlacementHeightOffset = Properties->RandomHeightOffsetRange.Min + (Properties->RandomHeightOffsetRange.Size() / Properties->HeightOffsetRandomizationDivisions) * FMath::RandRange(0, FMath::Abs(Properties->HeightOffsetRandomizationDivisions));
	}
	else
		Properties->CurrentPlacementHeightOffset = FMath::RandRange(Properties->RandomHeightOffsetRange.Min, Properties->RandomHeightOffsetRange.Max);
}

void UPlacementTool::CycleRotationAxis()
{
	Properties->CurrentRotationAxis = static_cast<ERotationAxis>((static_cast<uint8>(Properties->CurrentRotationAxis) + 1) % 3);
}

void UPlacementTool::ChangeRotationOffset(EPlacementParameterChangeMode ChangeMode)
{
	float CurrentRotationOnAxis = 0.0f;
	switch(Properties->CurrentRotationAxis)
	{
		case ERotationAxis::X: CurrentRotationOnAxis = Properties->CurrentPlacementRotation.Roll; break;
		case ERotationAxis::Y: CurrentRotationOnAxis = Properties->CurrentPlacementRotation.Pitch; break;
		case ERotationAxis::Z: CurrentRotationOnAxis = Properties->CurrentPlacementRotation.Yaw; break;
	}
	switch(ChangeMode)
	{
		case IncreaseMajor: CurrentRotationOnAxis += Properties->PlacementRotationMajorIncrement; break;
		case DecreaseMajor: CurrentRotationOnAxis -= Properties->PlacementRotationMajorIncrement; break;
		case IncreaseMinor: CurrentRotationOnAxis += Properties->PlacementRotationMinorIncrement; break;
		case DecreaseMinor: CurrentRotationOnAxis -= Properties->PlacementRotationMinorIncrement; break;
	}
	if(CurrentRotationOnAxis >= 360.0f)
		CurrentRotationOnAxis -= 360.0f;
	else if(CurrentRotationOnAxis < 0.0f)
		CurrentRotationOnAxis += 360.0f;
	switch(Properties->CurrentRotationAxis)
	{
		case ERotationAxis::X: Properties->CurrentPlacementRotation.Roll = CurrentRotationOnAxis; break;
		case ERotationAxis::Y: Properties->CurrentPlacementRotation.Pitch = CurrentRotationOnAxis; break;
		case ERotationAxis::Z: Properties->CurrentPlacementRotation.Yaw = CurrentRotationOnAxis; break;
	}
}

void UPlacementTool::RandomizeRotation()
{
	float RandomRotation = 0.0f;
	if(FMath::Abs(Properties->RotationRandomizationDivisions) > 0)
	{
		RandomRotation = Properties->RandomRotationRange.Min + (Properties->RandomRotationRange.Size() / Properties->RotationRandomizationDivisions) * FMath::RandRange(0, FMath::Abs(Properties->RotationRandomizationDivisions) - 1);
	}
	else
		RandomRotation = FMath::RandRange(Properties->RandomRotationRange.Min, Properties->RandomRotationRange.Max);
	switch(Properties->CurrentRotationAxis)
	{
		case ERotationAxis::X: Properties->CurrentPlacementRotation.Roll = RandomRotation; break;
		case ERotationAxis::Y: Properties->CurrentPlacementRotation.Pitch = RandomRotation; break;
		case ERotationAxis::Z: Properties->CurrentPlacementRotation.Yaw = RandomRotation; break;
	}
}

void UPlacementTool::RandomizeScale()
{
	Properties->CurrentPlacementScale = FMath::RandRange(Properties->RandomScaleRange.Min, Properties->RandomScaleRange.Max);
}

void UPlacementTool::OnActivePaletteChanged()
{
	UPaletteObject* NextChosenObject = PickNextObjectFromPalette();
	if(NextChosenObject)
		SpawnPreviewActor(NextChosenObject);
	else
		DestroyPreviewActor();
}

UPaletteObject* UPlacementTool::PickNextObjectFromPalette()
{
	auto ActivePaletteObjects = Properties->ObjectPalette.GetActivePaletteObjects();
	if(ActivePaletteObjects.Num() == 0)
		return nullptr;
	switch(Properties->PalettePickingMode)
	{
	case EPalettePickingMode::Random: return ActivePaletteObjects[FMath::RandRange(0, ActivePaletteObjects.Num() - 1)];
	case EPalettePickingMode::Cycle:
		//Make sure the index is in bounds
		CurrentPaletteCyclingIndex %= ActivePaletteObjects.Num();
		UPaletteObject* ChosenObject = ActivePaletteObjects[CurrentPaletteCyclingIndex++];
		//Make sure the incremented index is still in bounds
		CurrentPaletteCyclingIndex %= ActivePaletteObjects.Num();
		return ChosenObject;
	}
	return nullptr;
}

void UPlacementTool::UpdateGridSpace(const FRay& WorldRay)
{
	FVector TraceStart;
	FVector TraceEnd;
	FCollisionQueryParams QueryParams;
	FHitResult Hit;
	
	switch(Properties->GridSpaceMode)
	{
	case EGridSpaceMode::Global:
		break;
	case EGridSpaceMode::Local:
		TraceStart = WorldRay.Origin;
		TraceEnd = TraceStart + WorldRay.Direction * 100000;
		if(PreviewActor)
			QueryParams.AddIgnoredActor(PreviewActor);
		if(TargetWorld->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, Properties->LocalGridCollisionChannel, QueryParams)){
			if(AActor* LocalTarget = Hit.GetActor())
			{
				//Origin
				Properties->GridOrigin = FVector::PointPlaneProject(LocalTarget->GetActorLocation(), Hit.ImpactPoint, Hit.ImpactNormal);
				//Rotation
				FVector HitTangent;
				if(FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector) > 0.9f)
					HitTangent = FVector::CrossProduct(Hit.ImpactNormal, FVector::RightVector);
				else
					HitTangent = FVector::CrossProduct(Hit.ImpactNormal, FVector::UpVector);
				Hit.ImpactNormal = LocalTarget->GetActorRotation().Quaternion().Inverse() * Hit.ImpactNormal;
				HitTangent = LocalTarget->GetActorRotation().Quaternion().Inverse() * HitTangent;
				Properties->GridRotation = UKismetMathLibrary::MakeRotFromXZ(HitTangent, Hit.ImpactNormal);
				Properties->GridRotation += LocalTarget->GetActorRotation();
			}
		}
		break;
	case EGridSpaceMode::Custom:
		if(Properties->CustomGridSpaceTarget)
		{
			Properties->GridOrigin = Properties->CustomGridSpaceTarget->GetActorLocation();
			Properties->GridRotation = Properties->CustomGridSpaceTarget->GetActorRotation();
		}
		break;
	}
}

FVector UPlacementTool::WorldToGridSpace(FVector WorldSpace) const
{
	WorldSpace -= Properties->GridOrigin;
	WorldSpace = Properties->GridRotation.Quaternion().Inverse() * WorldSpace;
	return WorldSpace;
}

FVector UPlacementTool::WorldToGridSpaceDirection(FVector WorldSpaceDirection) const
{
	WorldSpaceDirection = Properties->GridRotation.Quaternion().Inverse() * WorldSpaceDirection;
	return WorldSpaceDirection;
}

FVector UPlacementTool::GridToWorldSpace(FVector GridSpace) const
{
	GridSpace = Properties->GridRotation.Quaternion() * GridSpace;
	GridSpace += Properties->GridOrigin;
	return GridSpace;
}

FVector UPlacementTool::GridToWorldSpaceDirection(FVector GridSpaceDirection) const
{
	GridSpaceDirection = Properties->GridRotation.Quaternion() * GridSpaceDirection;
	return GridSpaceDirection;
}

void UPlacementTool::UpdatePlacementPoint(const FRay& WorldRay)
{
	//Find Ray-Grid-Intersection
	FRay GridSpaceRay;
	GridSpaceRay.Origin = WorldToGridSpace(WorldRay.Origin);
	GridSpaceRay.Direction = WorldToGridSpaceDirection(WorldRay.Direction);
	if(GridSpaceRay.Direction.Z == 0.0f)
		return;
	FVector RayGridIntersection = GridSpaceRay.Origin - GridSpaceRay.Direction * (GridSpaceRay.Origin.Z / GridSpaceRay.Direction.Z);
	RawPlacementPoint = RayGridIntersection;
	//Snap RawPlacementPoint to grid
	SnapPlacementPointToGrid();
	CurrentGridCell = RawPlacementPoint;
	CurrentGridCell.X = FMath::Floor(CurrentGridCell.X / Properties->GridSize.X) * Properties->GridSize.X;
	CurrentGridCell.Y = FMath::Floor(CurrentGridCell.Y / Properties->GridSize.Y) * Properties->GridSize.Y;
}

void UPlacementTool::SnapPlacementPointToGrid()
{
	SnappedPlacementPoint = RawPlacementPoint;
	FVector CellCenter = SnappedPlacementPoint;
	TArray<FVector> EdgePositions;
	int ClosestEdgeIdx = 0;
	float ClosestEdgeDistance = Properties->GridSize.X + Properties->GridSize.Y;
	switch(Properties->SnappingMode)
	{
		case ESnappingMode::Center:
			SnappedPlacementPoint.X = (FMath::RoundToFloat(SnappedPlacementPoint.X / Properties->GridSize.X + 0.5f) - 0.5f) * Properties->GridSize.X;
			SnappedPlacementPoint.Y = (FMath::RoundToFloat(SnappedPlacementPoint.Y / Properties->GridSize.Y + 0.5f) - 0.5f) * Properties->GridSize.Y;
			break;
		case ESnappingMode::Edges:
			CellCenter.X = (FMath::RoundToFloat(SnappedPlacementPoint.X / Properties->GridSize.X + 0.5f) - 0.5f) * Properties->GridSize.X;
			CellCenter.Y = (FMath::RoundToFloat(SnappedPlacementPoint.Y / Properties->GridSize.Y + 0.5f) - 0.5f) * Properties->GridSize.Y;
			EdgePositions.Add(CellCenter + FVector::ForwardVector * Properties->GridSize.X * 0.5f);
			EdgePositions.Add(CellCenter - FVector::ForwardVector * Properties->GridSize.X * 0.5f);
			EdgePositions.Add(CellCenter + FVector::RightVector * Properties->GridSize.Y * 0.5f);
			EdgePositions.Add(CellCenter - FVector::RightVector * Properties->GridSize.Y * 0.5f);
			for (int i = 0; i < 4; ++i)
			{
				float CurrentEdgeDistance = FVector::Dist(RawPlacementPoint, EdgePositions[i]);
				if(CurrentEdgeDistance < ClosestEdgeDistance)
				{
					ClosestEdgeDistance = CurrentEdgeDistance;
					ClosestEdgeIdx = i;
				}
			}
			SnappedPlacementPoint = EdgePositions[ClosestEdgeIdx];
			break;
		case ESnappingMode::Corners:
			SnappedPlacementPoint.X = FMath::RoundToFloat(SnappedPlacementPoint.X / Properties->GridSize.X) * Properties->GridSize.X;
			SnappedPlacementPoint.Y = FMath::RoundToFloat(SnappedPlacementPoint.Y / Properties->GridSize.Y) * Properties->GridSize.Y;
			break;
		case ESnappingMode::None: break;
	}
}

FVector UPlacementTool::GetHeightOffsetVector()
{
	switch(Properties->CurrentHeightOffsetSpace)
	{
	case EGridSpaceMode::Global: return FVector::UpVector * Properties->CurrentPlacementHeightOffset;
	case EGridSpaceMode::Local: return GridToWorldSpaceDirection(FVector::UpVector * Properties->CurrentPlacementHeightOffset);
	case EGridSpaceMode::Custom:
		if(Properties->CustomHeightOffsetSpaceTarget)
			return Properties->CustomHeightOffsetSpaceTarget->GetActorUpVector() * Properties->CurrentPlacementHeightOffset;
		return FVector::UpVector * Properties->CurrentPlacementHeightOffset;
	default: return GridToWorldSpaceDirection(FVector::UpVector * Properties->CurrentPlacementHeightOffset);
	}
}

void UPlacementTool::OnBeginSequencePreview(const FInputDeviceRay& ClickPos)
{
	UpdateGridSpace(ClickPos.WorldRay);
	UpdatePlacementPoint(ClickPos.WorldRay);
	if(PreviewActor){
		//Set Location
		PreviewActor->SetActorLocation(GridToWorldSpace(SnappedPlacementPoint) + GetHeightOffsetVector());
		//Set Rotation
		switch(Properties->CurrentRotationSpace)
		{
		case EGridSpaceMode::Global:
			PreviewActor->SetActorRotation(Properties->CurrentPlacementRotation);
			break;
		case EGridSpaceMode::Local:
			PreviewActor->SetActorRotation(Properties->GridRotation);
			PreviewActor->AddActorLocalRotation(Properties->CurrentPlacementRotation);
			break;
		case EGridSpaceMode::Custom:
			if(Properties->CustomRotationSpaceTarget)
			{
				PreviewActor->SetActorRotation(Properties->CustomRotationSpaceTarget->GetActorRotation());
				PreviewActor->AddActorLocalRotation(Properties->CurrentPlacementRotation);
			}
			else
			{
				PreviewActor->SetActorRotation(Properties->CurrentPlacementRotation);
			}
			break;
		}
		//Account for rotation pivot
		switch(Properties->CurrentRotationPivotMode)
		{
			case ERotationPivotMode::ObjectPivot:
				break;
		case ERotationPivotMode::Grid:
				PreviewActor->SetActorLocation(GridToWorldSpace(SnappedPlacementPoint) + PreviewActor->GetActorRotation().Quaternion() * FVector::UpVector * Properties->CurrentPlacementHeightOffset);
				break;
		}
		//Set Scale
		PreviewActor->SetActorScale3D(FVector::OneVector * Properties->CurrentPlacementScale);
	}
}
bool UPlacementTool::CanBeginClickSequence(const FInputDeviceRay& ClickPos)
{
	return true;
}
void UPlacementTool::OnBeginClickSequence(const FInputDeviceRay& ClickPos)
{
	//Realize preview and pick new random object to preview from the active palette
	RealizePreview();
	UPaletteObject* NextChosenObject = PickNextObjectFromPalette();
	if(NextChosenObject){
		SpawnPreviewActor(NextChosenObject);
		if(Properties->RandomizeHeightOffset)
			RandomizeHeightOffset();
		if(Properties->RandomizeRotation)
			RandomizeRotation();
		if(Properties->RandomizeScale)
			RandomizeScale();
	}
	ShouldClickSequenceTerminate = true;
}
bool UPlacementTool::OnNextSequenceClick(const FInputDeviceRay& ClickPos)
{
	return false;
}
void UPlacementTool::OnTerminateClickSequence()
{
	ShouldClickSequenceTerminate = false;
}

bool UPlacementTool::RequestAbortClickSequence()
{
	return ShouldClickSequenceTerminate;
}
#pragma endregion

#pragma region Properties
UPlacementToolProperties::UPlacementToolProperties()
{
	GridSize.X = 100.0f;
	GridSize.Y = 100.0f;
	GridOrigin = FVector::ZeroVector;
	GridRotation = FRotator::ZeroRotator;
	SnappingMode = ESnappingMode::Center;
}
#pragma endregion

#pragma region Builder
UInteractiveTool* UPlacementToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UPlacementTool* NewTool = NewObject<UPlacementTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}
#pragma endregion

#undef LOCTEXT_NAMESPACE