# MABS Debugging

## What it is

This document explains the current Phase 3 debugging tools for MABS.

## Why it exists

Cooldown and cost denial are only useful if users can tell why activation failed. Phase 3 extends the runtime-safe debug layer so users can see cooldown start, cooldown rejection, cost validation, cost spending, and cost denial alongside the existing target and effect events.

## What is available

Phase 3 debugging includes:

* structured `FMABSAbilityDebugEvent` output
* a latest `FMABSTargetTraceDebugInfo` snapshot on the ability component
* granted ability runtime summaries with cooldown remaining
* cooldown query helpers on `UMABSAbilityComponent`
* optional in-world trace visualization
* `UMABSDebugBlueprintLibrary` formatting and color helpers
* `AMABSDebugHUD` for a lightweight on-screen overlay

## How to use it

### Structured events

Use these runtime sources:

* `OnAbilityDebugEvent`
* `GetRecentDebugEvents()`
* `LogMABSAbilitySystem`

Important Phase 3 event names:

* `CooldownRejected`
* `CooldownStarted`
* `CostValidated`
* `CostRejected`
* `CostSpent`
* `TargetResolved`
* `EffectApplied`
* `CommitSucceeded`

### Cooldown queries

Use these public helpers on `UMABSAbilityComponent`:

* `GetCooldownRemainingByTag(...)`
* `IsAbilityOnCooldown(...)`
* `GetCooldownGroupRemaining(...)`
* `IsCooldownGroupActive(...)`

These helpers report the authoritative remaining time based on the replicated ability and cooldown-group runtime state.

### Latest target trace snapshot

Use `GetLatestTargetTraceDebugInfo()` on `UMABSAbilityComponent` to read the newest authoritative actor-target trace state for the local owner.

### Runtime overlay

`AMABSDebugHUD` is the current lightweight runtime overlay in `MABSDebug`.

What it shows:

* the latest target trace snapshot
* granted ability runtime summaries, including cooldown remaining
* recent MABS events
* color-coded success, cooldown-active, and failure text

The default Third Person host harness still sets `HUDClass` to `AMABSDebugHUD` through `AMABSGameMode`.

### Blueprint helper formatting

`UMABSDebugBlueprintLibrary` now formats:

* individual debug events
* latest target trace snapshots
* granted ability runtime summaries

This is useful if you want a custom widget instead of the built-in HUD overlay.

## Example

Example cooldown and cost debugging workflow:

1. Create a self-heal ability with `CooldownSeconds = 5` and `ResourceCost = 20`.
2. Implement `IMABSCostReceiver` on the owning actor or use the host-project `AMABSCharacter` example.
3. Start PIE.
4. Activate the ability once.
5. Verify:
   * `CostValidated`
   * `TargetResolved`
   * `EffectApplied`
   * `CostSpent`
   * `CooldownStarted`
   * `CommitSucceeded`
6. Activate it again immediately.
7. Verify `CooldownRejected` and a readable remaining-time message.
8. Reduce the owner resource below the cost and activate after cooldown expires.
9. Verify `CostRejected` and a readable insufficient-resource message.

## What this phase still does not include

Phase 3 debugging does not yet include:

* a full ability inspector
* attribute or resource bars managed by the plugin
* cooldown timelines beyond simple remaining-time text
* a finalized debug harness UI
* deep editor tooling

## Test checklist

Singleplayer:

* self-heal starts cooldown after success
* self-heal cannot be reactivated until cooldown expires
* cost denial produces readable debug output
* cooldown denial produces readable debug output
* the overlay shows cooldown remaining on granted abilities

Listen server:

* the host sees authoritative cooldown and cost results immediately
* a remote client receives authoritative cooldown/cost denial events

Dedicated server:

* runtime debug helpers remain client-safe
* no editor-only debug dependencies leak into runtime or server builds
