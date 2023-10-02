// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class ObjectPaletteCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new ObjectPaletteCustomization());
	}
public:
	// IPropertyTypeCustomization interface
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		class FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils
	) override;

	virtual void CustomizeChildren(
		TSharedRef<class IPropertyHandle> StructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils
	) override;

private:
	bool OnIsAssetAcceptableForDrop(TArrayView<FAssetData> DraggedAssets);
	void OnAssetsDropped(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> DraggedAssets);
	void OnPaletteChanged(TSharedPtr<IPropertyHandle> Handle, TSharedPtr<IPropertyUtilities> Utils);
	struct FObjectPalette* GetData();
private:
	TSharedPtr<IPropertyHandle> PropertyHandle;
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;
};
