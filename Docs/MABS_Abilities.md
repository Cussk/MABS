# MABS Abilities

## What it is

This document explains the Phase 2 ability model in plain language: what an ability definition is, what granting means, how activation works, and how targeting and instant effects fit into the runtime path.

## Why it exists

MABS is intentionally split into authored data and live runtime state. That split is what makes multiplayer rules, targeting, and debugging easier to reason about.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset from `MABSCore`.

It describes what an ability is supposed to do, not what it is doing right now.

Typical authored fields in Phase 2:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`
* `TargetTraceDistance`

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

## Activation flow

Activation in Phase 2 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches the authoritative path
3. the component validates the grant and authored data
4. the component accepts the request
5. the component resolves a target
6. the component applies an instant effect
7. the component emits debug events for each step
8. the ability returns to idle

Successful activation no longer stops at a lightweight commit. It now produces a small gameplay result.

## Runtime state versus authored data

Authored data answers:

* what tag this ability uses
* how designers should label it
* what target style it expects
* what instant effect type it applies
* how strong that effect is

Runtime state answers:

* was this ability granted on this actor
* is it idle, active, or blocked right now
* what happened the last time activation was attempted

Keeping those separate is what makes the system multiplayer-safe and extensible.

## How to use it

1. Create a `UMABSAbilityDefinition` asset.
2. Set `TargetType` to `Self` or `Actor`.
3. Set `InstantEffectType` to `Damage` or `Heal`.
4. Set `EffectMagnitude` to a positive value.
5. Grant it on the authoritative owner with `GrantAbility`.
6. Activate it with `TryActivateAbilityByTag`.
7. Read `FMABSAbilitySpec` or debug events to inspect the result.

## Example

Example self-heal setup:

1. Create `DA_Test_SelfHeal`.
2. Set `AbilityTag` to `Test.Ability.SelfHeal`.
3. Set `DisplayName` to `Self Heal`.
4. Set `TargetType` to `Self`.
5. Set `InstantEffectType` to `Heal`.
6. Set `EffectMagnitude` to `25`.
7. Grant it to the player character on the server.
8. Press an input key that calls `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
9. Observe `TargetResolved`, `EffectApplied`, and `CommitSucceeded` in the log for a successful activation.

## Not included in this phase

Phase 2 still does not include:

* gameplay effect durations
* cooldown execution
* cost payment
* AoE targeting
* location targeting
* team filtering
* prediction
* projectile handling
* animation systems
