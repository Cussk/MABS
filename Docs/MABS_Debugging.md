# MABS Debugging

## What it is

This document explains the current Phase 2.5 debugging tools for MABS.

## Why it exists

Logs alone are too slow when you are diagnosing targeting behavior in PIE or multiplayer. Phase 2.5 adds a small runtime-safe debug layer so users can see what the targeting system attempted, what it hit, and why it accepted or rejected the result.

## What is available

Phase 2.5 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* a latest `FMABSTargetTraceDebugInfo` snapshot on the ability component
* optional in-world trace visualization
* `UMABSDebugBlueprintLibrary` formatting and color helpers
* `AMABSDebugHUD` for a lightweight on-screen recent-event overlay

## How to use it

### Structured events

Use these runtime sources:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `LogMABSAbilitySystem`

Common Phase 2.5 event names:

* `RequestStarted`
* `RequestSentToServer`
* `RequestAccepted`
* `TargetTraceStarted`
* `TargetTraceHit`
* `TargetTraceRejected`
* `TargetResolved`
* `TargetResolutionFailed`
* `EffectApplied`
* `EffectApplicationFailed`
* `CommitSucceeded`

### Latest target trace snapshot

Use `GetLatestTargetTraceDebugInfo()` on `UMABSAbilityComponent` to read the newest authoritative actor-target trace state for the local owner.

This is useful when you need to know:

* trace mode
* trace start and end
* hit location
* hit actor and component
* hit distance
* accept or reject result
* human-readable rejection reason

### In-world trace debug draw

Enable these authored fields on an actor-target ability:

* `bDrawTargetTraceDebug = true`
* `TargetTraceDebugDuration > 0`

When enabled, MABS draws:

* the trace line
* start and end spheres for sphere traces
* a hit point marker
* a text label showing the accept or reject reason

### Runtime overlay

`AMABSDebugHUD` is the first lightweight runtime overlay in `MABSDebug`.

What it shows:

* the latest target trace snapshot
* recent MABS events
* color-coded success, in-progress, and failure text

The default Third Person host harness now sets `HUDClass` to `AMABSDebugHUD` through `AMABSGameMode`.

For custom game modes, set the HUD class manually or subclass `AMABSDebugHUD`.

At runtime you can toggle the overlay through:

* `SetOverlayEnabled(bool bEnabled)`
* `ToggleOverlayEnabled()`

## Example

Example fireball debugging workflow:

1. Create an actor-targeted damage ability.
2. Set `ActorTargetTraceMode` to `Sphere`.
3. Set `TargetTraceRadius` to `50`.
4. Enable `bIgnoreNonTargetWorldHits`.
5. Enable `bDrawTargetTraceDebug`.
6. Start PIE with a listen server and one client.
7. Activate the ability while facing a nearby character.
8. Verify:
   * the world shows the trace line and hit marker
   * the overlay shows the latest trace result
   * the event list includes `TargetTraceStarted`, then `TargetTraceHit` or `TargetTraceRejected`

## What this phase still does not include

Phase 2.5 debugging does not yet include:

* a full ability inspector
* cooldown and cost visualization
* tags or granted-ability panels
* a finalized debug harness UI
* deep editor tooling

## Test checklist

Singleplayer:

* self-heal still succeeds
* actor-target abilities show the latest trace result in the overlay
* debug draw appears only when enabled
* invalid world hits show clear rejection reasons

Listen server:

* the host sees authoritative targeting results immediately
* a remote client receives authoritative event history and latest trace state

Dedicated server:

* runtime debug helpers remain client-safe
* no editor-only debug dependencies leak into runtime or server builds
