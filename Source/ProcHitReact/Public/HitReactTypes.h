// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHitReact, Log, All);

UENUM(BlueprintType)
enum class EHitReactToggleState : uint8
{
	Disabled,
	Disabling,
	Enabling,
	Enabled
};
