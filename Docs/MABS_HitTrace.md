# MABS HitTrace

## What it is

`HitTrace` is the instant ranged delivery mode in MABS.

## Why it exists

Many abilities need a practical rifle, beam, or instant-shot path without spawning a projectile.

## Authored fields

Use:

* `TargetType = Actor`
* `DeliveryMode = HitTrace`
* `HitTraceDistance`
* `HitTraceRadius`
* `DeliveryPresentation.Cue`
* `DeliveryPresentation.HitTraceTracer`
* `ImpactPresentation.Cue`

`HitTraceRadius = 0` behaves like a line trace. A positive radius behaves like a sphere trace.

## Runtime behavior

On authority, MABS:

1. enters delivery at the authored delivery time
2. triggers delivery presentation at the origin
3. traces from the authoritative aim direction
4. spawns tracer presentation from the trace start to the hit or final trace end if authored
5. validates the first resolved actor
6. applies the authored instant effect
7. triggers impact presentation on successful effect application
8. spends cost and starts cooldown on successful commit

## Debug events

Use:

* `DeliveryPresentationTriggered`
* `TracerSpawned`
* `HitTraceHit`
* `HitTraceRejected`
* `ImpactPresentationTriggered`

## Example

Example rifle shot:

* `HitTraceOriginSocketName = Muzzle`
* `DeliveryPresentation.Cue.VFX = P_RifleMuzzle`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = P_RifleTracer`
* `ImpactPresentation.Cue.VFX = P_BulletImpact`
