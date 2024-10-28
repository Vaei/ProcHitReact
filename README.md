# Physics Hit React <img align="right" width=128, height=128 src="https://github.com/Vaei/PhysicsHitReact/blob/main/Resources/Icon128.png">

> [!IMPORTANT]
> **Procedural physics-driven hit reactions for skeletal meshes!**
> <br>High levels of customization and extendability, covering many use-cases
> <br>Exceptionally easy to use
> <br>Fully usable & extendable in Blueprint

> [!TIP]
> Suitable for both singleplayer and multiplayer games

## Features
### Impulses
Supports Linear, Angular, and Radial impulses

Supports applying impulses to bones separate from the simulated bone

### Per-Bone Tuning
Supports disabling physics or using custom blend weights on child bones of the simulated bone

This means we can disable physics only on the arms for our character who is aiming a rifle!

### Profiles
Supports extendable profiles, simply specify the profile you want to use when triggering a Hit React

## How to Use
> [!IMPORTANT]
> [Read the Wiki to Learn How to use Hit React](https://github.com/Vaei/PhysicsHitReact/wiki)

## Example

TODO

## Changelog

### 1.3.0-beta
* Introduced the concept of interpolation decay, to handle reapplication of in-progress hit reacts
	* Getting hit repeatedly can now smoothly rewind the blend system partially!
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
