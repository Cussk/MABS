# MABS API

## What it is

This document lists the current Phase 7 public surface for MABS and shows which module owns authored combo / AoE / periodic data, runtime combo state, periodic runtime entries, and the main gameplay component API.

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
* `EMABSAoEShape`
* `EMABSPeriodicEffectType`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSCooldownGroupState`
* `FMABSComboData`
* `FMABSAoEData`
* `FMABSPeriodicEffectData`
* `FMABSActivePeriodicEffect`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `FMABSPresentationCueEvent`
* `FMABSTracerCueEvent`
* `FMABSProjectileTravelCueEvent`
* `UMABSAbilityDefinition`

### `MABSGameplay`

Owns gameplay and cue execution:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important authored fields

### `FMABSComboData`

Combo authored fields:

* `NextComboAbilityTag`
* `ComboWindowStart`
* `ComboWindowEnd`
* `bBufferComboInput`

### `FMABSAoEData`

AoE authored fields:

* `bEnabled`
* `Shape`
* `Radius`
* `BoxExtent`
* `CapsuleRadius`
* `CapsuleHalfHeight`
* `Offset`

### `FMABSPeriodicEffectData`

Periodic authored fields:

* `bEnabled`
* `EffectType`
* `Duration`
* `TickInterval`
* `TickMagnitude`

## Important runtime types

### `FMABSAbilitySpec`

Phase 7 adds combo runtime fields:

* `ComboWindowStartTime`
* `ComboWindowEndTime`
* `QueuedComboAbilityTag`

### `FMABSActivePeriodicEffect`

Authority-side periodic runtime fields:

* `RuntimeId`
* `AbilityTag`
* `AbilityHandle`
* `AbilityDefinition`
* `SourceActor`
* `TargetActor`
* `EffectType`
* `TickMagnitude`
* `TickInterval`
* `AppliedWorldTime`
* `ExpirationWorldTime`

### `UMABSAbilityComponent`

Important Phase 7 API points:

* `TryActivateAbilityByTag(...)`
* `GetGrantedAbilities()`
* `GetActivePeriodicEffects()`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`

## Important debug events

Phase 7 adds or highlights:

* `ComboWindowStarted`
* `ComboWindowEnded`
* `ComboQueued`
* `ComboRejected`
* `AoEResolved`
* `AoETargetRejected`
* `PeriodicEffectApplied`
* `PeriodicEffectRefreshed`
* `PeriodicEffectTick`
* `PeriodicEffectExpired`
