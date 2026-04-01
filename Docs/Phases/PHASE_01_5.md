# Phase 1.5 - Plugin-First Modular Reorganization

## What it is

Phase 1.5 reorganizes MABS into a plugin-first module layout without changing the Phase 01 gameplay feature set.

The plugin now contains:

* `MABSCore`
* `MABSGameplay`
* `MABSDebug`
* `MABSEditor`

## Why it exists

Phase 01 proved the core multiplayer ability request flow. Before adding targeting, effects, or other Phase 2 work, the project needed a cleaner ownership model for runtime code, debug tooling, and editor-only extensions.

This phase exists to:

* separate foundational runtime data from gameplay execution
* keep debug tooling runtime-safe and modular
* prevent editor dependencies from leaking into runtime and server builds
* make the codebase closer to a marketplace-ready plugin layout

## Added in this phase

### Structure

* added a local `Plugins/MABS/MABS.uplugin`
* created runtime modules `MABSCore`, `MABSGameplay`, and `MABSDebug`
* created editor-only module `MABSEditor`

### Ownership

* moved `UMABSAbilityDefinition` into `MABSCore`
* moved `FMABSAbilityHandle`, `FMABSAbilitySpec`, and shared enums into `MABSCore`
* moved `FMABSAbilityDebugEvent` and `LogMABSAbilitySystem` into `MABSCore`
* moved `UMABSAbilityComponent` into `MABSGameplay`
* added `UMABSDebugBlueprintLibrary` to `MABSDebug`

### Project integration

* enabled the local plugin in `MABS.uproject`
* added Unreal core redirects so existing content can still resolve the moved types
* updated `.gitignore` to ignore `.idea/`

## Preserved behavior

Phase 1.5 keeps the Phase 01 runtime behavior intact:

* abilities can still be granted from `UMABSAbilityDefinition`
* granted specs still replicate
* activation requests still route remote clients through the server RPC path
* authority still validates and commits activation
* structured debug events still emit for request start, send, accept, reject, and commit

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset.
3. Add `UMABSAbilityComponent` to the owning actor.
4. Grant the ability on authority.
5. Call `TryActivateAbilityByTag`.
6. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or `UMABSDebugBlueprintLibrary::FormatAbilityDebugEvent`.

## Example

Example Phase 1.5 flow:

1. Create `DA_Test_Fireball` with tag `Test.Ability.Fireball`.
2. Add `UMABSAbilityComponent` to the player character.
3. On server `BeginPlay`, call `GrantAbility(DA_Test_Fireball)`.
4. On input, call `TryActivateAbilityByTag(Test.Ability.Fireball)`.
5. In standalone, expect `RequestAccepted` and `CommitSucceeded`.
6. In multiplayer, expect remote clients to see `RequestSentToServer` before receiving the server result.

## Content ownership notes

Plugin runtime code now lives in `Plugins/MABS`.

The current host project still carries development harness content such as:

* `Config/DefaultGameplayTags.ini`
* `Content/Data/DA_Test_Fireball.uasset`
* Third Person test wiring

Those items remain useful for validation, but they are not treated as core plugin runtime content in this phase.

## Not included in this phase

Phase 1.5 still does not add:

* cooldown execution
* resource costs
* targeting logic
* gameplay effects
* prediction
* advanced UI
* editor customizations beyond module bootstrap

## Test checklist

Compile / load:

* plugin modules compile
* editor opens with plugin enabled
* runtime modules load without startup errors

Singleplayer:

* ability definitions still create and open
* ability granting still works
* activation still succeeds for granted abilities
* debug events still record correctly

Listen server:

* host activation still works
* remote client activation still uses the server RPC path
* owning client still receives authoritative debug events

Dedicated server:

* dedicated server target remains present
* runtime code does not depend on `MABSEditor`
* remote client activation request flow remains server-authoritative
