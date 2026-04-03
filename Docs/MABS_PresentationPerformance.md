# MABS Presentation Performance

## What it is

This document explains the lightweight performance foundations added in Phase 6.5 for MABS presentation.

## Why it exists

The presentation path works in Phase 6, but repeated tiny effects can become expensive if every cue always spawns a fresh transient object.

Phase 6.5 improves that path without adding a heavy manager.

## What changed

Phase 6.5 now centralizes local realization for the spam-prone paths:

* world-space startup, delivery, and impact Niagara cues
* hit-trace tracer Niagara cues

`UMABSAbilityComponent` keeps small reusable Niagara component pools keyed by system asset for those paths.

## What that means in practice

When the same Niagara system is requested again:

* MABS first looks for an inactive local component that already uses that system
* if one exists, MABS repositions it and reactivates it
* if not, MABS spawns a new component and keeps it available for later reuse

This is intentionally small:

* no global manager
* no generalized significance system
* no heavy cross-map pool lifetime rules

## Dedicated server behavior

Dedicated servers still route cosmetic outcomes when needed, but they do not realize local presentation components.

That keeps server work focused on gameplay authority.

## How to use it

1. Author one-shot Niagara systems for startup, delivery, impact, or tracer cues.
2. Use `RelevantClients` only where players actually need to see the cue.
3. Use `OwnerOnly` or `LocalOnly` for intentionally narrow cosmetics.
4. Stress-test repeated rifle shots, muzzle flashes, or impact bursts and inspect routing events.

## Example

Example repeated rifle fire:

* `DeliveryPresentation.Cue.VFX = NS_RifleMuzzle`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = NS_RifleTracer`
* `ImpactPresentation.Cue.VFX = NS_BulletImpact`

Result:

* gameplay timing stays the same
* the cue router keeps the code path centralized
* repeated muzzle, tracer, and impact Niagara cues can reuse inactive local components instead of always creating fresh ones
