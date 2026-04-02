# MABS Overview

## What MABS is

MABS is a plugin-first, multiplayer-ready, data-driven ability framework for Unreal Engine.

As of Phase 4, the runtime path supports:

* granting authored abilities
* server-authoritative activation
* cooldown and cost validation
* direct target resolution
* delivery through `Direct`, `HitTrace`, `Melee`, or `Projectile`
* instant `Damage` and `Heal` effects
* structured debug events and the runtime overlay

## Why it exists

Most teams need the same baseline gameplay pieces:

* designer-authored data assets
* runtime ability state kept separate from authored data
* authoritative multiplayer behavior
* reusable delivery modes for common combat actions
* enough runtime visibility to explain success and failure

MABS provides that foundation without pulling in a larger framework.

## Current module ownership

* `MABSCore` owns authored data and shared runtime types
* `MABSGameplay` owns granting, activation, delivery, effects, costs, cooldowns, replication, and projectile runtime
* `MABSDebug` owns runtime-safe formatting helpers and the HUD overlay
* `MABSEditor` remains the editor-only extension point

## Current ability flow

1. A server-owned actor is granted a `UMABSAbilityDefinition`.
2. Input or gameplay code calls `TryActivateAbilityByTag`.
3. Authority validates grant, cooldown, and cost rules.
4. Authority executes the authored delivery mode.
5. `Direct`, `HitTrace`, and `Melee` apply the instant effect immediately on success.
6. `Projectile` commits on authoritative spawn, then applies the effect later on authoritative impact.
7. Authority emits debug events for request, delivery, effect, cost, cooldown, and commit steps.

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
