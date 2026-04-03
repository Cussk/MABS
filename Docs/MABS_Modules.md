# MABS Modules

## What it is

This document explains the Phase 8 module layout for MABS.

Phase 8 keeps the existing gameplay split, but moves the runtime debug harness more clearly into `MABSDebug` while `MABSGameplay` exposes small query helpers and summary read models.

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

## Phase 8 ownership split

Phase 8 intentionally keeps responsibilities like this:

* `MABSGameplay` exposes data
* `MABSDebug` decides how that data is shown

That means `UMABSAbilityComponent` now exposes compact query helpers such as:

* `GetGrantedAbilityDebugSummaries()`
* `GetCooldownGroupDebugSummaries()`
* `GetComboDebugSummary()`
* `GetPeriodicEffectDebugSummaries()`

The harness UI itself stays in `AMABSDebugHUD`.

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` runtime data
* `MABSEditor` may depend on runtime modules
