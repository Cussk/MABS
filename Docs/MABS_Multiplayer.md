# MABS Multiplayer

## What it is

This document explains how Phase 7.5 handles multiplayer authority for grouped granting, activation, delivery, combo acceptance, AoE target resolution, periodic effects, costs, cooldowns, projectile runtime, cue routing, and cosmetic realization.

## Why it exists

Phase 7.5 adds ability-set authoring and grouped granting, but the same rule still holds: gameplay results must stay authoritative on the server, while clients get enough replicated state and debug visibility to understand what happened.

## Server authority

The server owns:

* single ability granting
* ability-set granting
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

* whether a grouped grant is valid
* whether a combo request was valid
* which AoE actors were accepted
* when periodic ticks happen
* when periodic effects expire

## Ability sets in multiplayer

Ability sets do not create a separate replication or authority model.

Grouped granting still means:

* authority iterates the authored set
* each valid entry uses the normal `GrantAbility(...)` path
* `GrantedAbilities` remains the authoritative replicated runtime state

## Cue routing in multiplayer

Authority still decides when startup, delivery, tracer, projectile travel, and impact happen.

The visibility policy model from Phase 6.5 still applies:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

Ability sets do not change that cosmetic routing model.

## Dedicated server behavior

Dedicated servers:

* still make authoritative gameplay decisions
* still grant abilities and ability sets
* still route cosmetic results outward when needed
* do not realize local VFX, SFX, or camera shake
* still run combo timers and periodic timers authoritatively

## Verification checklist

Singleplayer:

* starting ability sets grant all valid entries
* null entries are skipped safely
* duplicate entries follow the normal grant policy

Listen server:

* host grouped grant behavior works correctly
* remote client-owned actors receive granted abilities through authority

Dedicated server:

* grouped granting remains authoritative
* normal granted ability replication still works
* server builds do not depend on cosmetic realization
