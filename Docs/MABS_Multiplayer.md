# MABS Multiplayer

## What it is

This document explains how Phase 6 handles multiplayer authority for activation, delivery, projectile runtime, costs, cooldowns, presentation, and debug visibility.

## Why it exists

Delivery and presentation are easy to get wrong if clients can decide hits, sweeps, projectile impact, or cosmetic timing. MABS keeps gameplay decisions on the server while routing relevant cosmetic results out to clients in a controlled way.

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
* presentation timing decisions

## Client behavior

When a remote client calls `TryActivateAbilityByTag`:

1. the local component records `RequestStarted`
2. the local component sends `ServerTryActivateAbilityByTag`
3. the local return value is `RequestSentToServer`
4. authority validates and executes the authored delivery mode
5. the owning client receives the authoritative debug events

## Presentation behavior in multiplayer

Presentation is cosmetic, but Phase 6 still routes it from authority:

* startup presentation starts when authority enters startup
* delivery presentation starts when authority reaches the authored delivery time
* hit-trace tracer presentation uses the authoritative trace path
* projectile travel presentation runs from the replicated projectile actor
* impact presentation starts after the authoritative gameplay result succeeds

Clients do not authoritatively decide gameplay results.

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
