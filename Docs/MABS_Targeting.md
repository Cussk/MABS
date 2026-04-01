# MABS Targeting

## What it is

This document explains the current Phase 2.5 targeting model for MABS.

## Why it exists

Phase 2 proved that server-authoritative target resolution worked, but the first `Actor` targeting slice was still easy to mistrust during playtesting. Phase 2.5 improves that path without expanding into a full combat targeting framework.

## Supported target types

Phase 2.5 supports:

* `Self`
* `Actor`

`Location` is still out of scope for this phase.

## How `Self` works

`Self` resolves the owning actor directly on authority.

This remains the simplest path and is ideal for:

* self-heal
* self-buff placeholders in future phases
* first-pass validation

## How `Actor` works now

`Actor` resolves a single actor target on authority through a configurable trace.

Phase 2.5 improvements:

* `ActorTargetTraceMode` supports `Line` and `Sphere`
* aim direction comes from the controller viewpoint when possible
* the actual gameplay trace starts from `GetActorEyesViewPoint`, which keeps the query near the owner instead of behind a third-person camera boom
* target validation is centralized in gameplay code
* trace attempts emit richer debug events
* the latest trace snapshot is stored for runtime overlay use

Relevant authored fields:

* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`

Recommended defaults for a general-purpose actor-target ability:

* `ActorTargetTraceMode = Sphere`
* `TargetTraceRadius = 50`
* `bRequireValidActorTarget = true`
* `bIgnoreNonTargetWorldHits = true`

## Valid-target rules

When `bRequireValidActorTarget` is enabled, MABS validates the hit actor before target resolution succeeds.

Current Phase 2.5 rules:

* self is rejected for `Actor` targeting
* damage abilities accept actors that can be damaged or are pawns
* heal abilities accept actors that implement `IMABSInstantEffectReceiver`

This keeps validation centralized while staying lightweight.

## World-hit behavior

If the trace hits world geometry or another non-target result:

* MABS emits a readable rejection event
* the latest trace snapshot stores the rejection reason
* if `bIgnoreNonTargetWorldHits` is enabled, the trace can continue evaluating later hits returned by the same query

This improves feedback when the trace shape brushes floor or environment during testing.

## Debug visibility

Phase 2.5 actor targeting adds:

* `TargetTraceStarted`
* `TargetTraceHit`
* `TargetTraceRejected`
* `TargetResolved`
* `TargetResolutionFailed`

If `bDrawTargetTraceDebug` is enabled, MABS also draws:

* the trace line
* start and end spheres for sphere traces
* a hit marker
* a text label with the accept or reject reason

The owning component stores the latest trace snapshot in `GetLatestTargetTraceDebugInfo()`.

## Failure behavior

If no valid actor is found for `Actor` targeting:

* the request fails cleanly
* `LastActivationResult` becomes `TargetResolutionFailed`
* `TargetTraceRejected` and/or `TargetResolutionFailed` explain the reason
* the latest trace snapshot records what was hit and why it was rejected

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Set `TargetType` to `Self` or `Actor`.
3. If using `Actor`, set `TargetTraceDistance`.
4. Choose `ActorTargetTraceMode`.
5. If using `Sphere`, set `TargetTraceRadius` to a non-zero value.
6. Decide whether `bRequireValidActorTarget` and `bIgnoreNonTargetWorldHits` should be enabled.
7. Optionally enable `bDrawTargetTraceDebug`.
8. Grant the ability on authority.
9. Activate it and inspect the resulting events and latest trace snapshot.

## Example

Example actor-targeted damage setup:

1. Create `DA_Test_Fireball`.
2. Set `TargetType` to `Actor`.
3. Set `TargetTraceDistance` to `1500`.
4. Set `ActorTargetTraceMode` to `Sphere`.
5. Set `TargetTraceRadius` to `50`.
6. Enable `bRequireValidActorTarget`, `bIgnoreNonTargetWorldHits`, and `bDrawTargetTraceDebug`.
7. Face a valid actor target in front of the owner.
8. Call `TryActivateAbilityByTag(Test.Ability.Fireball)`.
9. Observe `TargetTraceStarted`, then either `TargetTraceHit` plus `TargetResolved`, or `TargetTraceRejected` plus `TargetResolutionFailed`.

## Not included in this phase

Phase 2.5 still does not add:

* AoE targeting
* location targeting
* projectiles
* team or faction filters
* advanced target query objects
* a full combat targeting framework
