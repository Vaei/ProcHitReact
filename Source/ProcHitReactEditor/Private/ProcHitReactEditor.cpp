#include "ProcHitReactEditor.h"
#include "PropertyEditorModule.h"
#include "HitReact.h"
#include "HitReactDetailsCustomization.h"

#define LOCTEXT_NAMESPACE "FProcHitReactEditorModule"

void FProcHitReactEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Hit React Component
	PropertyModule.RegisterCustomClassLayout(
		UHitReact::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FHitReactDetailsCustomization::MakeInstance)
	);
}

void FProcHitReactEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		// Hit React Component
		PropertyModule->UnregisterCustomClassLayout(UHitReact::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FProcHitReactEditorModule, ProcHitReactEditor)