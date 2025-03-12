// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHitReact, Log, All);

/**
 * Global toggle state for the hit reaction system
 */
UENUM(BlueprintType)
enum class EHitReactToggleState : uint8
{
	Disabled,
	Disabling,
	Enabling,
	Enabled
};

UENUM(BlueprintType)
enum class EHitReactMaxBlendHandling : uint8
{
	Disabled			UMETA(ToolTip="Apply the hit react regardless of how many blends are active"),
	ImpulseOnly			UMETA(ToolTip="Only apply the impulse without modifying bone blend weights"),
	Blocked				UMETA(ToolTip="Block the hit react if the maximum number of blends are active"),
};