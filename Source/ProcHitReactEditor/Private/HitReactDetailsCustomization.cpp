// Copyright (c) Jared Taylor. All Rights Reserved


#include "HitReactDetailsCustomization.h"

#include "DetailLayoutBuilder.h"
#include "HitReact.h"


TSharedRef<IDetailCustomization> FHitReactDetailsCustomization::MakeInstance()
{
	return MakeShared<FHitReactDetailsCustomization>();
}

void FHitReactDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	for (const TWeakObjectPtr<UObject>& Object : Objects)
	{
		if (UHitReact* HitReact = Cast<UHitReact>(Object.Get()))
		{
			// Bump HitReact to the top of the details panel
			DetailBuilder.EditCategory(TEXT("HitReact"), FText::GetEmpty(), ECategoryPriority::Transform);
			
			// We cannot hide these properties because they're abnormal, just hide the category
			DetailBuilder.HideCategory("ComponentTick");
			DetailBuilder.HideCategory("Sockets");
			
			DetailBuilder.HideProperty("bReplicates", UActorComponent::StaticClass());
			DetailBuilder.HideProperty("bIsEditorOnly", UActorComponent::StaticClass());
			DetailBuilder.HideProperty("bCanEverAffectNavigation", UActorComponent::StaticClass());
			DetailBuilder.HideProperty("bReplicateUsingRegisteredSubObjectList", UActorComponent::StaticClass());
		}
	}
}