# MABS Presentation

## What it is

Phase 6.5 keeps the Phase 6 authored presentation groups, but runtime cosmetics now flow through lightweight cue events instead of realizing every effect inline inside gameplay branches.

Abilities still author:

* `StartupPresentation`
* `DeliveryPresentation`
* `ImpactPresentation`
* `DeliveryPresentation.HitTraceTracer`
* `DeliveryPresentation.ProjectileTravel`

Each shared cue, tracer cue, and projectile-travel cue now also authors a small `VisibilityPolicy`.

## Why it exists

Phase 6 already proved the timing model worked, but large combat scenes needed a cleaner presentation path:

* gameplay should still decide when startup, delivery, tracer, travel, and impact happen
* cosmetic realization should not be hard-wired into every gameplay branch
* repeated small Niagara effects should have a clearer reuse path
* dedicated servers should skip local cosmetic work cleanly

Phase 6.5 solves that without turning MABS into a large gameplay-cue framework.

## Authored data

`UMABSAbilityDefinition` still exposes the same grouped fields:

* `StartupPresentation.Cue`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.HitTraceTracer`
* `DeliveryPresentation.ProjectileTravel`
* `ImpactPresentation.Cue`

Shared cue data still authors:

* `VFX`
* `SFX`
* `CameraShake`
* `SocketName`
* `LocationOffset`
* `RotationOffset`
* `VisibilityPolicy`

Tracer and projectile-travel data now also author `VisibilityPolicy`.

## Runtime cue model

Authority still decides when presentation moments happen.

At runtime, MABS now builds lightweight payloads:

* `FMABSPresentationCueEvent`
* `FMABSTracerCueEvent`
* `FMABSProjectileTravelCueEvent`

Those payloads carry only the data needed to realize the cue:

* cue phase
* visibility policy
* asset references
* transform or trace path
* minimal debug identity like ability tag and handle

`UMABSAbilityComponent` still clearly shows when startup, delivery, tracer, and impact happen. The difference is that it now routes those moments through small helpers before anything is spawned locally.

Projectile travel still activates from `AMABSProjectileBase`, but it now follows the same cue-style payload and visibility-policy model.

## Visibility policy

Phase 6.5 adds a small policy model for cosmetic routing:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

Practical behavior:

* `RelevantClients` is the default for shared gameplay presentation
* `OwnerOnly` routes to the owning client when one exists
* `OwnerOnly` falls back to `RelevantClients` if no owning client exists, and emits `PresentationCuePolicyFallbackUsed`
* `LocalOnly` realizes only on a locally controlled instance or standalone, and is skipped on dedicated server

This policy changes cosmetic routing only. It does not change gameplay authority.

## Reuse and pooling foundations

Phase 6.5 does not add a giant pool manager.

It does centralize the spam-prone Niagara paths:

* world-space startup, delivery, and impact cues
* hit-trace tracer cues

`UMABSAbilityComponent` now reuses Niagara components for those paths when the same system is available and inactive, which gives repeated muzzle flashes, impact bursts, and tracers a lightweight reuse path without adding a heavy subsystem.

## How to use it

1. Author presentation the same way you did in Phase 6.
2. Leave `VisibilityPolicy = RelevantClients` unless the cue is intentionally owner-only or local-only.
3. Use `OwnerOnly` for UI-adjacent or personal cues.
4. Use `LocalOnly` only when a cue should stay entirely local.
5. Test the cue in standalone, listen server, and dedicated server.
6. Read routing events in the debug log or overlay if the cue does not behave as expected.

## Example

Example rifle shot:

* `DeliveryPresentation.Cue.VFX = NS_RifleMuzzle`
* `DeliveryPresentation.Cue.VisibilityPolicy = RelevantClients`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = NS_RifleTracer`
* `DeliveryPresentation.HitTraceTracer.VisibilityPolicy = RelevantClients`
* `ImpactPresentation.Cue.VFX = NS_BulletImpact`

Runtime result:

* authority decides when delivery happens
* MABS builds a delivery cue event and a tracer cue event
* the router sends both through the relevant-clients path
* repeated tracer and impact Niagara systems can reuse inactive local components

## Not included

Phase 6.5 still does not add:

* a tag-heavy GAS-style cue database
* a giant global VFX or audio manager
* advanced significance or distance culling
* prediction-driven presentation
* a generalized pool framework for every cue type
