# MABS Abilities

## What it is

This document explains the current Phase 3 ability model in plain language: what an ability definition is, what granting means, how activation works, and how cooldowns and costs fit into the runtime path.

## Why it exists

MABS is intentionally split into authored data and live runtime state. That split is what makes multiplayer rules, cooldown tracking, targeting, and debugging easier to reason about.

## Ability definitions

`UMABSAbilityDefinition` is the authored asset from `MABSCore`.

It describes what an ability is supposed to do, not what it is doing right now.

Typical authored fields in Phase 3:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`
* `TargetTraceDistance`
* `CooldownSeconds`
* `CooldownGroupTag`
* `ResourceCost`

Designers edit this asset once and reuse it across actors.

## Granted abilities

Granting means an actor receives a runtime entry for a definition.

That runtime entry is `FMABSAbilitySpec`. Each granted spec stores:

* the granted definition reference
* a copied gameplay tag for fast lookup
* a stable handle
* the current runtime state
* the last activation result
* the personal cooldown end time

The runtime component in `MABSGameplay` owns these specs. The data asset does not.

## Activation flow

Activation in Phase 3 means:

1. code or input calls `TryActivateAbilityByTag`
2. the request reaches the authoritative path
3. the component validates the grant and authored data
4. the component checks cooldown and cooldown-group state
5. the component validates resource affordability if `ResourceCost > 0`
6. the component accepts the request
7. the component resolves a target
8. the component applies an instant effect
9. the component spends cost on authority
10. the component starts cooldown on authority
11. the component emits debug events for each step
12. the ability returns to idle

## Runtime state versus authored data

Authored data answers:

* what tag this ability uses
* how designers should label it
* what target style it expects
* what instant effect type it applies
* how strong that effect is
* how long its cooldown should be
* whether it belongs to a shared cooldown group
* how much resource it costs

Runtime state answers:

* was this ability granted on this actor
* is it idle, active, or blocked right now
* what happened the last time activation was attempted
* when its personal cooldown ends
* which shared cooldown groups are active on the owning component

Keeping those separate is what makes the system multiplayer-safe and extensible.

## How to use it

1. Create a `UMABSAbilityDefinition` asset.
2. Set `TargetType` to `Self` or `Actor`.
3. Set `InstantEffectType` to `Damage` or `Heal`.
4. Set `EffectMagnitude` to a positive value.
5. Set `CooldownSeconds` when the ability should lock out after success.
6. Optionally set `CooldownGroupTag` when multiple abilities should share a lockout.
7. Set `ResourceCost` when the owner should pay a cost.
8. Grant it on the authoritative owner with `GrantAbility`.
9. Activate it with `TryActivateAbilityByTag`.
10. Read `FMABSAbilitySpec`, cooldown query helpers, or debug events to inspect the result.

## Example

Example self-heal setup:

1. Create `DA_Test_SelfHeal`.
2. Set `AbilityTag` to `Test.Ability.SelfHeal`.
3. Set `DisplayName` to `Self Heal`.
4. Set `TargetType` to `Self`.
5. Set `InstantEffectType` to `Heal`.
6. Set `EffectMagnitude` to `25`.
7. Set `CooldownSeconds` to `5`.
8. Set `CooldownGroupTag` to `Test.CooldownGroup.Support`.
9. Set `ResourceCost` to `20`.
10. Grant it to the player character on the server.
11. Press an input key that calls `TryActivateAbilityByTag(Test.Ability.SelfHeal)`.
12. Observe `CostValidated`, `EffectApplied`, `CostSpent`, `CooldownStarted`, and `CommitSucceeded`.

## Not included in this phase

Phase 3 still does not include:

* gameplay effect durations
* a full stat or attribute system
* resource regeneration
* cooldown reduction stats
* charge systems
* AoE targeting
* location targeting
* team filtering
* prediction
* projectile handling
* animation systems
