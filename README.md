# Physics Hit React <img align="right" width=128, height=128 src="https://github.com/Vaei/PhysicsHitReact/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **Procedural physics-driven hit reactions for skeletal meshes!**
> <br>High levels of customization and extendability
> <br>Fully extendable in Blueprint
> <br>And its **Free!**

> [!TIP]
> Suitable for both singleplayer and multiplayer games

## Watch Me
TODO Showcase

TODO Overview

## Why Make a Hit Reaction Plugin?

> [!NOTE]
> Existing solutions and tutorials have severe limitations:

  * Visually disruptive when multiple hit reactions overlap
  * Lacks throttling to prevent frequent re-application of effects
  * No built-in handling for rapid re-application scenarios
  * Child bones’ blend weights are often overridden by parent bones
  * No option to disable blend weights or physics simulation on individual bones
  * Lacks support for overriding physics on arms when wielding a weapon
  * Re-simulation of an already active bone is not managed
  * Designed for single reactions, leading to instability and visual disruption with repeated hits
  * Limited consideration for real-world application in complex projects

> [!NOTE]
> PhysicsHitReact addresses all these limitations, delivering a stable, production-ready framework capable of commercial game development.

## Features
### Ease of Use
In many cases, all it takes is putting a component on your character and calling HitReact on it.

When setting up something out of the ordinary, like the turret in the videos, further setup is necessary, but it can be done even in blueprint.

### Impulses
Supports Linear, Angular, and Radial impulses.
TODO

### Per-Bone Tuning
Supports disabling physics or using custom blend weights on child bones of the simulated bone.

This means we can disable physics only on the arms for our character who is aiming a rifle!

Blacklist bones that don't simulate well, or remap them onto other bones. Don't want your pelvis to simulate because it offsets the entire mesh? Remap it to spine_01.

You can override per-bone application parameters, perhaps the standard impulse being provided for a shot is too strong for your clavicles? Simply add the clavicle to the overrides, and change the scaling.

### Profiles
Supports extendable profiles, simply specify the profile you want to use when triggering a Hit React.

The included HitReactComponent has many profiles setup for you out of the box with good defaults.

### Global Toggle
You can toggle the entire system on and off, with or without interpolation.

If you have the Gameplay Abilities (GAS) plugin enabled in your project, you can use tags to enable and disable the system. This is an optional dependency, PhysicsHitReact does not require that your project uses GAS and will not enable it for you.

### Sleep & Wake
The system automatically stops itself from ticking when it doesn't need to.

### Powerful Blending
Featuring a purpose-built interpolation framework, you can customize the blending to your liking.

The available states are: Blend In, Hold, and Blend Out. Hold will maintain your physics at full blend weight for the duration.

There is also Decay. When a hit react is already in progress, reapplying it will decay the simulation, making it interpolate backwards. Other systems either reinitialize physics from 0 causing a snap, or simply let it continue causing poor results under continuous application. Decay is the answer to that.

Reapplication can be throttled by setting a Cooldown.

Child bones are always simulated last so that the parent bones don't overwrite their simulation.

### Networking
Generally hit reacts are entirely cosmetic and should be applied via gameplay cues or other generalized multicast/replication events.

The data types used by PhysicsHitReact, including the application parameters, are net serialized for you, so you can replicate these too.

Dedicated servers don't process hit reacts, unless you enable the setting.

PhysicsHitReact was designed with multiplayer games in mind.

### Data Validation
PhysicsHitReact has reasonably stringent data validation to aid users who make a mess of the settings, further improving ease of use.

### Debugging Capability
See the [debugging section on the Wiki](https://github.com/Vaei/PhysicsHitReact/wiki/Debugging) to learn how to Debug PhysicsHitReact.

## How to Use
> [!IMPORTANT]
> [Read the Wiki to Learn How to use Hit React](https://github.com/Vaei/PhysicsHitReact/wiki)

## Changelog

### 1.4.1-beta
* Auto Wake and Sleep -- toggle ticking on and off to reduce overhead

### 1.4.0-beta
* Completely rebuilt the interpolation to better handle multiple states
* Completely rebuilt the global toggle
* All interpolation values are time based instead of rate based now
* Decay works properly, full support for Hold state
* Bones are sorted so child bones are processed last, allowing them to maintain their own blend weights
* Added new built-in blend profiles
	* Added TakeShot Unarmed/Armed profiles designed for rapid reapplication
	* Added TakeMelee Unarmed/Armed profiles designed for slower reapplication

### 1.3.0-beta
* Introduced the concept of interpolation decay, to handle reapplication of in-progress hit reacts
	* Getting hit repeatedly can now smoothly rewind the blend system partially!
	* WIP system that will become fully fledged in 1.4.0
* Overhauled application of hit reacts based on real-world usage
* Overhauled default profiles for great starting point parameters
	* Added more profiles
* Added PreActivate as a convenient point to cast and cache the owner in derived UHitReactComponent
* Add BlacklistBones and RemapBones for both simulated and impulse bones
* Added intensive data validation to UHitReactComponent to ensure coherent data
	* This is only really intended to avoid user-error resulting in undesired outcomes, hit react itself doesn't need it
* Improved debugging capability with p.HitReact.Debug.BlendWeights
* Fixed bug with cooldowns not applying
* Fixed bug with collision settings not updating
* Fixed edge case where completed hit reacts were not removed

### 1.2.1-beta
* Added global disable CVar

### 1.2.0-beta
* Restructure and unify properties and parameters based on tested use-cases
	* Introduce FHitReactApplyParams for passing around the required data to apply a hit react
	* Update UHitReactComponent::HitReact() to use this
* Expose the blend params -- oops
* Refactor properties>params for consistency
* Add net serialization to FHitReactImpulseParams and FHitReactImpulseWorldParams and all contained types
* Add BlueprintCosmetic where appropriate

### 1.1.2-beta
* Improve tick settings
* Improve dedi server handling
* Add PhysicsHitReactEditor module
* Add details customization to hide properties that shouldn't be touched
	* Tick in particular, because the system must handle it internally

### 1.1.1-beta
* Default profile if none provided to HitReact()
* Separate transient data from ImpulseParams into new WorldSpaceParams
* Improved failure debug logging
* Added check for physics asset

### 1.1.0-beta
* Add support to include GameplayAbilities as optional plugin
* Add support for ability tags to disable/enable the system
* Fix missing null check for optional PhysicalAnimationComponent

### 1.0.0-beta
* Initial Release
