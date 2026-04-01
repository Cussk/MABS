# Phase 2.5 - Targeting Polish and Runtime Debug Visibility

## What it is

Phase 2.5 improves the usability and inspectability of the Phase 2 targeting and instant-effect slice.

This phase adds:

* configurable line or sphere actor traces
* centralized actor-target validation
* clearer target rejection behavior
* richer target trace debug events
* optional in-world trace debug drawing
* a first lightweight runtime debug overlay

## Why it exists

Phase 2 proved the authority-side effect pipeline worked, but the first actor-target path was still hard to trust during live testing. A technically correct trace is not enough if users cannot tell:

* what the trace attempted
* what it hit
* why the hit was rejected
* what the authoritative result was on a remote client

Phase 2.5 fixes that class of problem before later phases add more combat complexity.

## Added in this phase

### Core data

Added:

* `EMABSTargetTraceMode`
* `FMABSTargetTraceDebugInfo`

Expanded `UMABSAbilityDefinition` with:

* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`
* `bDrawTargetTraceDebug`
* `TargetTraceDebugDuration`

### Gameplay execution

Added or changed:

* controller-viewpoint-first trace origin selection
* line and sphere trace support for actor targeting
* centralized target validation for damage and heal abilities
* `TargetTraceStarted`, `TargetTraceHit`, and `TargetTraceRejected`
* latest trace snapshot storage on `UMABSAbilityComponent`
* client mirroring of the authoritative latest trace snapshot
* optional runtime trace debug drawing

### Debug tooling

Added:

* `UMABSDebugBlueprintLibrary::FormatTargetTraceDebugInfo`
* event and trace color helpers
* `AMABSDebugHUD`

### Host project harness

Added:

* default `AMABSGameMode` wiring to `AMABSDebugHUD`

## How to use it

1. Create or update an actor-target ability definition.
2. Configure `ActorTargetTraceMode`, `TargetTraceRadius`, and `TargetTraceDistance`.
3. Enable `bRequireValidActorTarget` to reject non-usable hit actors.
4. Enable `bIgnoreNonTargetWorldHits` to make testing more forgiving when the same query returns later valid actor hits.
5. Enable `bDrawTargetTraceDebug` when you want in-world visualization.
6. Activate the ability and inspect:
   * recent debug events
   * the latest trace snapshot
   * the runtime overlay

## Example

Example fireball verification flow:

1. Use a `Damage` ability with `TargetType = Actor`.
2. Set `ActorTargetTraceMode = Sphere`.
3. Set `TargetTraceRadius = 50`.
4. Set `TargetTraceDistance = 1500`.
5. Enable `bIgnoreNonTargetWorldHits` and `bDrawTargetTraceDebug`.
6. Place a second character in front of the player.
7. Activate the ability in PIE.
8. Verify that:
   * the trace is drawn in the world
   * `TargetTraceStarted` is followed by `TargetTraceHit` and `TargetResolved`
   * the overlay shows the latest trace result and recent events

## Not included in this phase

Phase 2.5 still does not add:

* cooldown execution
* resource cost payment
* projectiles
* AoE targeting
* location targeting
* advanced team filtering
* a full final debug harness
* gameplay UI beyond the debug overlay

## Test checklist

Singleplayer:

* self-heal still succeeds
* actor-target damage resolves a valid nearby character more reliably
* invalid world hits are rejected with readable reasons
* trace debug drawing appears when enabled
* the runtime overlay shows recent target and effect events

Listen server:

* host targeting uses the authority path directly
* remote client targeting still resolves on authority
* the owning remote client receives the authoritative trace snapshot and event feed

Dedicated server:

* target resolution remains authoritative
* runtime overlay code remains client-safe
* no editor-only dependency leaks into runtime or server targets

## Deliverables

Phase 2.5 delivers:

1. Improved actor targeting behavior
2. Better target validation and rejection reporting
3. Runtime trace visualization
4. A first lightweight on-screen debug overlay
5. Updated user-facing documentation
