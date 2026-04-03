# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 8, the runtime path supports:

* granting authored abilities
* grouped granting through authored ability sets
* server-authoritative activation
* cooldown and cost validation
* timing through startup, delivery, and recovery
* delivery through `Direct`, `HitTrace`, `Melee`, or `Projectile`
* basic melee combo chaining
* optional AoE target resolution using sphere, box, or capsule shapes
* instant `Damage` and `Heal` effects
* timed `DOT` and `HOT` effects with duration refresh on reapply
* socket-first delivery origins
* optional montage requests
* startup, delivery, tracer, projectile-travel, and impact presentation
* lightweight runtime cue routing with small visibility policies
* structured debug events
* the Phase 8 runtime debug harness

## Why it exists

Most teams need the same baseline gameplay pieces:

* designer-authored data assets
* runtime ability state kept separate from authored data
* authoritative multiplayer behavior
* reusable delivery modes for common combat actions
* common combat breadth such as combos, bursts, burns, and regen
* grouped starting or themed ability authoring
* enough runtime visibility to explain success and failure during gameplay testing

MABS provides that foundation without pulling in a larger framework.

## Current module ownership

* `MABSCore` owns authored data, ability-set data, ability runtime structs, and shared debug summary types
* `MABSGameplay` owns granting, activation, delivery, effects, combo runtime, AoE resolution, periodic timers, cue routing, replication, projectile runtime, and debug summary accessors
* `MABSDebug` owns runtime-safe formatting helpers and the runtime harness HUD
* `MABSEditor` remains the editor-only extension point

## Current ability flow

1. A server-owned actor is granted one or more `UMABSAbilityDefinition` assets directly or through a `UMABSAbilitySet`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, cost, and authored gameplay effect data.
4. Authority enters `Startup`, may open a combo window later, and routes startup presentation if authored.
5. Authority executes the authored delivery mode at the authored delivery time and routes delivery presentation.
6. Single-target delivery resolves the primary impact context.
7. If AoE is enabled, authority gathers and validates the final affected actor set from the authored shape.
8. Authority applies instant effects and optionally starts or refreshes periodic effects on the affected targets.
9. Authority spends cost, starts cooldown, enters `Recovery`, and may auto-trigger a queued combo follow-up when recovery completes.
10. Authority emits categorized debug events and maintains summary state for the runtime harness.

## How to use it

1. Enable the local `MABS` plugin.
2. Create one or more `UMABSAbilityDefinition` assets.
3. Optionally create a `UMABSAbilitySet`.
4. Set target intent, delivery mode, timing, cost, cooldown, and gameplay effect data on each ability.
5. Fill the `Combo`, `AoE`, or `PeriodicEffect` groups only when the ability needs them.
6. Fill the presentation groups that matter for that delivery mode.
7. Add `UMABSAbilityComponent` to the owning actor.
8. Grant the ability or set on authority.
9. Call `TryActivateAbilityByTag`.
10. Optionally set the HUD class to `AMABSDebugHUD` and enable `mabs.DebugHarness 1` for runtime inspection.
