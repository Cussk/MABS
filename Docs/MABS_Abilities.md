# MABS Abilities

## What it is

This document explains the Phase 01 ability model in plain language: what an ability definition is, what granting means, what activation means, and how runtime state differs from authored data.

## Why it exists

MABS is intentionally split into authored data and live runtime state. Without that split, multiplayer rules and debugging become much harder to reason about.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset from `MABSCore`.

It describes what an ability is, not what it is doing right now.

Typical authored fields in Phase 01:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* placeholder cost and cooldown data

Designers edit this asset once and reuse it across actors.

## Granted abilities

Granting means an actor receives a runtime entry for a definition.

That runtime entry is `FMABSAbilitySpec`. Each granted spec stores:

* the granted definition reference
* a copied gameplay tag for fast lookup
* a stable handle
* the current runtime state
* the last activation result

The runtime component in `MABSGameplay` owns these specs. The data asset does not.

## Activation

Activation in Phase 01 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches the authoritative path
3. the component validates the request
4. the component commits success or records a failure reason
5. debug events show what happened

Successful activation currently stops at a lightweight commit. It does not apply damage, cooldowns, or costs yet.

## Runtime state versus authored data

Authored data answers:

* what tag this ability uses
* how designers should label it
* what target style it expects

Runtime state answers:

* was this ability granted on this actor
* is it idle, active, or blocked right now
* what happened the last time activation was attempted

Keeping those separate is what makes the system multiplayer-safe and extensible.

## How to use it

1. Create a `UMABSAbilityDefinition` asset.
2. Grant it on the authoritative owner with `GrantAbility`.
3. Activate it with `TryActivateAbilityByTag`.
4. Optionally use `SetAbilityBlockedByTag` to force the runtime state to `Blocked`.
5. Read `FMABSAbilitySpec` or debug events to inspect the result.

## Example

Example ability setup:

1. Create `DA_TestFireball`.
2. Set `AbilityTag` to `Test.Ability.Fireball`.
3. Set `DisplayName` to `Fireball`.
4. Grant it to the player character on the server.
5. Press an input key that calls `TryActivateAbilityByTag(Test.Ability.Fireball)`.
6. Observe `RequestAccepted` and `CommitSucceeded` in the log for a successful activation.

## Not included in this phase

Phase 01 does not yet include:

* gameplay effects
* real cooldown execution
* cost payment
* prediction
* targeting resolution
* animation handling
