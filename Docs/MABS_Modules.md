# MABS Modules

## What it is

This document explains the Phase 2 module layout for MABS and what each module owns.

## Why it exists

Phase 01 proved the grant-and-activate flow. Phase 1.5 reorganized that flow into a plugin-first layout. Phase 2 keeps that structure and adds targeting and instant effects without collapsing responsibilities back together.

## Module map

### `MABSCore`

What it is:

* the foundational runtime module

Why it exists:

* it keeps shared authored data and runtime types stable
* it lets higher-level modules build on one clean dependency

How to use it:

* create `UMABSAbilityDefinition` assets
* read `FMABSAbilitySpec` and `FMABSAbilityDebugEvent`
* use shared enums such as `EMABSTargetType` and `EMABSInstantEffectType`

Example:

* a designer-authored self-heal asset lives entirely in `MABSCore` data

### `MABSGameplay`

What it is:

* the primary runtime gameplay module

Why it exists:

* it owns the server-authoritative execution path
* it keeps grant, activation, target resolution, effect application, and replication together

How to use it:

* add `UMABSAbilityComponent` to an actor or character
* grant abilities on authority
* call `TryActivateAbilityByTag`
* optionally implement `IMABSInstantEffectReceiver` on actors that should accept MABS heals

Example:

* a player character component grants `Test.Ability.SelfHeal` on the server, then activates it from input and heals the owner

### `MABSDebug`

What it is:

* the runtime-safe debug tooling module

Why it exists:

* it keeps debug consumers out of the core gameplay execution path
* it provides a place for helper libraries and future runtime-safe overlays

How to use it:

* consume `FMABSAbilityDebugEvent`
* format events with `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`

Example:

* a Blueprint HUD or PIE test harness formats the latest target/effect debug event into a readable string

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

Allowed dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore`
* `MABSEditor` may depend on runtime modules

This keeps runtime code one-way and prevents editor dependencies from leaking into game or server targets.

## Host project versus plugin content

Phase 2 still separates the reusable plugin from the local development harness.

Current host-project-only items include:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint wiring used for local verification
* the example heal receiver state on `AMABSCharacter`

Current plugin runtime responsibility includes:

* reusable code modules only
* shared runtime types and ability definitions
* runtime ability execution
* runtime-safe debug helpers

## Example integration

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset in content.
3. Add `UMABSAbilityComponent` to a character.
4. Grant the ability on authority.
5. Activate by tag and inspect debug events for target and effect steps.
