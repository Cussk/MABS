# MABS Modules

## What it is

This document explains the Phase 9 module layout for MABS.

Phase 9 keeps the same runtime ownership, but cleans the internal gameplay implementation so `UMABSAbilityComponent` remains the API surface instead of a monolithic single-file implementation.

## Module map

### `MABSCore`

Owns:

* `UMABSAbilityDefinition`
* `UMABSAbilitySet`
* ability runtime structs and enums
* authored timing fields
* authored socket fields
* authored montage references
* authored presentation structs and fields
* authored combo data
* authored AoE data
* authored periodic effect data
* `FMABSAbilitySpec`
* `FMABSActivePeriodicEffect`
* shared debug event types
* shared debug event categories
* shared harness summary structs

### `MABSGameplay`

Owns:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* single-ability granting
* grouped set granting
* timer-based startup, delivery, recovery, combo, and periodic scheduling
* authority-side combo queueing
* authority-side AoE resolution
* authority-side instant and periodic effect application
* periodic reapply refresh behavior
* socket transform resolution
* montage request hooks
* authority-side delivery execution
* cue routing helpers for startup, delivery, tracer, travel, and impact
* cooldown and cost integration
* replicated gameplay state
* debug summary query helpers and owner-facing debug state accessors

### `MABSDebug`

Owns:

* runtime-safe debug formatting
* the runtime harness HUD
* category toggles
* recent-event presentation
* harness section layout and filtering

### `MABSEditor`

Owns:

* editor-only extensions
* future validation and authoring helpers

## Phase 9 ownership split

Phase 9 intentionally keeps responsibilities like this:

* `MABSGameplay` exposes and mutates runtime state
* `MABSDebug` decides how that data is shown

That means `UMABSAbilityComponent` still exposes compact query helpers such as:

* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`

The harness UI itself stays in `AMABSDebugHUD`.

## `MABSGameplay` internal split

Phase 9 keeps the public header stable, but moves the private implementation into focused files:

* `MABSAbilityComponent_Core.inl`
* `MABSAbilityComponent_Granting.inl`
* `MABSAbilityComponent_Activation.inl`
* `MABSAbilityComponent_Delivery.inl`
* `MABSAbilityComponent_Effects.inl`
* `MABSAbilityComponent_Debug.inl`
* `MABSAbilityComponent_Presentation.inl`

Those files are still part of the `MABSGameplay` runtime layer. They do not introduce new runtime state owners.

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` runtime data
* `MABSEditor` may depend on runtime modules
