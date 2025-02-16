// Copyright (c) Jared Taylor. All Rights Reserved.

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

enum class EHitReactTickRequest : uint8
{
	None,
	Removal,
	Finalize,
	BlendWeight
};

struct PROCHITREACT_API FHitReactBlendWeights
{
	TArray<float> BlendWeights;

	FORCEINLINE_DEBUGGABLE float GetBlendWeightAverage() const
	{
		if (BlendWeights.Num() == 0)
		{
			return 0.f;
		}

		float Total = 0.f;
		for (float Weight : BlendWeights)
		{
			Total += Weight;
		}

		return Total / BlendWeights.Num();
	}

	// FORCEINLINE_DEBUGGABLE void AverageBlendWeights()
	// {
	// 	if (BlendWeights.Num() <= 1)
	// 	{
	// 		return;
	// 	}
	//
	// 	float Total = 0.f;
	// 	for (float Weight : BlendWeights)
	// 	{
	// 		Total += Weight;
	// 	}
	//
	// 	const float Average = Total / BlendWeights.Num();
	// 	for (float& Weight : BlendWeights)
	// 	{
	// 		Weight = Average;
	// 	}
	// }
};