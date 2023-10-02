// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "BaseBehaviors/MultiClickSequenceInputBehavior.h"
#include "PlacementToolInputBehavior.h"
#include "PlacementTool.generated.h"

/**
 * Builder for UPlacementTool
 */
UCLASS()
class GRIDPLACER_API UPlacementToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override { return true; }
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};

UENUM(BlueprintType)
enum class EPaletteObjectType : uint8
{
	StaticMesh,
	ActorClass
};

UCLASS()
class UPaletteObject : public UObject{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsActiveInPalette;
	void SetIsActiveInPalette(const bool IsActive = true){
		this->IsActiveInPalette = IsActive;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPaletteObjectType ObjectType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Asset;
};

DECLARE_DELEGATE(FOnActivePaletteChanged)

USTRUCT(BlueprintType)
struct FObjectPalette {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UPaletteObject*> ObjectsInPalette;

	void NotifyActivePaletteChanged()
	{
		OnActivePaletteChanged.Execute();
	}

	TArray<UPaletteObject*> GetActivePaletteObjects()
	{
		TArray<UPaletteObject*> ActivePaletteObjects;
		for(auto It : ObjectsInPalette)
			if(It->IsActiveInPalette)
				ActivePaletteObjects.Add(It);
		return ActivePaletteObjects;
	}

	void RemoveObjectFromPalette(UPaletteObject* Target)
	{
		ObjectsInPalette.Remove(Target);
		NotifyActivePaletteChanged();
	}

	void SelectAll()
	{
		for(auto It : ObjectsInPalette)
			It->SetIsActiveInPalette(true);
		NotifyActivePaletteChanged();
	}
	
	void DeselectAll()
	{
		for(auto It : ObjectsInPalette)
			It->SetIsActiveInPalette(false);
		NotifyActivePaletteChanged();
	}
	
	void Clear()
	{
		ObjectsInPalette.Empty();
		NotifyActivePaletteChanged();
	}

	FOnActivePaletteChanged OnActivePaletteChanged;
	
	FObjectPalette() {

	}
};

UENUM(BlueprintType)
enum class EPalettePickingMode : uint8
{
	Random,
	Cycle
};

UENUM(BlueprintType)
enum class EGridSpaceMode : uint8
{
	Global,
	Local,
	Custom
};

UENUM(BlueprintType)
enum class ESnappingMode : uint8
{
	Center,
	Edges,
	Corners,
	None
};

UENUM(BlueprintType)
enum class ERotationAxis : uint8
{
	X,
	Y,
	Z
};

UENUM(BlueprintType)
enum class ERotationPivotMode : uint8
{
	ObjectPivot,
	Grid
};

/**
 * Property set for the UPlacementTool
 */
UCLASS(Transient)
class GRIDPLACER_API UPlacementToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
	
public:
	UPlacementToolProperties();

	/*The palette contains all objects that you might want to place.
	 Tick the checkmark on an object to add it to the active pool.
	 Objects to be placed will be chosen from the active pool.
	 */
	UPROPERTY(EditAnywhere, Category = "Palette", meta = (TransientToolProperty))
	FObjectPalette ObjectPalette;
	/*Determines how objects will be picked from the active pool
	 * Random: After an object has been placed a random one will be chosen from the active pool
	 * Cycle: After an object has been placed the next one will be chosen from the active pool
	 */
	UPROPERTY(EditAnywhere, Category = "Palette")
	EPalettePickingMode PalettePickingMode;

	/*The width and height of a grid cell in centimeters*/
	UPROPERTY(EditAnywhere, Category = "Grid")
	FVector2D GridSize = FVector2D(100.0f, 100.0f);
	/*Determines the origin and orientation of the grid
	 * Global: Use GridOrigin and GridRotation to position the grid anywhere in the world
	 * Local: The grid will align itself to surfaces of objects the mouse cursor hovers above
	 * Custom: The grid origin and rotation will be determined by the transform of CustomGridSpaceTarget
	 */
	UPROPERTY(EditAnywhere, Category = "Grid|Space")
	EGridSpaceMode GridSpaceMode;
	/*Where should the grid plane originate*/
	UPROPERTY(EditAnywhere, Category = "Grid|Space", meta = (EditCondition = "GridSpaceMode == EGridSpaceMode::Global", EditConditionHides))
	FVector GridOrigin;
	/*How should the grid plane be rotated*/
	UPROPERTY(EditAnywhere, Category = "Grid|Space", meta = (EditCondition = "GridSpaceMode == EGridSpaceMode::Global", EditConditionHides))
	FRotator GridRotation;
	/*What collision channel to test against when finding a target for the local grid*/
	UPROPERTY(EditAnywhere, Category = "Grid|Space", meta = (EditCondition = "GridSpaceMode == EGridSpaceMode::Local", EditConditionHides))
	TEnumAsByte<ECollisionChannel> LocalGridCollisionChannel = ECC_Visibility;
	/*The transform to use as the grid origin and rotation*/
	UPROPERTY(EditAnywhere, Category = "Grid|Space", meta = (EditCondition = "GridSpaceMode == EGridSpaceMode::Custom", EditConditionHides, TransientToolProperty))
	AActor* CustomGridSpaceTarget;
	/*Determines what parts of the grid the cursor position will snap to
	 * (1) Center: Snap to the center of the closest grid cell
	 * (2) Edges: Snap to the closest edge of the closest grid cell
	 * (3) Corners: Snap to the closest corner of the closest grid cell
	 * (4) None: No Snapping - the cursor position will simply be projected onto the grid plane
	 */
	UPROPERTY(EditAnywhere, Category = "Grid")
	ESnappingMode SnappingMode;

