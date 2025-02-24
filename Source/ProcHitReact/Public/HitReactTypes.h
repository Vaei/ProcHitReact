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

/**
 * How to limit the max number of simulated bones
 */
UENUM(BlueprintType)
enum class EHitReactMaxHandling : uint8
{
	RemoveOldest,
	PreventNewest
};