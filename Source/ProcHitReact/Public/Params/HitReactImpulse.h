// Copyright (c) Jared Taylor

#pragma once

#include "CoreMinimal.h"
#include "HitReactImpulse.generated.h"

class UHitReactProfile;

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

UENUM(BlueprintType)
enum class EHitReactFalloff : uint8
{
	Linear,
	Constant,
};

USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactImpulse
{
	GENERATED_BODY()

	FHitReactImpulse()
		: bApplyImpulse(false)
		, bFactorMass(false)
		, Impulse(500.f)
	{}
	virtual ~FHitReactImpulse() = default;

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
struct TStructOpsTypeTraits<FHitReactImpulse> : TStructOpsTypeTraitsBase2<FHitReactImpulse>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactImpulse_Linear : public FHitReactImpulse
{
	GENERATED_BODY()

	FHitReactImpulse_Linear()
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
struct TStructOpsTypeTraits<FHitReactImpulse_Linear> : TStructOpsTypeTraitsBase2<FHitReactImpulse_Linear>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactImpulse_Angular : public FHitReactImpulse_Linear
{
	GENERATED_BODY()

	FHitReactImpulse_Angular()
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
struct TStructOpsTypeTraits<FHitReactImpulse_Angular> : TStructOpsTypeTraitsBase2<FHitReactImpulse_Angular>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactImpulse_Radial : public FHitReactImpulse
{
	GENERATED_BODY()

	FHitReactImpulse_Radial()
		: Radius(150.f)
		, Falloff(EHitReactFalloff::Linear)
	{}

	/** Radius of the impulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides, UIMin="0", ClampMin="0", ForceUnits="cm"))
	float Radius;

	/** How the strength of the impulse should fall off with distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics, meta=(EditCondition="bApplyImpulse", EditConditionHides))
	EHitReactFalloff Falloff;

	virtual bool CanBeApplied() const override final
	{
		return FHitReactImpulse::CanBeApplied() && Radius > 0.f;
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
struct TStructOpsTypeTraits<FHitReactImpulse_Radial> : TStructOpsTypeTraitsBase2<FHitReactImpulse_Radial>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct PROCHITREACT_API FHitReactImpulseParams
{
	GENERATED_BODY()

	FHitReactImpulseParams()
	{}

	FHitReactImpulseParams(const FHitReactImpulse_Linear& InLinearImpulse, const FHitReactImpulse_Angular& InAngularImpulse, const FHitReactImpulse_Radial& InRadialImpulse)
		: LinearImpulse(InLinearImpulse)
		, AngularImpulse(InAngularImpulse)
		, RadialImpulse(InRadialImpulse)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactImpulse_Linear LinearImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactImpulse_Angular AngularImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Physics)
	FHitReactImpulse_Radial RadialImpulse;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		LinearImpulse.NetSerialize(Ar, Map, bOutSuccess);
		AngularImpulse.NetSerialize(Ar, Map, bOutSuccess);
		RadialImpulse.NetSerialize(Ar, Map, bOutSuccess);
		return !Ar.IsError();
	}

	bool CanBeApplied() const
	{
		return LinearImpulse.CanBeApplied() || AngularImpulse.CanBeApplied() || RadialImpulse.CanBeApplied();
	}
};

template<>
struct TStructOpsTypeTraits<FHitReactImpulseParams> : TStructOpsTypeTraitsBase2<FHitReactImpulseParams>
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
struct PROCHITREACT_API FHitReactImpulse_WorldParams
{
	GENERATED_BODY()

	FHitReactImpulse_WorldParams()
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
struct TStructOpsTypeTraits<FHitReactImpulse_WorldParams> : TStructOpsTypeTraitsBase2<FHitReactImpulse_WorldParams>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct PROCHITREACT_API FHitReactPendingImpulse
{
	GENERATED_BODY()

	FHitReactPendingImpulse()
		: ImpulseScalar(1.f)
		, Profile(nullptr)
		, ImpulseBoneName(NAME_None)
	{}

	FHitReactPendingImpulse(const FHitReactImpulseParams& InImpulse, const FHitReactImpulse_WorldParams& InWorld,
		float InImpulseScalar, const UHitReactProfile* InProfile, FName InImpulseBoneName);

	UPROPERTY()
	FHitReactImpulseParams Impulse;

	UPROPERTY()
	FHitReactImpulse_WorldParams World;

	UPROPERTY()
	float ImpulseScalar;

	UPROPERTY()
	const UHitReactProfile* Profile;

	UPROPERTY()
	FName ImpulseBoneName;

	bool IsValid() const
	{
		return Profile && Impulse.CanBeApplied();
	}
};