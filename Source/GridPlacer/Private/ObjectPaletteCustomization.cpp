// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPaletteCustomization.h"

//Include Slate
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboBox.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "PropertyHandle.h"
#include "SlateBasics.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "SAssetDropTarget.h"
#include "Editor/UnrealEd/Public/AssetThumbnail.h"
#include "Tools/PlacementTool.h"

void ObjectPaletteCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyHandle = StructPropertyHandle;
	PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, & ObjectPaletteCustomization::OnPaletteChanged, PropertyHandle, StructCustomizationUtils.GetPropertyUtilities()));

	ThumbnailPool = MakeShared<FAssetThumbnailPool>(64);
	
	HeaderRow.NameContent()[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void ObjectPaletteCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FObjectPalette* ObjectPalette = GetData();
	
	StructBuilder.AddCustomRow(FText::FromString("Palette Control"))
	.WholeRowContent()[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Text(FText::FromString("Clear Palette"))
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this, ObjectPalette]
			{
				ObjectPalette->Clear();
				this->PropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayClear);
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Text(FText::FromString("Select All"))
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this, ObjectPalette]
			{
				ObjectPalette->SelectAll();
				this->PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Text(FText::FromString("Deselect All"))
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([this, ObjectPalette]
			{
				ObjectPalette->DeselectAll();
				this->PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				return FReply::Handled();
			})
		]
	];
	TSharedPtr<SWrapBox> WrapBox;
	StructBuilder.AddCustomRow(FText::FromString("Drop Assets Row"))
	.WholeRowContent()[
		SNew( SAssetDropTarget )
		.OnAreAssetsAcceptableForDrop( this, &ObjectPaletteCustomization::OnIsAssetAcceptableForDrop )
		.OnAssetsDropped( this, &ObjectPaletteCustomization::OnAssetsDropped )
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.5f)))
				.Text(FText::FromString("Drop Assets Here\nStatic Mesh or Actor Blueprint"))
			]
			+ SScrollBox::Slot()
			[
				SAssignNew(WrapBox, SWrapBox)
				.InnerSlotPadding(FVector2D(5.0f, 5.0f))
				.UseAllottedSize(true)
				.UseAllottedWidth(true)
			]
		]
	];
	
	const TSharedPtr<IPropertyHandleArray> PaletteArrayHandle = PropertyHandle->GetChildHandle(FName("ObjectsInPalette"))->AsArray();
	uint32 NumObjects;
	PaletteArrayHandle->GetNumElements(NumObjects);
	for(uint32 i = 0; i < NumObjects; i++)
	{
		TSharedPtr<IPropertyHandle> ElementHandle = PaletteArrayHandle->GetElement(i);
		if(ElementHandle->IsValidHandle())
		{
			UObject* ElementObject;
			FPropertyAccess::Result Result = ElementHandle->GetValue(ElementObject);
			if(Result == FPropertyAccess::Success)
			{
				if(UPaletteObject* PaletteObject = Cast<UPaletteObject>(ElementObject))
				{
					FAssetThumbnailConfig ThumbnailConfig;
					ThumbnailConfig.Padding = FMargin(0.0f);
					TSharedPtr<FAssetThumbnail> Thumbnail = MakeShareable(new FAssetThumbnail(PaletteObject->Asset, 70, 70, ThumbnailPool));
					TSharedRef<SWidget> ThumbnailWidget = Thumbnail->MakeThumbnailWidget(ThumbnailConfig);
					WrapBox->AddSlot()
						[
						SNew(SOverlay)
						+ SOverlay::Slot()[
							ThumbnailWidget
						]
						+ SOverlay::Slot()
						.VAlign(EVerticalAlignment::VAlign_Top)
						.HAlign(EHorizontalAlignment::HAlign_Left)
						[
							SNew(SCheckBox)
							.IsChecked(PaletteObject->IsActiveInPalette ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
							.OnCheckStateChanged_Lambda([ObjectPalette, PaletteObject](ECheckBoxState State)
							{
								if(State == ECheckBoxState::Checked)
									PaletteObject->SetIsActiveInPalette(true);
								else if(State == ECheckBoxState::Unchecked)
									PaletteObject->SetIsActiveInPalette(false);
								ObjectPalette->NotifyActivePaletteChanged();
							})
						]
						+ SOverlay::Slot()
						.VAlign(EVerticalAlignment::VAlign_Top)
						.HAlign(EHorizontalAlignment::HAlign_Right)
						[
							SNew(SButton)
							.ContentPadding(0.0f)
							.Text(FText::FromString("X"))
							.OnClicked_Lambda([this, ObjectPalette, PaletteObject]
							{
								ObjectPalette->RemoveObjectFromPalette(PaletteObject);
								PropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayRemove);
								return FReply::Handled();
							})
						]
					];
				}
			}
		}
	}
}

bool ObjectPaletteCustomization::OnIsAssetAcceptableForDrop(TArrayView<FAssetData> DraggedAssets)
{
	return true;
}

void ObjectPaletteCustomization::OnAssetsDropped(const FDragDropEvent& DragDropEvent,
	TArrayView<FAssetData> DraggedAssets)
{
	TArray<void*> RawData;
	PropertyHandle->AccessRawData(RawData);
	ensure(RawData.Num() == 1);
	TArray<UPaletteObject*>* Ref = reinterpret_cast<TArray<UPaletteObject*>*>(RawData[0]);
	for(auto It : DraggedAssets)
	{
		if(UStaticMesh* StaticMesh = Cast<UStaticMesh>(It.GetAsset())){
			UPaletteObject* NewPaletteObject = NewObject<UPaletteObject>();
			NewPaletteObject->StaticMesh = StaticMesh;
			NewPaletteObject->ObjectType = EPaletteObjectType::StaticMesh;
			NewPaletteObject->Asset = It.GetAsset();
			Ref->Add(NewPaletteObject);
			PropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
		}
		else if(UBlueprint* Blueprint = Cast<UBlueprint>(It.GetAsset())){
			if(!Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
				continue;
			UPaletteObject* NewPaletteObject = NewObject<UPaletteObject>();
			NewPaletteObject->ActorClass = Blueprint->GeneratedClass;
			NewPaletteObject->ObjectType = EPaletteObjectType::ActorClass;
			NewPaletteObject->Asset = It.GetAsset();
			Ref->Add(NewPaletteObject);
			PropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
		}
	}
}

void ObjectPaletteCustomization::OnPaletteChanged(TSharedPtr<IPropertyHandle> Handle,
	TSharedPtr<IPropertyUtilities> Utils)
{
	if(Utils)
	{
		Utils->ForceRefresh();
	}
}

FObjectPalette* ObjectPaletteCustomization::GetData()
{
	TArray<UObject*> objects;
	PropertyHandle->GetOuterObjects(objects);

	return (FObjectPalette*)PropertyHandle->GetValueBaseAddress((uint8*)objects[0]);
}