// Fill out your copyright notice in the Description page of Project Settings.


#include "PlacementToolInputBehavior.h"

#include "Tools/PlacementTool.h"

void UPlacementToolInputBehavior::Initialize(UPlacementTool* Tool)
{
	this->PlacementTool = Tool;
}

EInputDevices UPlacementToolInputBehavior::GetSupportedDevices()
{
	return EInputDevices::Keyboard | EInputDevices::Mouse;
}

bool UPlacementToolInputBehavior::IsPressed(const FInputDeviceState& Input)
{
	if (Input.IsFromDevice(EInputDevices::Keyboard))
	{
		ActiveDevice = EInputDevices::Keyboard;
		if(Input.Keyboard.ActiveKey.bDown)
		{
			if(Input.Keyboard.ActiveKey.Button == EKeys::E)
			{
				if(Input.bShiftKeyDown)
					PlacementTool->ChangeRotationOffset(UPlacementTool::EPlacementParameterChangeMode::IncreaseMinor);
				else
					PlacementTool->ChangeRotationOffset(UPlacementTool::EPlacementParameterChangeMode::IncreaseMajor);
				return true;
			}
			if(Input.Keyboard.ActiveKey.Button == EKeys::Q)
			{
				if(Input.bShiftKeyDown)
					PlacementTool->ChangeRotationOffset(UPlacementTool::EPlacementParameterChangeMode::DecreaseMinor);
				else
					PlacementTool->ChangeRotationOffset(UPlacementTool::EPlacementParameterChangeMode::DecreaseMajor);
				return true;
			}

			if(Input.Keyboard.ActiveKey.Button == EKeys::One)
			{
				PlacementTool->SetSnappingMode(ESnappingMode::Center);
				return true;
			}
			if(Input.Keyboard.ActiveKey.Button == EKeys::Two)
			{
				PlacementTool->SetSnappingMode(ESnappingMode::Edges);
				return true;
			}
			if(Input.Keyboard.ActiveKey.Button == EKeys::Three)
			{
				PlacementTool->SetSnappingMode(ESnappingMode::Corners);
				return true;
			}
			if(Input.Keyboard.ActiveKey.Button == EKeys::Four)
			{
				PlacementTool->SetSnappingMode(ESnappingMode::None);
				return true;
			}
			
			if(Input.Keyboard.ActiveKey.Button == EKeys::X)
			{
				PlacementTool->CycleRotationAxis();
				return true;
			}
		}
	}
	else if(Input.IsFromDevice(EInputDevices::Mouse))
	{
		ActiveDevice = EInputDevices::Mouse;
		if(Input.Mouse.WheelDelta > 0.0f)
		{
			if(Input.bShiftKeyDown)
				PlacementTool->ChangeHeightOffset(UPlacementTool::EPlacementParameterChangeMode::IncreaseMinor);
			else
				PlacementTool->ChangeHeightOffset(UPlacementTool::EPlacementParameterChangeMode::IncreaseMajor);
			return true;
		}
		else if(Input.Mouse.WheelDelta < 0.0f)
		{
			if(Input.bShiftKeyDown)
				PlacementTool->ChangeHeightOffset(UPlacementTool::EPlacementParameterChangeMode::DecreaseMinor);
			else
				PlacementTool->ChangeHeightOffset(UPlacementTool::EPlacementParameterChangeMode::DecreaseMajor);
			return true;
		}
	}
	
	return false;
}

FInputCaptureRequest UPlacementToolInputBehavior::WantsCapture(const FInputDeviceState& InputState)
{
	if (IsPressed(InputState))
	{
		return FInputCaptureRequest::Begin(this, EInputCaptureSide::Any, 0.f);
	}

	return FInputCaptureRequest::Ignore();
}

FInputCaptureUpdate UPlacementToolInputBehavior::BeginCapture(const FInputDeviceState& InputState,
	EInputCaptureSide eSide)
{
	return FInputCaptureUpdate::Begin(this, eSide);
}

FInputCaptureUpdate UPlacementToolInputBehavior::UpdateCapture(const FInputDeviceState& InputState,
	const FInputCaptureData& CaptureData)
{
	return FInputCaptureUpdate::End();
}
