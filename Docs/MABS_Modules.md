# MABS Modules

## What it is

This document explains the current module layout for MABS and what each module owns after Phase 2.5 targeting polish and runtime debug visibility.

## Why it exists

Phase 01 proved the grant-and-activate flow. Phase 1.5 reorganized that flow into a plugin-first layout. Phase 2 added targeting and instant effects. Phase 2.5 keeps those layers separate while improving targeting behavior and adding runtime debugging without collapsing everything into one module.

## Module map

### `MABSCore`

What it is:

* the foundational runtime module

Why it exists:

* it keeps shared authored data and runtime types stable
* it lets higher-level modules build on one clean dependency

How to use it:

* create `UMABSAbilityDefinition` assets
* read `FMABSAbilitySpec`, `FMABSAbilityDebugEvent`, and `FMABSTargetTraceDebugInfo`
* use shared enums such as `EMABSTargetType`, `EMABSTargetTraceMode`, and `EMABSInstantEffectType`

Example:

* a designer-authored fireball asset stores its target trace mode, radius, and debug-draw settings in `MABSCore` data

### `MABSGameplay`

What it is:

* the primary runtime gameplay module

Why it exists:

* it owns the server-authoritative execution path
* it keeps grant, activation, target resolution, target validation, effect application, and replication together

How to use it:

* add `UMABSAbilityComponent` to an actor or character
* grant abilities on authority
* call `TryActivateAbilityByTag`
* read `GetLatestTargetTraceDebugInfo()` when you need the newest authoritative actor-target trace result
* optionally implement `IMABSInstantEffectReceiver` on actors that should accept MABS heals

Example:

* a player character component grants `Test.Ability.Fireball`, runs a sphere trace from the controller viewpoint, validates the hit actor, stores the latest trace snapshot, and applies damage on authority

### `MABSDebug`

What it is:

* the runtime-safe debug tooling module

Why it exists:

* it keeps debug consumers out of the core gameplay execution path
* it provides a place for formatting helpers and the first lightweight runtime overlay

How to use it:

* format `FMABSAbilityDebugEvent` and `FMABSTargetTraceDebugInfo`
* use `AMABSDebugHUD` for a simple recent-events and latest-trace overlay

Example:

* the Third Person host harness uses `AMABSDebugHUD` to show the newest trace result and recent MABS events in PIE

### `MABSEditor`

What it is:

* the editor-only module

Why it exists:

* runtime and server builds should not depend on editor code
* future validation and authoring tools need an isolated home

How to use it:

* users do not need to reference this module directly for runtime gameplay
* it exists for future editor tooling and validation work

## Dependency direction

Current dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore` and consumes `MABSGameplay` data for the runtime overlay
* `MABSEditor` may depend on runtime modules

This keeps runtime code one-way and prevents editor dependencies from leaking into game or server targets.

## Host project versus plugin content

The plugin remains the reusable runtime product. The host project still contains the local verification harness.

Current host-project-only items include:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint wiring used for local verification
* the example heal receiver state on `AMABSCharacter`
* the default `AMABSGameMode` wiring that opts into `AMABSDebugHUD`

Current plugin runtime responsibility includes:

* reusable code modules only
* shared runtime types and ability definitions
* runtime ability execution
* runtime-safe debug formatting helpers
* the reusable debug HUD overlay

## Example integration

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset in content.
3. Add `UMABSAbilityComponent` to a character.
4. Grant the ability on authority.
5. Activate by tag and inspect recent debug events and the latest trace snapshot through the overlay or Blueprint helpers.
