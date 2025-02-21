// Copyright (c) Jared Taylor


#include "HitReactProfile.h"

#include "Misc/DataValidation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HitReactProfile)

#define LOCTEXT_NAMESPACE "HitReactProfile"

#if WITH_EDITOR
#if UE_5_03_OR_LATER
EDataValidationResult UHitReactProfile::IsDataValid(class FDataValidationContext& Context) const
#else
EDataValidationResult UHitReactProfile::IsDataValid(class FDataValidationContext& Context)
#endif
{
	// Subsequent impulse scalars must be in ascending order
	for (int32 i = 1; i < SubsequentImpulseScalars.Num(); i++)
	{
		if (SubsequentImpulseScalars[i].ElapsedTime <= SubsequentImpulseScalars[i - 1].ElapsedTime)
		{
			Context.AddError(FText::Format(
				LOCTEXT("HitReactProfile_SubsequentImpulseScalarsOrderError", "Subsequent impulse scalars must be in ascending order. Index {0} has an elapsed time of {1} which is less than or equal to the previous index's elapsed time of {2}"),
				i, SubsequentImpulseScalars[i].ElapsedTime, SubsequentImpulseScalars[i - 1].ElapsedTime));
			return EDataValidationResult::Invalid;
		}
	}

	return Super::IsDataValid(Context);
}
#endif

#undef LOCTEXT_NAMESPACE