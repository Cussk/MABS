# MABS API

## What it is

This document lists the current Phase 7.5 public surface for MABS and shows which module owns authored ability-set data, granted runtime state, and grouped grant helpers.

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
* `UMABSAbilitySet`

### `MABSGameplay`

Owns gameplay and grant execution:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important authored assets

### `UMABSAbilityDefinition`

Owns authored per-ability fields such as:

* `AbilityTag`
* `DeliveryMode`
* `TargetType`
* timing fields
* presentation groups
* `Combo`
* `AoE`
* `PeriodicEffect`

### `UMABSAbilitySet`

Owns grouped grant authoring through:

* `AbilityDefinitions`

`UMABSAbilitySet` does not store runtime grant state. It is only a grouped authoring asset.

## Important runtime types

### `FMABSAbilitySpec`

Granted runtime state still lives here:

* definition reference
* granted handle
* runtime state
* last activation result
* cooldown timing
* startup, delivery, and recovery timestamps
* combo window timestamps
* queued combo follow-up tag

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

## Important `UMABSAbilityComponent` API

Granting:

* `GrantAbility(...)`
* `GrantAbilitySet(...)`
* `GrantAbilitySets(...)`

Activation and queries:

* `TryActivateAbilityByTag(...)`
* `GetGrantedAbilities()`
* `GetActivePeriodicEffects()`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`

## Important debug events

Phase 7.5 adds or highlights:

* `AbilitySetGranted`
* `AbilitySetGrantSkipped`
* `AbilitySetGrantFailed`

Earlier grant, activation, delivery, effect, combo, AoE, and periodic events still remain.
