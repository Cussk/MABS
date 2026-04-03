# MABS VFX

## What it is

Phase 6 VFX authoring uses `UParticleSystem` references on the presentation data structs.

## Why it exists

This gives MABS a simple, runtime-safe v1 VFX workflow without adding a large editor or content pipeline.

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

When MABS spawns a hit-trace tracer effect, it sets these vector parameters if the particle system uses them:

* `MABS_TraceStart`
* `MABS_TraceEnd`
* `MABS_ImpactPoint`

That lets authored tracer effects behave like beams or travel streaks without custom gameplay code.

## How to use it

1. Author a particle system for the needed phase.
2. Assign it to the relevant presentation field.
3. Optionally author a socket override and offsets on the cue.
4. For hit-trace tracers, read the `MABS_TraceStart` and `MABS_TraceEnd` parameters in the particle effect if needed.

## Example

Example fireball:

* `DeliveryPresentation.Cue.VFX = P_FireballCast`
* `DeliveryPresentation.ProjectileTravel.TravelVFX = P_FireballTrail`
* `ImpactPresentation.Cue.VFX = P_FireballImpact`

## Not included

Phase 6 does not add:

* Niagara-specific helper APIs
* custom editor previews for presentation assets
