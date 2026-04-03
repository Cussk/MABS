# MABS Architecture

## What it is

This document explains the Phase 9.5 runtime architecture for MABS after the true de-monolith pass.

The goal is to keep runtime behavior stable while giving each major runtime concern a clearer internal home.

## Module map

### `MABSCore`

Owns:

* authored data assets
* shared enums and runtime structs
* combo, AoE, cooldown, and periodic-effect authored data
* shared debug event and summary read-model types

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* server-authoritative granting, activation, delivery, cooldown, cost, combo, AoE, and periodic-effect execution
* replicated runtime state
* runtime debug event emission and summary query accessors
* presentation routing entry points

### `MABSDebug`

Owns:

* `AMABSDebugHUD`
* `UMABSDebugBlueprintLibrary`
* formatting helpers
* category filtering
* harness layout and section rendering

### `MABSEditor`

Owns:

* editor-only extensions
* future validation and authoring helpers

## `UMABSAbilityComponent` ownership

`UMABSAbilityComponent` is still the public facade and authoritative state owner.

It owns:

* the public Blueprint and C++ API
* authoritative runtime state
* replication boundaries
* activation entry points
* lifecycle hooks
* top-level orchestration between activation, delivery, effects, presentation, and debug state

It does not own harness rendering or formatting rules, and it no longer relies on a `.inl` split as its primary internal structure.

## Internal runtime structure

Phase 9.5 replaces the old fragment split with real private implementation units:

* `MABSAbilityComponent.cpp`
* `MABSAbilityRuntime_Internal.h`
* `MABSAbilityRuntime_Internal.cpp`
* `MABSAbilityRuntime_Core.cpp`
* `MABSAbilityRuntime_Granting.cpp`
* `MABSAbilityRuntime_Activation.cpp`
* `MABSAbilityRuntime_Delivery.cpp`
* `MABSAbilityRuntime_Effects.cpp`
* `MABSAbilityRuntime_Presentation.cpp`
* `MABSAbilityRuntime_Debug.cpp`

That structure keeps the public component stable while making future runtime work easier to place.

## Runtime concern boundaries

### Core

Owns:

* replication setup
* small runtime queries
* debug replication toggles
* local state lookup helpers

### Granting

Owns:

* single ability grant flow
* ability set grant flow
* duplicate handling
* grouped grant bookkeeping

### Activation

Owns:

* activation validation
* combo queue acceptance and rejection
* startup scheduling
* combo window open and close
* recovery scheduling and completion

### Delivery

Owns:

* direct, hit trace, melee, and projectile delivery
* target resolution
* delivery socket/origin resolution
* AoE gather from impact context

### Effects

Owns:

* instant effect application
* periodic effect apply, tick, refresh, and expire
* cooldown finalization
* cost spend/finalization

### Presentation

Owns:

* montage request helpers
* cue transform resolution
* cue routing
* tracer routing
* startup, delivery, impact, and projectile-travel presentation triggers

### Debug runtime

Owns:

* debug event creation helpers
* debug event categorization
* target trace snapshot handling
* summary read-model assembly

## Runtime state model

Gameplay state still lives on `UMABSAbilityComponent`.

Primary replicated or authoritative state:

* `GrantedAbilities`
* `CooldownGroupStates`
* `ActivePeriodicEffects`
* `bReplicateDebugDataToOwningClient`

Primary transient runtime state:

* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`
* `ActiveAbilityExecutionContexts`
* `ActivePeriodicEffectRuntimes`
* reusable cue and tracer Niagara pools

That means the new runtime units separate behavior ownership, not state ownership.

## Runtime flow

1. Authority grants one or more `UMABSAbilityDefinition` assets directly or through `UMABSAbilitySet`.
2. Gameplay code calls `TryActivateAbilityByTag`.
3. The activation runtime validates grant, runtime state, cooldown, cost, and authored gameplay effect data.
4. Startup and recovery timing are coordinated by the activation runtime.
5. The delivery runtime resolves direct, trace, melee, or projectile delivery.
6. The effects runtime applies instant and periodic results and finalizes cost/cooldown state.
7. The presentation runtime routes montage and cue work.
8. The debug runtime records events and assembles summary read models for the harness.

## Debug read-model flow

The debug harness does not own gameplay state.

The flow is:

1. `MABSGameplay` records `FMABSAbilityDebugEvent` and `FMABSTargetTraceDebugInfo`.
2. `UMABSAbilityComponent` exposes summary accessors such as `GetGrantedAbilityDebugSummaries()`.
3. `MABSDebug` reads those accessors.
4. `UMABSDebugBlueprintLibrary` formats the read model for display.
5. `AMABSDebugHUD` decides section layout, filtering, and presentation.

## How to extend it

Extend MABS by touching the smallest owning layer:

* new authored data fields belong in `MABSCore`
* new grant rules belong in granting runtime
* new activation or combo behavior belongs in activation runtime
* new delivery logic belongs in delivery runtime
* new gameplay outcomes belong in effects runtime
* new cue/montage work belongs in presentation runtime
* new summaries belong in debug runtime
* new harness rendering belongs in `MABSDebug`

## Example

Example future work placement:

* a new trace variant belongs in `MABSAbilityRuntime_Delivery.cpp`
* a new status-timer rule belongs in `MABSAbilityRuntime_Effects.cpp`
* a new compact harness summary belongs in `MABSAbilityRuntime_Debug.cpp`
* a new HUD section belongs in `MABSDebugHUD.cpp`
