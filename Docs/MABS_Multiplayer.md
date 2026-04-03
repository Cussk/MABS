# MABS Multiplayer

## What it is

This document explains how Phase 7 handles multiplayer authority for activation, delivery, combo acceptance, AoE target resolution, periodic effects, costs, cooldowns, projectile runtime, cue routing, and cosmetic realization.

## Why it exists

Phase 7 adds more combat breadth, but the same rule still holds: gameplay results must stay authoritative on the server, while clients get enough replicated state and debug visibility to understand what happened.

## Server authority

The server owns:

* granting
* cooldown validation
* cost validation and spending
* combo window timing and combo acceptance
* direct target resolution
* hit-trace and melee hit resolution
* AoE overlap gathering and target validation
* projectile spawn
* projectile impact
* instant damage and heal application
* periodic effect application
* periodic ticking and expiration
* presentation timing decisions

## Client behavior

Clients may request:

* ability activation
* combo follow-up input

Clients do not authoritatively decide:

* whether a combo request was valid
* which AoE actors were accepted
* when periodic ticks happen
* when periodic effects expire

## Cue routing in multiplayer

Authority still decides when startup, delivery, tracer, projectile travel, and impact happen.

The visibility policy model from Phase 6.5 still applies:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

Combo, AoE, and periodic effects do not change that cosmetic routing model.

## Dedicated server behavior

Dedicated servers:

* still make authoritative gameplay decisions
* still route cosmetic results outward when needed
* do not realize local VFX, SFX, or camera shake
* still run combo timers and periodic timers authoritatively

## Verification checklist

Singleplayer:

* combo follow-ups queue and auto-trigger correctly
* AoE hits multiple valid actors
* periodic ticks and expiration are readable

Listen server:

* host combo behavior works correctly
* remote combo requests are validated on authority
* AoE remains authoritative
* periodic ticks remain authoritative

Dedicated server:

* combo acceptance remains authoritative
* AoE resolution remains authoritative
* periodic ticking remains authoritative
* server builds do not depend on cosmetic realization
