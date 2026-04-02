# MABS API

## What it is

This document lists the current Phase 5 public surface for MABS and shows which module owns authored timing, socket data, runtime scheduling state, projectile runtime, and debug helpers.

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
* `UMABSAbilityDefinition`

### `MABSGameplay`

Owns gameplay execution:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe presentation helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important enums

### `EMABSAbilityActivationResult`

Current notable values:

* `Success`
* `RequestSentToServer`
* `OnCooldown`
* `InsufficientResources`
* `DeliveryFailed`
* `TargetResolutionFailed`
* `EffectApplicationFailed`

### `EMABSDeliveryMode`

Current values:

* `Direct`
* `HitTrace`
* `Melee`
* `Projectile`

## Important structs

### `FMABSAbilitySpec`

Current fields:

* `Handle`
* `AbilityDefinition`
* `AbilityTag`
* `RuntimeState`
* `LastActivationResult`
* `CooldownEndTime`
* `ActivationStartTime`
* `ScheduledDeliveryTime`
* `RecoveryEndTime`

### `FMABSTargetTraceDebugInfo`

The latest trace or sweep snapshot stored on the ability component.

Current notable fields:

* `AbilityTag`
* `AbilityHandle`
* `DeliveryMode`
* `TraceMode`
* `TraceStart`
* `TraceEnd`
* `TraceRadius`
* `HitActorName`
* `TraceLabel`
* `ViewPointDescription`
* `ResultMessage`

## Important classes

### `UMABSAbilityDefinition`

Current authored fields include:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `DeliveryMode`
* `InstantEffectType`
* `EffectMagnitude`
* `StartupDuration`
* `DeliveryTime`
* `RecoveryDuration` as total ability duration from activation start
* direct targeting fields
* `DeliveryOriginSocketName`
* `HitTraceDistance`
* `HitTraceRadius`
* `HitTraceOriginSocketName`
* `HitTraceOriginOffset`
* `MeleeRange`
* `MeleeRadius`
* `MeleeOriginSocketName`
* `MeleeOriginOffset`
* `MeleeForwardOffset`
* `ProjectileActorClass`
* `ProjectileSpawnSocketName`
* `ProjectileSpawnOffset`
* `ActivationMontage`
* `MontagePlayRate`
* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

### `UMABSAbilityComponent`

Still exposes:

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

Runtime ownership now includes:

* granted ability specs
* cooldown-group state
* startup, delivery, and recovery scheduling
* socket-based origin resolution
* optional montage request hooks
* delivery execution
* projectile spawn and impact integration
* recent debug events
* the latest target or delivery trace snapshot

### `AMABSProjectileBase`

The built-in reusable projectile actor for `Projectile` delivery.

It:

* replicates movement
* stores the source ability and source actor context
* routes authoritative impact handling back through `UMABSAbilityComponent`
* applies the authored instant effect on a valid impact

## Important debug events

Phase 5 adds or highlights:

* `StartupStarted`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `SocketResolved`
* `SocketFallbackUsed`
* `MontagePlayRequested`
* `MontagePlayFailed`
* `RecoveryStarted`
* `RecoveryCompleted`
* `DeliveryStarted`
* `DeliveryFailed`
* `HitTraceHit`
* `HitTraceRejected`
* `MeleeHit`
* `MeleeRejected`
* `ProjectileSpawned`
* `ProjectileSpawnFailed`
* `ProjectileImpact`
* `ProjectileImpactRejected`
* `EffectApplied`
* `CooldownStarted`
* `CommitSucceeded`

## Example

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(FireballDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(FireballTag);
```

For `Projectile`, a `Success` result still means the projectile was spawned authoritatively and the ability committed successfully. The later impact result is reported through debug events, while the granted spec already moves through `Startup`, `Active`, and `Recovery`.
