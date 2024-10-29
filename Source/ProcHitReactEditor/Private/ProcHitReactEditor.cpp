#include "ProcHitReactEditor.h"
#include "HitReactComponent.h"
#include "PropertyEditorModule.h"
#include "HitReactComponentDetailsCustomization.h"

#define LOCTEXT_NAMESPACE "FProcHitReactEditorModule"

void FProcHitReactEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Hit React Component
	PropertyModule.RegisterCustomClassLayout(
		UHitReactComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FHitReactComponentDetailsCustomization::MakeInstance)
	);
}

void FProcHitReactEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		PropertyModule->UnregisterCustomClassLayout(UHitReactComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FProcHitReactEditorModule, ProcHitReactEditor)