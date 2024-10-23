#include "PhysicsHitReactEditor.h"
#include "HitReactComponent.h"
#include "PropertyEditorModule.h"
#include "HitReactComponentDetailsCustomization.h"

#define LOCTEXT_NAMESPACE "FPhysicsHitReactEditorModule"

void FPhysicsHitReactEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Hit React Component
	PropertyModule.RegisterCustomClassLayout(
		UHitReactComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FHitReactComponentDetailsCustomization::MakeInstance)
	);
}

void FPhysicsHitReactEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		PropertyModule->UnregisterCustomClassLayout(UHitReactComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPhysicsHitReactEditorModule, PhysicsHitReactEditor)