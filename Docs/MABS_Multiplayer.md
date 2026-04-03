# MABS Multiplayer

## What it is

This document explains how Phase 8 handles multiplayer authority for gameplay and for the new runtime debug harness.

## Why it exists

Phase 8 adds a stronger runtime inspection layer, but it does not change the core rule:

* gameplay stays authoritative on the server
* the owning client gets enough replicated state and routed debug visibility to understand authoritative results

## Server authority

The server still owns:

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

Clients still do not authoritatively decide:

* whether a grouped grant is valid
* whether a combo request was valid
* which AoE actors were accepted
* when periodic ticks happen
* when periodic effects expire

## Harness visibility in multiplayer

Phase 8 focuses on local-owner inspection first.

That means the owning client can inspect:

* granted abilities through replicated `GrantedAbilities`
* cooldown groups through replicated `CooldownGroupStates`
* recent authoritative debug events routed from the server
* latest target trace or delivery snapshot routed from the server
* owner-only periodic effect summaries when debug replication is enabled

The harness does not try to become a full remote debugger for every actor in the match.

## Ability sets in multiplayer

Ability sets still do not create a separate replication or authority model.

Grouped granting still means:

* authority iterates the authored set
* each valid entry uses the normal `GrantAbility(...)` path
* `GrantedAbilities` remains the authoritative replicated runtime state

## Cue routing in multiplayer

Authority still decides when startup, delivery, tracer, projectile travel, and impact happen.

The visibility policy model still applies:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

The Phase 8 harness only observes those decisions. It does not change them.

## Dedicated server behavior

Dedicated servers:

* still make authoritative gameplay decisions
* still grant abilities and ability sets
* still route cosmetic results outward when needed
* do not realize local VFX, SFX, or camera shake
* still run combo timers and periodic timers authoritatively
* can still feed the owning client enough harness data to inspect their own MABS state

## What disabling debug changes

If the harness is hidden or debug replication is disabled:

* gameplay still works normally
* activation, delivery, combo, AoE, periodic, and cue logic still run
* only the extra owner-facing debug visibility is reduced

## Verification checklist

Singleplayer:

* the harness reads local state correctly

Listen server:

* the host can inspect local harness data
* the remote owning client receives authoritative harness data for their own component

Dedicated server:

* the owning client sees recent debug events and target trace results
* the owning client can inspect active periodic summaries when debug replication is enabled
* gameplay still works correctly if the harness is disabled
