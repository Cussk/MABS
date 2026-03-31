# Phase 00 - Foundation

## Goal

Phase 00 establishes the first usable MABS architecture slice inside the runtime module.

The goal is not full gameplay abilities. The goal is a clean foundation that already separates authored data, runtime state, execution ownership, and debug visibility.

## Added in this phase

### Source

* `UMABSAbilityDefinition` for authored ability data
* `UMABSAbilityComponent` for runtime ownership
* `FMABSAbilityHandle` for lightweight runtime IDs
* `FMABSAbilitySpec` for granted ability state
* `FMABSAbilityDebugEvent` for structured debug output
* `LogMABSAbilitySystem` for ability-specific logging

### Behavior

* abilities can be granted on the authoritative owner
* granted abilities receive stable runtime handles
* activation requests can be made by gameplay tag
* activation currently stops at a stub result of `NotImplemented`
* every grant or activation path emits a structured debug event

## Why it exists

This phase gives later work a stable place to live. Without this slice, future activation, cost, cooldown, and effect logic would have no clear separation between designer-authored data and live runtime state.

## How to use it

1. Add `UMABSAbilityComponent` to an actor or character.
2. Create one or more `MABSAbilityDefinition` assets.
3. On the server, call `GrantAbility`.
4. Call `TryActivateAbilityByTag` with the authored tag.
5. Inspect `OnAbilityDebugEvent` or `GetRecentDebugEvents`.

## Example

Example Phase 00 flow:

1. Create a `DA_Fireball` ability definition with tag `Ability.Fireball`.
2. Add `UMABSAbilityComponent` to the player character.
3. On authoritative `BeginPlay`, call `GrantAbility(DA_Fireball)`.
4. On input, call `TryActivateAbilityByTag(Ability.Fireball)`.
5. Expect a debug event showing `ActivationRequested` with result `NotImplemented`.

## Not included in this phase

* RPC activation flow
* replicated runtime ability state
* cooldown execution
* cost payment
* targeting logic
* gameplay effects
* prediction

## Test checklist

Singleplayer:

* module compiles
* ability component can be attached to an actor
* ability definition assets can be created
* grant requests add a runtime spec
* activation requests emit a debug event

Listen server:

* server grants succeed
* client grant attempts are rejected with `RequiresAuthority`
* activation stubs log correctly on the server

Dedicated server:

* `MABSServer.Target.cs` is present for dedicated server builds
* actual dedicated server compilation requires a source-built Unreal Engine distribution
* no editor-only dependencies are required by the ability types
* grant and activation stub paths execute without content dependencies
