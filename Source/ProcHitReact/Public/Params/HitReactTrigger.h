// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactImpulse.h"
#include "HitReactParams.h"
#include "HitReactTrigger.generated.h"

/*
 * NOTE:
 * Triggers are structs that hold all data required to trigger a hit react, primarily for the purpose of replicating hit reacts
 * This is actually against the intended use of the HitReact system, as it is designed to be sent as a data-less event
 * Ordinarily, you would have a 'get shot' system that already knows how to trigger a hit react based on the hit normal, magnitude, etc.
 *
 * Separate structs exist for irregular use-cases involving multicasts, such as triggering linear, angular, or radial impulses separately,
 *	so that you don't need to send all three impulses when only one is being applied
 */

/**
 * The required data to trigger a hit reaction
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactTrigger : public FHitReactInputParams
{
	using Super = FHitReactInputParams;
	
	GENERATED_BODY()

	FHitReactTrigger()
	{}

	FHitReactTrigger(const TSoftObjectPtr<UHitReactProfile>& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactImpulseParams& InImpulse)
		: Super(InProfileToUse, InBoneName, bInIncludeSelf)
		, Impulse(InImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactImpulseParams Impulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (Impulse.LinearImpulse || Impulse.AngularImpulse || Impulse.RadialImpulse)
		{
			Ar << Profile;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			Impulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType);
	const FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const;
};

template<>
struct TStructOpsTypeTraits<FHitReactTrigger> : public TStructOpsTypeTraitsBase2<FHitReactTrigger>
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
struct PROCHITREACT_API FHitReactTrigger_Linear : public FHitReactInputParams
{
	using Super = FHitReactInputParams;
	
	GENERATED_BODY()

	FHitReactTrigger_Linear()
	{}

	FHitReactTrigger_Linear(const TSoftObjectPtr<UHitReactProfile>& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactImpulse_Linear& InLinearImpulse)
		: Super(InProfileToUse, InBoneName, bInIncludeSelf)
		, LinearImpulse(InLinearImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactImpulse_Linear LinearImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (LinearImpulse)
		{
			Ar << Profile;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			LinearImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) { return LinearImpulse; }
	const FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const { return LinearImpulse; }
};

template<>
struct TStructOpsTypeTraits<FHitReactTrigger_Linear> : public TStructOpsTypeTraitsBase2<FHitReactTrigger_Linear>
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
struct PROCHITREACT_API FHitReactTrigger_Angular : public FHitReactInputParams
{
	using Super = FHitReactInputParams;
	
	GENERATED_BODY()

	FHitReactTrigger_Angular()
	{}

	FHitReactTrigger_Angular(const TSoftObjectPtr<UHitReactProfile>& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactImpulse_Angular& InAngularImpulse)
		: Super(InProfileToUse, InBoneName, bInIncludeSelf)
		, AngularImpulse(InAngularImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactImpulse_Angular AngularImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (AngularImpulse)
		{
			Ar << Profile;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			AngularImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) { return AngularImpulse; }
	const FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const { return AngularImpulse; }
};

template<>
struct TStructOpsTypeTraits<FHitReactTrigger_Angular> : public TStructOpsTypeTraitsBase2<FHitReactTrigger_Angular>
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
struct PROCHITREACT_API FHitReactTrigger_Radial : public FHitReactInputParams
{
	using Super = FHitReactInputParams;
	
	GENERATED_BODY()

	FHitReactTrigger_Radial()
	{}

	FHitReactTrigger_Radial(const TSoftObjectPtr<UHitReactProfile>& InProfileToUse, const FName& InBoneName, bool bInIncludeSelf,
		const FHitReactImpulse_Radial& InRadialImpulse)
		: Super(InProfileToUse, InBoneName, bInIncludeSelf)
		, RadialImpulse(InRadialImpulse)
	{}

	/** The impulse parameters to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HitReact)
	FHitReactImpulse_Radial RadialImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// Only serialize any params if they are actually being applied
		if (RadialImpulse)
		{
			Ar << Profile;
			Ar << SimulatedBoneName;
			Ar << bIncludeSelf;
			RadialImpulse.NetSerialize(Ar, Map, bOutSuccess);
		}
		return !Ar.IsError();
	}

	FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) { return RadialImpulse; }
	const FHitReactImpulse& GetImpulseParamsBase(const EHitReactImpulseType& ImpulseType) const { return RadialImpulse; }
};