# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 11, the runtime path supports:

* granting authored abilities
* grouped granting through authored ability sets
* server-authoritative activation
* cooldown and cost validation
* timing through startup, delivery, and recovery
* delivery through `Direct`, `HitTrace`, `Melee`, or `Projectile`
* custom delivery handlers in C++ or Blueprint for those delivery modes
* basic melee combo chaining
* optional AoE target resolution using sphere, box, or capsule shapes
* instant `Damage` and `Heal` effects
* timed `DOT` and `HOT` effects with duration refresh on reapply
* socket-first delivery origins
* optional montage requests
* startup, delivery, tracer, projectile-travel, and impact presentation
* lightweight runtime cue routing with small visibility policies
* structured debug events
* the runtime debug harness

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

* `MABSCore` owns authored data, ability-set data, ability runtime structs, shared debug summary types, and the authored delivery-handler class reference on `UMABSAbilityDefinition`
* `MABSGameplay` owns granting, activation, delivery, delivery handlers, effects, combo runtime, AoE resolution, periodic timers, cue routing, replication, projectile runtime, and debug summary accessors through real private runtime units behind `UMABSAbilityComponent`
* `MABSDebug` owns runtime-safe formatting helpers and the runtime harness HUD
* `MABSEditor` remains the editor-only extension point

## Current ability flow

1. A server-owned actor is granted one or more `UMABSAbilityDefinition` assets directly or through a `UMABSAbilitySet`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, cost, and authored gameplay effect data.
4. Authority enters `Startup`, may open a combo window later, and routes startup presentation if authored.
5. Authority resolves the authored or built-in delivery handler at the authored delivery time and routes delivery presentation.
6. The handler resolves the primary impact context and may customize the final affected targets.
7. If AoE is enabled, built-in delivery still gathers and validates the final affected actor set from the authored shape.
8. Authority applies instant effects and optionally starts or refreshes periodic effects on the affected targets.
9. Authority spends cost, starts cooldown, enters `Recovery`, and may auto-trigger a queued combo follow-up when recovery completes.
10. Authority emits categorized debug events and maintains summary state for the runtime harness.

## How to use it

1. Enable the local `MABS` plugin.
2. Create one or more `UMABSAbilityDefinition` assets.
3. Optionally create a `UMABSAbilitySet`.
4. Set target intent, delivery mode, timing, cost, cooldown, and gameplay effect data on each ability.
5. Leave `DeliveryHandlerClass` empty for normal built-in delivery, or assign a custom delivery handler class when one ability needs custom delivery behavior.
6. Fill the `Combo`, `AoE`, or `PeriodicEffect` groups only when the ability needs them.
7. Fill the presentation groups that matter for that delivery mode.
8. Add `UMABSAbilityComponent` to the owning actor.
9. Grant the ability or set on authority.
10. Call `TryActivateAbilityByTag`.
11. Optionally set the HUD class to `AMABSDebugHUD` and enable `mabs.DebugHarness 1` for runtime inspection.

## Phase 11 extensibility note

Phase 11 does not change the normal setup path for existing content.

It adds a delivery-handler seam so hit trace, melee, direct, and projectile delivery can be customized in C++ or Blueprint without moving authoritative runtime state off `UMABSAbilityComponent`.
