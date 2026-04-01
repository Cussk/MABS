# MABS Targeting

## What it is

This document explains the Phase 2 targeting model for MABS.

## Why it exists

An ability system needs a clear answer to "who is this ability acting on?" Phase 2 introduces the smallest useful targeting slice without turning MABS into a full targeting framework.

## Supported target types

Phase 2 supports:

* `Self`
* `Actor`

`Location` is still out of scope for this phase.

## How `Self` works

`Self` resolves the owning actor directly on authority.

This is the simplest path and is ideal for:

* self-heal
* self-buff placeholders in future phases
* first-pass validation

## How `Actor` works

`Actor` resolves a single actor through a simple forward trace from the owning actor on authority.

The authored `TargetTraceDistance` field controls the max trace distance.

Phase 2 keeps this intentionally small:

* one actor target
* no team filtering
* no AoE
* no projectile system

## Failure behavior

If no valid actor is found for `Actor` targeting:

* the request fails cleanly
* `LastActivationResult` becomes `TargetResolutionFailed`
* a `TargetResolutionFailed` debug event is emitted

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Set `TargetType` to `Self` or `Actor`.
3. If using `Actor`, set `TargetTraceDistance` to a useful test value.
4. Grant the ability on authority.
5. Activate it and inspect the resulting debug events.

## Example

Example actor-targeted damage setup:

1. Create `DA_Test_Fireball`.
2. Set `TargetType` to `Actor`.
3. Set `TargetTraceDistance` to `1500`.
4. Face a valid actor target in front of the owner.
5. Call `TryActivateAbilityByTag(Test.Ability.Fireball)`.
6. Observe `TargetResolved` on success or `TargetResolutionFailed` if the trace misses.
