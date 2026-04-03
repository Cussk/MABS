# MABS Modules

## What it is

This document explains the Phase 9.5 module layout for MABS.

Phase 9.5 keeps the module ownership from earlier phases, but replaces the old `.inl`-based component decomposition with real private runtime implementation units inside `MABSGameplay`.

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
* granting runtime
* activation and combo runtime
* delivery and targeting runtime
* effects, cooldown, cost, and periodic runtime
* presentation routing runtime
* debug summary and event assembly runtime
* replicated gameplay state

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

## `MABSGameplay` internal split

Phase 9.5 keeps `UMABSAbilityComponent` as the public facade, but the private runtime now lives in:

* `MABSAbilityRuntime_Internal.h`
* `MABSAbilityRuntime_Internal.cpp`
* `MABSAbilityRuntime_Core.cpp`
* `MABSAbilityRuntime_Granting.cpp`
* `MABSAbilityRuntime_Activation.cpp`
* `MABSAbilityRuntime_Delivery.cpp`
* `MABSAbilityRuntime_Effects.cpp`
* `MABSAbilityRuntime_Presentation.cpp`
* `MABSAbilityRuntime_Debug.cpp`

That is now the primary refactor story, not a large component source file with included fragments.

## Ownership split

Phase 9.5 intentionally keeps responsibilities like this:

* `MABSGameplay` owns gameplay truth and runtime read models
* `MABSDebug` decides how that runtime data is displayed

That means `UMABSAbilityComponent` still exposes compact query helpers such as:

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
