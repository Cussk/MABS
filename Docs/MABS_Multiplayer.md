# MABS Multiplayer

## What it is

This document explains how Phase 4 handles multiplayer authority for activation, delivery, projectile runtime, costs, cooldowns, and debug visibility.

## Why it exists

Delivery is easy to get wrong if clients can decide hits, sweeps, or projectile impact. MABS keeps those decisions on the server while still mirroring readable results back to the owning client.

## Server authority

The server owns:

* granting
* cooldown validation
* cost validation and spending
* direct target resolution
* hit-trace and melee hit resolution
* projectile spawn
* projectile impact
* damage and heal application

## Client behavior

When a remote client calls `TryActivateAbilityByTag`:

1. the local component records `RequestStarted`
2. the local component sends `ServerTryActivateAbilityByTag`
3. the local return value is `RequestSentToServer`
4. authority validates and executes the authored delivery mode
5. the owning client receives the authoritative debug events

## Projectile behavior in multiplayer

Projectile delivery uses a simple spawn-success model:

* authority spawns the projectile
* successful spawn commits the ability
* cost is spent and cooldown starts on successful spawn
* later impact applies the authored instant effect on authority

Clients do not authoritatively decide projectile results.

## Runtime overlay

`AMABSDebugHUD` is still client-side runtime UI. It does not make gameplay decisions.

It displays:

* the latest authoritative trace or sweep snapshot
* replicated ability runtime summaries
* recent mirrored debug events

## Verification checklist

Singleplayer:

* each delivery mode succeeds or fails correctly

Listen server:

* host can use every delivery mode
* remote clients receive authoritative delivery results
* projectile impact remains authoritative

Dedicated server:

* delivery logic stays server-authoritative
* projectile spawn and impact stay server-authoritative
* no editor-only dependency leaks into runtime code
