# MABS VFX

## What it is

Phase 6 VFX authoring uses `UNiagaraSystem` references on the presentation data structs.

## Why it exists

This gives MABS a simple, runtime-safe Niagara workflow without adding a large editor or content pipeline.

## Supported VFX paths

Startup and delivery:

* `StartupPresentation.Cue.VFX`
* `DeliveryPresentation.Cue.VFX`

Hit trace:

* `DeliveryPresentation.HitTraceTracer.TracerVFX`

Projectile travel:

* `DeliveryPresentation.ProjectileTravel.TravelVFX`

Impact:

* `ImpactPresentation.Cue.VFX`

## Tracer parameter names

When MABS spawns a hit-trace tracer effect, it sets these vector parameters on the Niagara component if the system exposes matching user parameters:

* `User.MABS_TraceStart`
* `User.MABS_TraceEnd`
* `User.MABS_ImpactPoint`

That lets authored tracer effects behave like beams or travel streaks without custom gameplay code.

## How to use it

1. Author a Niagara system for the needed phase.
2. Assign it to the relevant presentation field.
3. Optionally author a socket override and offsets on the cue.
4. For hit-trace tracers, add Niagara user parameters for `User.MABS_TraceStart`, `User.MABS_TraceEnd`, and optionally `User.MABS_ImpactPoint`.
5. Read those parameters in the Niagara system if the effect needs beam endpoints or impact location data.

## Example

Example fireball:

* `DeliveryPresentation.Cue.VFX = NS_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = NS_FireballTrail`
* `ImpactPresentation.Cue.VFX = NS_FireballImpact`

## Not included

Phase 6 does not add:

* a custom Niagara manager or pooling framework
* custom editor previews for presentation assets
