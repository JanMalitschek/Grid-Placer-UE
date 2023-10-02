// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseBehaviors/AnyButtonInputBehavior.h"
#include "PlacementToolInputBehavior.generated.h"

/**
 * 
 */
UCLASS()
class GRIDPLACER_API UPlacementToolInputBehavior : public UAnyButtonInputBehavior
{
	GENERATED_BODY()

public:
	void Initialize(class UPlacementTool* PlacementTool);
	
	virtual EInputDevices GetSupportedDevices() override;
	virtual bool IsPressed(const FInputDeviceState& input) override;
	virtual FInputCaptureRequest WantsCapture(const FInputDeviceState& InputState) override;
	virtual FInputCaptureUpdate BeginCapture(const FInputDeviceState& InputState, EInputCaptureSide eSide) override;
	virtual FInputCaptureUpdate UpdateCapture(const FInputDeviceState& InputState, const FInputCaptureData& CaptureData) override;

private:
	class UPlacementTool* PlacementTool;
};
