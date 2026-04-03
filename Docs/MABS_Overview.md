# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 6, the runtime path supports:

* granting authored abilities
* server-authoritative activation
* cooldown and cost validation
* timing through startup, delivery, and recovery
* delivery through `Direct`, `HitTrace`, `Melee`, or `Projectile`
* instant `Damage` and `Heal` effects
* socket-first delivery origins
* optional montage requests
* startup, delivery, tracer, projectile-travel, and impact presentation
* structured debug events and the runtime overlay

## Why it exists

Most teams need the same baseline gameplay pieces:

* designer-authored data assets
* runtime ability state kept separate from authored data
* authoritative multiplayer behavior
* reusable delivery modes for common combat actions
* enough runtime visibility to explain success and failure
* a lightweight built-in presentation layer

MABS provides that foundation without pulling in a larger framework.

## Current module ownership

* `MABSCore` owns authored data and shared runtime types
* `MABSGameplay` owns granting, activation, delivery, effects, costs, cooldowns, presentation routing, replication, and projectile runtime
* `MABSDebug` owns runtime-safe formatting helpers and the HUD overlay
* `MABSEditor` remains the editor-only extension point

## Current ability flow

1. A server-owned actor is granted a `UMABSAbilityDefinition`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, and cost rules.
4. Authority enters `Startup` and triggers startup presentation if authored.
5. Authority executes the authored delivery mode at the authored delivery time and triggers delivery presentation.
6. `HitTrace` may spawn a tracer and `Projectile` may start travel presentation from the projectile actor.
7. `Direct`, `HitTrace`, and `Melee` apply the instant effect immediately on success, then trigger impact presentation.
8. `Projectile` commits on authoritative spawn, then applies the effect and impact presentation later on authoritative impact.
9. Authority emits debug events for request, delivery, presentation, effect, cost, cooldown, and commit steps.

## Host-project content

The plugin is the reusable product. The current host project still supplies the local verification harness:

* gameplay tags in `Config/DefaultGameplayTags.ini`
* example content assets
* `AMABSCharacter` sample health and resource state
* `AMABSGameMode` wiring for the debug HUD overlay

## How to use it

1. Enable the local `MABS` plugin.
2. Create a `UMABSAbilityDefinition`.
3. Set target intent, effect data, cooldown, cost, and `DeliveryMode`.
4. Add `UMABSAbilityComponent` to the owning actor.
5. Grant the ability on authority.
6. Call `TryActivateAbilityByTag`.
7. Verify the result through recent debug events and the overlay.