	/*How far away from the surface of the grid should the object be placed*/
	UPROPERTY(EditAnywhere, Category = "Height Offset")
	float CurrentPlacementHeightOffset = 0.0f;
	/*The space the height offset will be applied in
	 * Global: The height offset will be applied in world space
	 * Local: The height offset will be applied in grid space
	 * Custom: The height offset will be applied relative to the transform of CustomHeightOffsetSpaceTarget
	 */
	UPROPERTY(EditAnywhere, Category = "Height Offset|Space")
	EGridSpaceMode CurrentHeightOffsetSpace = EGridSpaceMode::Local;
	/*The transform to apply the height offset relative to*/
	UPROPERTY(EditAnywhere, Category = "Height Offset|Space", meta = (EditCondition = "CurrentHeightOffsetSpace == EGridSpaceMode::Custom", EditConditionHides, TransientToolProperty))
	AActor* CustomHeightOffsetSpaceTarget;
	/*The axis of CurrentPlacementRotation that will be affected by the increment and randomization settings
	 * (X) Cycle Axis
	 */
	/*(Scroll) Change CurrentPlacementHeightOffset by a major increment*/
	UPROPERTY(EditAnywhere, Category = "Height Offset", meta = (ClampMin = "1.0", ClampMax = "1000.0", UIMin = "1.0", UIMax = "1000.0"))
	float HeightOffsetMajorIncrement = 100.0f;
	/*(Shift+Scroll) Change CurrentPlacementHeightOffset by a minor increment*/
	UPROPERTY(EditAnywhere, Category = "Height Offset", meta = (ClampMin = "0.1", ClampMax = "100.0", UIMin = "0.1", UIMax = "100.0"))
	float HeightOffsetMinorIncrement = 10.0f;
	/*Wether to randomize CurrentPlacementHeightOffset after each placement*/
	UPROPERTY(EditAnywhere, Category = "Height Offset|Randomization")
	bool RandomizeHeightOffset = false;
	/*The range of valid values for the random height offset*/
	UPROPERTY(EditAnywhere, Category = "Height Offset|Randomization", meta = (EditCondition = "RandomizeHeightOffset == true", EditConditionHides))
	FFloatInterval RandomHeightOffsetRange = FFloatInterval(-100.0f, 100.0f);
	/*How many divisions of the random height offset range to use
	 * Leave at 0 to allow any value from the random height offset range
	 */
	UPROPERTY(EditAnywhere, Category = "Height Offset|Randomization", meta = (EditCondition = "RandomizeHeightOffset == true", EditConditionHides))
	int HeightOffsetRandomizationDivisions = 0;
	
	/*The current rotation of the object around the grid planes normal*/
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
    FRotator CurrentPlacementRotation;
	/*The space the rotation will happen relative to
	 * Global: The rotation will be applied in world space
	 * Local: The rotation will be applied in grid space
	 * Custom: The rotation will be applied relative to the transform of CustomRotationSpaceTarget
	 */
	UPROPERTY(EditAnywhere, Category = "Rotation|Space")
	EGridSpaceMode CurrentRotationSpace = EGridSpaceMode::Local;
	/*The transform to rotate the object relative to*/
	UPROPERTY(EditAnywhere, Category = "Rotation|Space", meta = (EditCondition = "CurrentRotationSpace == EGridSpaceMode::Custom", EditConditionHides, TransientToolProperty))
	AActor* CustomRotationSpaceTarget;
	/*The axis of CurrentPlacementRotation that will be affected by the increment and randomization settings
	 * (X) Cycle Axis
	 */
	UPROPERTY(EditAnywhere, Category = "Rotation|Space")
	ERotationAxis CurrentRotationAxis = ERotationAxis::Z;
	/*What point should the object be rotated around
	 * ObjectPivot: Rotate around the objects pivot point
	 * Grid: Rotate around the snapped point on the grid
	 */
	UPROPERTY(EditAnywhere, Category = "Rotation|Space")
	ERotationPivotMode CurrentRotationPivotMode = ERotationPivotMode::ObjectPivot;
	/*(E/Q) Change CurrentPlacementRotation by a major increment*/
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float PlacementRotationMajorIncrement = 90.0f;
	/*(Shift+E/Shift+Q) Change CurrentPlacementRotation by a minor increment*/
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float PlacementRotationMinorIncrement = 45.0f;
	/*Wether to randomize CurrentPlacementRotation after each placement*/
	UPROPERTY(EditAnywhere, Category = "Rotation|Randomization")
	bool RandomizeRotation = false;
	/*The range of valid values for the random rotation range*/
	UPROPERTY(EditAnywhere, Category = "Rotation|Randomization", meta = (EditCondition = "RandomizeRotation == true", EditConditionHides))
	FFloatInterval RandomRotationRange = FFloatInterval(0.0f, 360.0f);
	/*How many divisions of the random rotation range to use
	 * Leave at 0 to allow any value from the random rotation range
	 */
	UPROPERTY(EditAnywhere, Category = "Rotation|Randomization", meta = (EditCondition = "RandomizeRotation == true", EditConditionHides))
	int RotationRandomizationDivisions = 0;

