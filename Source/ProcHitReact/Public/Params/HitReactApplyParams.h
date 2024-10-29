// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitReactImpulseParams.h"
#include "HitReactParams.h"
#include "HitReactApplyParams.generated.h"

/*
 * NOTE:
 * Apply params are structs that hold all data required to apply a hit react, primarily for the purpose of replicating hit reacts
 * This is actually against the intended use of the HitReact system, as it is designed to be sent as a data-less event
 * Ordinarily, you would have a 'get shot' system that already knows how to apply a hit react based on the hit normal, magnitude, etc.
 *
 * Separate structs exist for irregular use-cases involving multicasts, such as applying linear, angular, or radial impulses separately,
 *	so that you don't need to send all three impulses when only one is being applied
 */

/**
 * The required data to apply a hit reaction
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactApplyParams : public FHitReactParams
{
	GENERATED_BODY()

	FHitReactApplyParams()
	{}

	FHitReactApplyParams(const FGameplayTag& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactImpulseParams& InImpulseParams)
		: FHitReactParams(InProfileToUse, InBoneName, bInIncludeSelf)
		, ImpulseParams(InImpulseParams)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactImpulseParams ImpulseParams;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (ImpulseParams.LinearImpulse || ImpulseParams.AngularImpulse || ImpulseParams.RadialImpulse)
		{
			Ar << ProfileToUse;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			ImpulseParams.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType);
	const FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const;
};

template<>
struct TStructOpsTypeTraits<FHitReactApplyParams> : public TStructOpsTypeTraitsBase2<FHitReactApplyParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * The required data to apply a hit reaction for linear impulses
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactApplyParamsLinear : public FHitReactParams
{
	GENERATED_BODY()

	FHitReactApplyParamsLinear()
	{}

	FHitReactApplyParamsLinear(const FGameplayTag& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactLinearImpulseParams& InLinearImpulse)
		: FHitReactParams(InProfileToUse, InBoneName, bInIncludeSelf)
		, LinearImpulse(InLinearImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactLinearImpulseParams LinearImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (LinearImpulse)
		{
			Ar << ProfileToUse;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			LinearImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType);
	const FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const;
};

template<>
struct TStructOpsTypeTraits<FHitReactApplyParamsLinear> : public TStructOpsTypeTraitsBase2<FHitReactApplyParamsLinear>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * The required data to apply a hit reaction for angular impulses
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactApplyParamsAngular : public FHitReactParams
{
	GENERATED_BODY()

	FHitReactApplyParamsAngular()
	{}

	FHitReactApplyParamsAngular(const FGameplayTag& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactAngularImpulseParams& InAngularImpulse)
		: FHitReactParams(InProfileToUse, InBoneName, bInIncludeSelf)
		, AngularImpulse(InAngularImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactAngularImpulseParams AngularImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (AngularImpulse)
		{
			Ar << ProfileToUse;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			AngularImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType);
	const FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const;
};

template<>
struct TStructOpsTypeTraits<FHitReactApplyParamsAngular> : public TStructOpsTypeTraitsBase2<FHitReactApplyParamsAngular>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * The required data to apply a hit reaction for radial impulses
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactApplyParamsRadial : public FHitReactParams
{
	GENERATED_BODY()

	FHitReactApplyParamsRadial()
	{}

	FHitReactApplyParamsRadial(const FGameplayTag& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactRadialImpulseParams& InRadialImpulse)
		: FHitReactParams(InProfileToUse, InBoneName, bInIncludeSelf)
		, RadialImpulse(InRadialImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactRadialImpulseParams RadialImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (RadialImpulse)
		{
			Ar << ProfileToUse;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			RadialImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType);
	const FHitReactImpulseParamsBase& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const;
};