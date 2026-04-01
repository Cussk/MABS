# MABS Modules

## What it is

This document explains the Phase 1.5 module layout for MABS and what each module owns.

## Why it exists

Phase 01 proved the grant-and-activate flow. Phase 1.5 reorganizes that flow into a plugin-first layout so runtime code, debug tooling, and editor-only code can evolve without collapsing back into one module.

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
* use the shared enums and handles in gameplay or debug code

Example:

* a designer-authored fireball asset is a `UMABSAbilityDefinition` from `MABSCore`

### `MABSGameplay`

What it is:

* the primary runtime gameplay module

Why it exists:

* it owns the server-authoritative execution path
* it keeps grant, activation, validation, commit, and replication together

How to use it:

* add `UMABSAbilityComponent` to an actor or character
* grant abilities on authority
* call `TryActivateAbilityByTag`

Example:

* a player character component grants `Test.Ability.Fireball` on the server, then activates it from input

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

* a Blueprint HUD or PIE test harness formats the latest debug event into a readable string

### `MABSEditor`

What it is:

* the editor-only module

Why it exists:

* runtime and server builds should not depend on editor code
* future validation and authoring tools need an isolated home

How to use it:

* users do not need to reference this module directly for runtime gameplay
* it exists for future editor tooling and validation work

Example:

* a future asset validation pass for `UMABSAbilityDefinition` belongs here, not in runtime modules

## Dependency direction

Allowed dependency flow is:

* `MABSCore` depends on no other MABS modules
* `MABSGameplay` depends on `MABSCore`
* `MABSDebug` depends on `MABSCore`
* `MABSEditor` may depend on runtime modules

This keeps runtime code one-way and prevents editor dependencies from leaking into game or server targets.

## Host project versus plugin content

Phase 1.5 separates the reusable plugin from the local development harness.

Current host-project-only items include:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person Blueprint wiring used for local verification

Current plugin runtime responsibility includes:

* reusable code modules only
* shared runtime types
* runtime ability execution
* runtime-safe debug helpers

## Example integration

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset in content.
3. Add `UMABSAbilityComponent` to a character.
4. Grant the ability on authority.
5. Activate by tag and inspect debug events.