	/*The uniform scale of the object to place*/
	UPROPERTY(EditAnywhere, Category = "Scale", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0"))
	float CurrentPlacementScale = 1.0f;
	/*Wether to randomize the CurrentPlacementScale after each placement*/
	UPROPERTY(EditAnywhere, Category = "Scale|Randomization")
	bool RandomizeScale = false;
	/*The range of valid values for the random scale*/
	UPROPERTY(EditAnywhere, Category = "Scale|Randomization", meta = (EditCondition = "RandomizeScale == true", EditConditionHides))
	FFloatInterval RandomScaleRange = FFloatInterval(0.5f, 2.0f);
};

/**
 * 
 */
UCLASS()
class UPlacementTool : public UInteractiveTool, public IClickSequenceBehaviorTarget
{
	GENERATED_BODY()

public:
	virtual void SetWorld(UWorld* World);

	/** UInteractiveTool overrides */
	virtual void Setup() override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	
public:
	void SetSnappingMode(ESnappingMode SnappingMode) { Properties->SnappingMode = SnappingMode; }
	
	enum EPlacementParameterChangeMode
	{
		IncreaseMajor,
		DecreaseMajor,
		IncreaseMinor,
		DecreaseMinor
	};

	void ChangeHeightOffset(EPlacementParameterChangeMode ChangeMode);
	void RandomizeHeightOffset();

	void CycleRotationAxis();
	void ChangeRotationOffset(EPlacementParameterChangeMode ChangeMode);
	void RandomizeRotation();

	void RandomizeScale();
	
protected:
	/** Properties of the tool are stored here */
	UPROPERTY()
	TObjectPtr<UPlacementToolProperties> Properties;

protected:
	UWorld* TargetWorld = nullptr;

	uint32 CurrentPaletteCyclingIndex = 0;
	UPaletteObject* PreviewPaletteObject = nullptr;
	AActor* PreviewActor = nullptr;
	
	FVector RawPlacementPoint;
	FVector SnappedPlacementPoint;
	FVector CurrentGridCell;

	UFUNCTION()
	void OnActivePaletteChanged();
	UPaletteObject* PickNextObjectFromPalette();

	void UpdateGridSpace(const FRay& WorldRay);
	FVector WorldToGridSpace(FVector WorldSpace) const;
	FVector WorldToGridSpaceDirection(FVector WorldSpaceDirection) const;
	FVector GridToWorldSpace(FVector GridSpace) const;
	FVector GridToWorldSpaceDirection(FVector GridSpaceDirection) const;
	
	void UpdatePlacementPoint(const FRay& WorldRay);
	void SnapPlacementPointToGrid();
	FVector GetHeightOffsetVector();

	void DrawGrid(IToolsContextRenderAPI* RenderAPI);
	void DrawRotationAxis(IToolsContextRenderAPI* RenderAPI);

	AActor* SpawnPaletteObject(UPaletteObject* PaletteObject, const FName& Name);
	void SpawnPreviewActor(UPaletteObject* PaletteObject);
	void DestroyPreviewActor();
	void RealizePreview();

	TWeakObjectPtr<UPlacementToolInputBehavior> PlacementToolBehavior;
	
	// Inherited via IClickSequenceBehaviorTarget
	void OnBeginSequencePreview(const FInputDeviceRay& ClickPos) override;
	bool CanBeginClickSequence(const FInputDeviceRay& ClickPos) override;
	void OnBeginClickSequence(const FInputDeviceRay& ClickPos) override;
	bool OnNextSequenceClick(const FInputDeviceRay& ClickPos) override;
	void OnTerminateClickSequence() override;
	virtual bool RequestAbortClickSequence() override;
	bool ShouldClickSequenceTerminate = false;
	// target World we will raycast into
};