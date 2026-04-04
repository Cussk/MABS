# MABS API

## What it is

This document lists the current Phase 11 public surface for MABS.

Phase 11 keeps the existing activation, effect, and debug API stable while adding a public delivery-handler seam for custom hit trace, melee, direct, and projectile behavior.

## Module ownership

### `MABSCore`

Owns shared authored data, runtime types, and shared debug read models:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSTargetTraceMode`
* `EMABSAbilityActivationPolicy`
* `EMABSDeliveryMode`
* `EMABSInstantEffectType`
* `EMABSAoEShape`
* `EMABSPeriodicEffectType`
* `EMABSDebugEventCategory`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSCooldownGroupState`
* `FMABSComboData`
* `FMABSAoEData`
* `FMABSPeriodicEffectData`
* `FMABSActivePeriodicEffect`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `FMABSGrantedAbilityDebugSummary`
* `FMABSCooldownGroupDebugSummary`
* `FMABSComboDebugSummary`
* `FMABSPeriodicEffectDebugSummary`
* `FMABSPresentationCueEvent`
* `FMABSTracerCueEvent`
* `FMABSProjectileTravelCueEvent`
* `UMABSAbilityDefinition`
* `UMABSAbilitySet`

### `MABSGameplay`

Owns gameplay execution, delivery handlers, and debug query accessors:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `FMABSDeliveryExecutionContext`
* `FMABSDeliveryExecutionResult`
* `UMABSDeliveryHandler`
* `UMABSDirectDeliveryHandler`
* `UMABSHitTraceDeliveryHandler`
* `UMABSMeleeDeliveryHandler`
* `UMABSProjectileDeliveryHandler`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns harness UI and formatting:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important authored assets

### `UMABSAbilityDefinition`

Owns authored per-ability fields such as:

* `AbilityTag`
* `DeliveryMode`
* `DeliveryHandlerClass`
* `TargetType`
* timing fields
* presentation groups
* `Combo`
* `AoE`
* `PeriodicEffect`

### `UMABSAbilitySet`

Owns grouped grant authoring through:

* `AbilityDefinitions`

`UMABSAbilitySet` does not store runtime grant state. It is still only a grouped authoring asset.

## Important runtime and debug read-model types

### Delivery handler types

`FMABSDeliveryExecutionContext` exposes the execution-side view that custom handlers use, including:

* owning `UMABSAbilityComponent`
* copied `FMABSAbilitySpec`
* owning actor / instigator access
* delivery mode
* source transform and current trace viewpoint data when available

`FMABSDeliveryExecutionResult` exposes the handler result that the runtime consumes, including:

* delivery success or failure
* failure result and debug message
* resolved target actors
* primary target / hit data
* impact location and normal
* whether standard effects and impact presentation should continue

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

### `FMABSAbilityDebugEvent`

The structured event model includes:

* `Category`

That category is intended for harness filtering, not for gameplay logic.

### `FMABSGrantedAbilityDebugSummary`

Compact harness-facing ability state:

* tag and display name
* runtime state and last activation result
* cooldown and cooldown-group remaining time
* delivery mode
* combo-window and queued-follow-up state
* resource cost

### `FMABSCooldownGroupDebugSummary`

Compact harness-facing cooldown-group state:

* cooldown group tag
* remaining time

### `FMABSComboDebugSummary`

Compact harness-facing combo state:

* source ability
* next combo tag
* window open or opens-in state
* queued follow-up
* buffer setting

### `FMABSPeriodicEffectDebugSummary`

Compact harness-facing periodic effect state:

* source ability
* target name
* type
* tick magnitude and interval
* time until next tick
* time remaining

## Important `UMABSAbilityComponent` API

Granting:

* `GrantAbility(...)`
* `GrantAbilitySet(...)`
* `GrantAbilitySets(...)`

Activation and gameplay-facing queries:

* `TryActivateAbilityByTag(...)`
* `GetGrantedAbilities()`
* `GetActivePeriodicEffects()`

Harness-facing queries:

* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`
* `SetDebugReplicationEnabled(...)`
* `IsDebugReplicationEnabled()`

## Important `AMABSDebugHUD` API

The runtime harness HUD exposes:

* `SetOverlayEnabled(...)`
* `ToggleOverlayEnabled()`
* `SetCategoryEnabled(...)`
* `ToggleCategoryEnabled(...)`
* `IsCategoryEnabled(...)`

## Important debug helpers

`UMABSDebugBlueprintLibrary` exposes harness-facing formatting helpers for:

* compact event lines
* granted ability summaries
* cooldown group summaries
* combo summaries
* periodic summaries
* category labels and colors

## Phase 11 implementation note

The public API above is still owned by `UMABSAbilityComponent`.

Internally, the implementation still lives in real runtime units for:

* core state and replication
* granting
* activation and combo flow
* delivery handler resolution, execution, and targeting
* effects, periodic runtime, cost, and cooldown
* debug read-model assembly
* presentation routing

Phase 11 adds public delivery handler headers plus private built-in handler implementations, while `MABSAbilityRuntime_Delivery.cpp` becomes the orchestrator that resolves a handler, runs it, and then continues the normal effect, presentation, cooldown, cost, and recovery flow.
