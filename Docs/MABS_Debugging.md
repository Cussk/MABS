# MABS Debugging

## What it is

This document explains the current Phase 6.5 debugging tools for MABS.

## Why it exists

Phase 6.5 adds cue routing, visibility policy decisions, and lightweight reuse behavior. Those cosmetic rules need to stay visible, especially in multiplayer, or the presentation path becomes hard to reason about.

## What is available

Phase 6.5 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* the latest `FMABSTargetTraceDebugInfo` snapshot for target traces, hit traces, and melee sweeps
* granted ability summaries with runtime timing state and cooldown remaining
* an owning-client debug replication toggle on `UMABSAbilityComponent`
* routing-focused cue events
* optional trace and sweep debug drawing
* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important event names

Timing and delivery events:

* `StartupStarted`
* `DeliveryScheduled`
* `DeliveryTriggered`
* `RecoveryStarted`
* `RecoveryCompleted`

Cue routing events:

* `PresentationCueRouted`
* `PresentationCueSkipped`
* `PresentationCueRealized`
* `PresentationCuePolicyFallbackUsed`
* `TracerCueRouted`
* `TracerCueSkipped`
* `TracerCueRealized`

Phase-specific presentation events that still remain:

* `StartupPresentationTriggered`
* `DeliveryPresentationTriggered`
* `ImpactPresentationTriggered`
* `ProjectileTravelPresentationTriggered`
* `TracerSpawned`
* `TracerSpawnFailed`

Socket and fallback events:

* `SocketResolved`
* `SocketFallbackUsed`
* `PresentationSocketFallbackUsed`

## How to use it

Read:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`
* `LogMABSAbilitySystem`

For cue problems, look for this order:

1. the phase-specific trigger event
2. the routed or skipped cue event
3. the realized event on the local instance
4. any fallback event if policy or socket rules changed the path

## Example

Example rifle-shot cue flow:

* `StartupStarted`
* `StartupPresentationTriggered`
* `PresentationCueRouted`
* `PresentationCueRealized`
* `DeliveryTriggered`
* `DeliveryPresentationTriggered`
* `PresentationCueRouted`
* `TracerCueRouted`
* `TracerCueRealized`
* `HitTraceHit`
* `ImpactPresentationTriggered`
* `PresentationCueRouted`
* `PresentationCueRealized`
