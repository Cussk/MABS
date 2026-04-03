# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 6.5, the runtime path supports:

* granting authored abilities
* server-authoritative activation
* cooldown and cost validation
* timing through startup, delivery, and recovery
* delivery through `Direct`, `HitTrace`, `Melee`, or `Projectile`
* instant `Damage` and `Heal` effects
* socket-first delivery origins
* optional montage requests
* startup, delivery, tracer, projectile-travel, and impact presentation
* lightweight runtime cue routing with small visibility policies
* pooling-friendly Niagara reuse for repeated world cues and tracers
* structured debug events and the runtime overlay

## Why it exists

Most teams need the same baseline gameplay pieces:

* designer-authored data assets
* runtime ability state kept separate from authored data
* authoritative multiplayer behavior
* reusable delivery modes for common combat actions
* enough runtime visibility to explain success and failure
* a practical presentation layer that stays lightweight

MABS provides that foundation without pulling in a larger framework.

## Current module ownership

* `MABSCore` owns authored data and shared runtime cue types
* `MABSGameplay` owns granting, activation, delivery, effects, costs, cooldowns, cue routing, replication, projectile runtime, and local cue realization
* `MABSDebug` owns runtime-safe formatting helpers and the HUD overlay
* `MABSEditor` remains the editor-only extension point

## Current ability flow

1. A server-owned actor is granted a `UMABSAbilityDefinition`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, and cost rules.
4. Authority enters `Startup` and routes startup presentation if authored.
5. Authority executes the authored delivery mode at the authored delivery time and routes delivery presentation.
6. `HitTrace` may route a tracer cue and `Projectile` may route travel presentation from the replicated projectile actor.
7. `Direct`, `HitTrace`, and `Melee` apply the instant effect immediately on success, then route impact presentation.
8. `Projectile` commits on authoritative spawn, then applies the effect and routes impact presentation later on authoritative impact.
9. Authority emits debug events for request, delivery, cue routing, effect, cost, cooldown, and commit steps.

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition`.
3. Set target intent, effect data, cooldown, cost, timing, and `DeliveryMode`.
4. Fill the presentation groups that matter for the delivery mode.
5. Set cue visibility policies only when a cue should not use the default relevant-client route.
6. Add `UMABSAbilityComponent` to the owning actor.
7. Grant the ability on authority.
8. Call `TryActivateAbilityByTag`.
