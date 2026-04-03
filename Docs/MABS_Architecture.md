# MABS Architecture

## What it is

This document explains the Phase 9 runtime architecture for MABS after the pre-v1 cleanup pass.

The goal is to keep runtime behavior stable while making ownership and extension points easier to follow.

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

`UMABSAbilityComponent` is still the orchestration surface.

It owns:

* the public Blueprint and C++ API
* authoritative runtime state
* replication boundaries
* activation entry points
* timing and lifecycle coordination
* debug event/query surfaces used by `MABSDebug`

It does not own harness rendering or formatting rules.

## Private implementation split

Phase 9 keeps the public header stable, but splits the private implementation into focused fragments:

* `MABSAbilityComponent_Core.inl`
* `MABSAbilityComponent_Granting.inl`
* `MABSAbilityComponent_Activation.inl`
* `MABSAbilityComponent_Delivery.inl`
* `MABSAbilityComponent_Effects.inl`
* `MABSAbilityComponent_Debug.inl`
* `MABSAbilityComponent_Presentation.inl`

This keeps the component readable without adding new UObject layers or moving gameplay authority out of the component.

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

That means the new fragments are organization helpers, not new state owners.

## Runtime flow

1. Authority grants one or more `UMABSAbilityDefinition` assets directly or through `UMABSAbilitySet`.
2. Gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, runtime state, cooldown, cost, and authored gameplay effect data.
4. The activation path enters startup, schedules delivery, and optionally opens combo windows.
5. Delivery resolves through `Direct`, `HitTrace`, `Melee`, or `Projectile`.
6. AoE and gameplay effects expand from the resolved impact context when authored.
7. Cooldowns, costs, recovery, and queued combo follow-ups are finalized.
8. Debug events and summary read models are updated for owner-facing inspection.

## Debug read-model flow

The debug harness does not own gameplay state.

The flow is:

1. `MABSGameplay` records `FMABSAbilityDebugEvent` and `FMABSTargetTraceDebugInfo`.
2. `UMABSAbilityComponent` exposes summary accessors such as `GetGrantedAbilityDebugSummaries()`.
3. `MABSDebug` reads those accessors.
4. `UMABSDebugBlueprintLibrary` formats the read model for display.
5. `AMABSDebugHUD` decides section layout, filtering, and presentation.

## Extension points

Extend MABS by touching the smallest owning layer:

* new authored data fields belong in `MABSCore`
* new delivery or effect behavior belongs in `MABSGameplay`
* new debug formatting or harness layout belongs in `MABSDebug`
* editor validation belongs in `MABSEditor`

For component work, add logic to the matching private implementation fragment before adding new systems.

## What MABS is not trying to be

Phase 9 still does not turn MABS into:

* a prediction-heavy ability framework
* a deep stacking or buff graph system
* a remote debugger
* a GAS replacement clone

The intent remains: a readable, multiplayer-ready, data-driven v1 ability framework.
