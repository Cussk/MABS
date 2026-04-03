# MABS Multiplayer

## What it is

This document explains how Phase 6.5 handles multiplayer authority for activation, delivery, projectile runtime, costs, cooldowns, cue routing, and cosmetic realization.

## Why it exists

Presentation is cosmetic, but cue routing still has multiplayer rules. If those rules are not explicit, it is easy to accidentally mix gameplay authority with cosmetic visibility.

## Server authority

The server still owns:

* granting
* cooldown validation
* cost validation and spending
* direct target resolution
* hit-trace and melee hit resolution
* projectile spawn
* projectile impact
* damage and heal application
* presentation timing decisions

## Cue routing in multiplayer

Authority still decides when startup, delivery, tracer, projectile travel, and impact happen.

Phase 6.5 then routes cosmetics through a small visibility policy:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

Practical behavior:

* `RelevantClients` uses the normal cosmetic multicast path
* `OwnerOnly` routes only to the owning client when one exists
* `OwnerOnly` falls back to `RelevantClients` if no owning client exists
* `LocalOnly` stays local and does not become a gameplay fact

## Dedicated server behavior

Dedicated servers:

* still make authoritative gameplay decisions
* still route cosmetic results outward when needed
* do not realize local VFX, SFX, or camera shake
* skip local-only cue realization completely

That keeps gameplay correct without wasting server time on local cosmetic work.

## Projectile travel behavior

Projectile travel cues still come from the replicated projectile actor.

Phase 6.5 changes that path so the projectile evaluates a lightweight travel cue payload and applies the same small visibility-policy model locally on each instance.

## Runtime overlay

`AMABSDebugHUD` is still local runtime UI only. It does not make gameplay decisions.

It now helps explain:

* whether a cue was routed, skipped, or realized
* whether a policy fallback was used
* the latest authoritative trace or sweep snapshot
* replicated ability runtime summaries

## Verification checklist

Singleplayer:

* startup, delivery, tracer, projectile travel, and impact cues all realize
* local-only cues still work

Listen server:

* host sees routed and realized cue timing correctly
* remote client requests still resolve on authority
* owner-only cues only appear for the owning player

Dedicated server:

* gameplay remains authoritative
* local-only cues are skipped cleanly
* server builds do not depend on cosmetic realization
