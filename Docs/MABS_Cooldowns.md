# MABS Cooldowns

## What it is

This document explains the Phase 3 cooldown model for MABS.

## Why it exists

Real abilities usually need a lockout after success. Phase 3 adds the smallest useful cooldown slice without introducing charge systems, cooldown modifiers, or a larger combat framework.

## Supported cooldown features

Phase 3 supports:

* per-ability cooldown duration
* optional shared cooldown groups
* authoritative cooldown start after successful commit
* authoritative cooldown denial before target/effect execution
* runtime remaining-time queries

## Authored cooldown data

Cooldowns are authored on `UMABSAbilityDefinition`.

Relevant fields:

* `CooldownSeconds`
* `CooldownGroupTag`

`CooldownSeconds` controls the personal cooldown duration for the granted ability.

`CooldownGroupTag` is optional. When set, multiple abilities can share a lockout on the same owner.

## Runtime cooldown state

Phase 3 tracks cooldown in two places:

* `FMABSAbilitySpec::CooldownEndTime` for personal cooldown
* `UMABSAbilityComponent::CooldownGroupStates` for shared cooldown-group runtime state

This keeps the ownership split readable:

* personal cooldown lives with the granted ability
* shared group cooldown lives with the owning component

## Cooldown behavior

### On activation attempt

Before target resolution and effect execution, MABS checks:

* whether the ability itself is on cooldown
* whether its cooldown group is active

If either is active:

* activation fails cleanly
* `LastActivationResult` becomes `OnCooldown`
* `CooldownRejected` explains the remaining time

### On successful commit

After a successful commit, MABS:

* starts the personal cooldown if `CooldownSeconds > 0`
* starts or updates the shared cooldown group if `CooldownGroupTag` is valid
* emits `CooldownStarted`

## Public runtime queries

`UMABSAbilityComponent` exposes:

* `GetCooldownRemainingByTag(...)`
* `IsAbilityOnCooldown(...)`
* `GetCooldownGroupRemaining(...)`
* `IsCooldownGroupActive(...)`

These helpers are useful for runtime debugging and future gameplay UI.

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Set `CooldownSeconds` to a positive number.
3. Optionally set `CooldownGroupTag`.
4. Grant the ability on authority.
5. Activate it once.
6. Try to activate it again before the cooldown expires.
7. Inspect `CooldownRejected`, `CooldownStarted`, and the cooldown query helpers.

## Example

Example shared cooldown setup:

1. Create `DA_Test_SelfHeal`.
2. Set `CooldownSeconds` to `5`.
3. Set `CooldownGroupTag` to `Test.CooldownGroup.Support`.
4. Create a second support ability that also uses `Test.CooldownGroup.Support`.
5. Grant both to the same owner.
6. Activate the first ability.
7. Verify that the second ability is also denied while the shared group is active.

## Not included in this phase

Phase 3 cooldowns do not yet include:

* cooldown reduction stats
* charge systems
* haste or scaling formulas
* per-level cooldown tuning systems
* UI beyond the existing runtime debug surfaces
