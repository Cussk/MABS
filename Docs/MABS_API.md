# MABS API

## What it is

This document lists the current Phase 6 public surface for MABS and shows which module owns authored presentation data, runtime scheduling state, projectile travel presentation, and debug helpers.

## Module ownership

### `MABSCore`

Owns shared authored data and runtime types:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSTargetTraceMode`
* `EMABSAbilityActivationPolicy`
* `EMABSDeliveryMode`
* `EMABSInstantEffectType`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSCooldownGroupState`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `FMABSPresentationCameraShakeData`
* `FMABSPresentationCueData`
* `FMABSHitTraceTracerPresentationData`
* `FMABSProjectileTravelPresentationData`
* `FMABSPresentationStartupData`
* `FMABSPresentationDeliveryData`
* `FMABSPresentationImpactData`
* `UMABSAbilityDefinition`

### `MABSGameplay`

Owns gameplay and cosmetic trigger execution:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important structs

### `FMABSAbilitySpec`

Runtime fields remain:

* `Handle`
* `AbilityDefinition`
* `AbilityTag`
* `RuntimeState`
* `LastActivationResult`
* `CooldownEndTime`
* `ActivationStartTime`
* `ScheduledDeliveryTime`
* `RecoveryEndTime`

### `FMABSPresentationCueData`

Shared authored cue fields:

* `VFX`
* `SFX`
* `CameraShake`
* `SocketName`
* `LocationOffset`
* `RotationOffset`

### `FMABSPresentationDeliveryData`

Delivery presentation fields:

* `Cue`
* `HitTraceTracer`
* `ProjectileTravel`

### `FMABSTargetTraceDebugInfo`

The latest trace or sweep snapshot stored on the ability component.

Current notable fields:

* `AbilityTag`
* `AbilityHandle`
* `DeliveryMode`
* `TraceMode`
* `TraceStart`
* `TraceEnd`
* `HitLocation`
* `TraceRadius`
* `HitActorName`
* `TraceLabel`
* `ViewPointDescription`
* `ResultMessage`

## Important classes

### `UMABSAbilityDefinition`

Current authored fields include:

* core ability identity, effect, cooldown, and cost data
* `StartupDuration`
* `DeliveryTime`
* `RecoveryDuration`
* direct targeting fields
* `DeliveryOriginSocketName`
* hit-trace, melee, and projectile delivery socket fields
* optional montage fields
* `StartupPresentation`
* `DeliveryPresentation`
* `ImpactPresentation`

### `UMABSAbilityComponent`

Public Blueprint-facing functions remain:

* `GrantAbility(...)`
* `SetAbilityBlockedByTag(...)`
* `TryActivateAbilityByTag(...)`
* `SetDebugReplicationEnabled(...)`
* `IsDebugReplicationEnabled()`
* `GetGrantedAbilities()`
* `FindGrantedAbilitySpecByTag(...)`
* cooldown query helpers
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`

### `AMABSProjectileBase`

The built-in reusable projectile actor for `Projectile` delivery.

It:

* replicates movement
* stores replicated source ability context
* routes authoritative impact handling back through `UMABSAbilityComponent`
* applies the authored instant effect on a valid impact
* locally activates authored projectile travel presentation from the replicated projectile

## Important debug events

Phase 6 adds or highlights:

* `StartupPresentationTriggered`
* `DeliveryPresentationTriggered`
* `TracerSpawned`
* `TracerSpawnFailed`
* `ProjectileTravelPresentationTriggered`
* `ImpactPresentationTriggered`
* `PresentationSocketFallbackUsed`

## Example

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(FireballDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(FireballTag);
```
