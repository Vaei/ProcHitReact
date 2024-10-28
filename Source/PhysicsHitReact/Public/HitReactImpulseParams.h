// Copyright (c) Jared Taylor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HitReactTypes.h"
#include "HitReactImpulseParams.generated.h"

UENUM(BlueprintType)
enum class EHitReactImpulseType : uint8
{
	Linear,
	Angular,
	Radial,
};

UENUM(BlueprintType)
enum class EHitReactUnits : uint8
{
	Degrees,
	Radians
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactImpulseParamsBase
{
	GENERATED_BODY()

	FHitReactImpulseParamsBase()
		: bApplyImpulse(false)
		, bFactorMass(false)
		, Impulse(500.f)
	{}
	virtual ~FHitReactImpulseParamsBase() = default;

	/** If false, will not be applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	bool bApplyImpulse;

	/**	If true, impulse is taken as a change in velocity instead of an impulse (i.e. mass will have no effect). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides))
	bool bFactorMass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides, UIMin="0", ClampMin="0"))
	float Impulse;

	operator bool() const { return bApplyImpulse; }

	bool IsVelocityChange() const { return !bFactorMass; }

	virtual bool CanBeApplied() const
	{
		return bApplyImpulse && Impulse > 0.f;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar.SerializeBits(&bApplyImpulse, 1);
		if (bApplyImpulse)
		{
			Ar.SerializeBits(&bFactorMass, 1);
			Ar << Impulse;
		}
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactImpulseParamsBase> : public TStructOpsTypeTraitsBase2<FHitReactImpulseParamsBase>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactLinearImpulseParams : public FHitReactImpulseParamsBase
{
	GENERATED_BODY()

	FHitReactLinearImpulseParams()
	{}

	virtual FVector GetImpulse(const FVector& WorldDirection) const
	{
		return WorldDirection * Impulse;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override
	{
		Ar.SerializeBits(&bApplyImpulse, 1);
		if (bApplyImpulse)
		{
			Ar.SerializeBits(&bFactorMass, 1);
			Ar << Impulse;
		}
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactLinearImpulseParams> : public TStructOpsTypeTraitsBase2<FHitReactLinearImpulseParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactAngularImpulseParams : public FHitReactLinearImpulseParams
{
	GENERATED_BODY()

	FHitReactAngularImpulseParams()
		: AngularUnits(EHitReactUnits::Degrees)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides))
	EHitReactUnits AngularUnits;
	
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override
	{
		Ar.SerializeBits(&bApplyImpulse, 1);
		if (bApplyImpulse)
		{
			Ar.SerializeBits(&bFactorMass, 1);
			Ar << Impulse;
			Ar << AngularUnits;
		}
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactAngularImpulseParams> : public TStructOpsTypeTraitsBase2<FHitReactAngularImpulseParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactRadialImpulseParams : public FHitReactImpulseParamsBase
{
	GENERATED_BODY()

	FHitReactRadialImpulseParams()
		: Radius(150.f)
		, Falloff(RIF_Linear)
	{}

	/** Radius of the impulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides, UIMin="0", ClampMin="0", ForceUnits="cm"))
	float Radius;

	/** How the strength of the impulse should fall off with distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides))
	TEnumAsByte<ERadialImpulseFalloff> Falloff;

	virtual bool CanBeApplied() const override final
	{
		return FHitReactImpulseParamsBase::CanBeApplied() && Radius > 0.f;
	}
	
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override
	{
		Ar.SerializeBits(&bApplyImpulse, 1);
		if (bApplyImpulse)
		{
			Ar.SerializeBits(&bFactorMass, 1);
			Ar << Impulse;
			Ar << Radius;
			Ar << Falloff;
		}
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactRadialImpulseParams> : public TStructOpsTypeTraitsBase2<FHitReactRadialImpulseParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactImpulseParams
{
	GENERATED_BODY()

	FHitReactImpulseParams()
	{}

	FHitReactImpulseParams(const FHitReactLinearImpulseParams& InLinearImpulse, const FHitReactAngularImpulseParams& InAngularImpulse, const FHitReactRadialImpulseParams& InRadialImpulse)
		: LinearImpulse(InLinearImpulse)
		, AngularImpulse(InAngularImpulse)
		, RadialImpulse(InRadialImpulse)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactLinearImpulseParams LinearImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactAngularImpulseParams AngularImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactRadialImpulseParams RadialImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		LinearImpulse.NetSerialize(Ar, Map, bOutSuccess);
		AngularImpulse.NetSerialize(Ar, Map, bOutSuccess);
		RadialImpulse.NetSerialize(Ar, Map, bOutSuccess);
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactImpulseParams> : public TStructOpsTypeTraitsBase2<FHitReactImpulseParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * World space parameters for applying impulses
 * These are set during runtime and are not saved
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactImpulseWorldParams
{
	GENERATED_BODY()

	FHitReactImpulseWorldParams()
		: LinearDirection(FVector::ZeroVector)
		, AngularDirection(FVector::ZeroVector)
		, RadialLocation(FVector::ZeroVector)
	{}

	/** Direction to apply the linear impulse */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadWrite, Category=Physics)
	FVector LinearDirection;

	/** Direction to apply the angular impulse */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadWrite, Category=Physics)
	FVector AngularDirection;

	/** World Location to apply the radial impulse */
	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadWrite, Category=Physics)
	FVector RadialLocation;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		LinearDirection.NetSerialize(Ar, Map, bOutSuccess);
		AngularDirection.NetSerialize(Ar, Map, bOutSuccess);
		RadialLocation.NetSerialize(Ar, Map, bOutSuccess);
		return !Ar.IsError();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactImpulseWorldParams> : public TStructOpsTypeTraitsBase2<FHitReactImpulseWorldParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * The required data to apply a hit reaction
 * Convenience struct to pass around data
 * Especially useful for replication
 */
USTRUCT(BlueprintType)
struct PHYSICSHITREACT_API FHitReactApplyParams : public FHitReactParams
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