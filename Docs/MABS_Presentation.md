# MABS Presentation

## What it is

Phase 6 adds the first built-in cosmetic layer to MABS.

Abilities can now author:

* startup presentation
* delivery presentation
* hit-trace tracer presentation
* projectile travel presentation
* impact presentation

## Why it exists

Gameplay correctness is not enough for a practical combat framework.

Phase 6 keeps the server-authoritative gameplay model from earlier phases, but adds the minimum presentation coverage most teams expect from a usable combat system.

## Authored data

`UMABSAbilityDefinition` now exposes three grouped presentation fields:

* `StartupPresentation`
* `DeliveryPresentation`
* `ImpactPresentation`

Those groups are built from:

* `FMABSPresentationCueData` for shared VFX, SFX, camera shake, optional socket override, and offsets
* `FMABSHitTraceTracerPresentationData` for hit-trace travel/tracer presentation
* `FMABSProjectileTravelPresentationData` for projectile-attached travel presentation

## When presentation happens

Startup:

* triggers when the ability enters `Startup`

Delivery:

* triggers at the authored delivery moment
* uses the same timing already used for gameplay delivery

Impact:

* triggers when the gameplay hit or effect application succeeds

Projectile travel:

* starts from the spawned replicated projectile actor

## Socket rules

Startup and delivery presentation use this origin order:

1. the presentation cue socket override
2. the delivery-mode socket on the ability definition
3. `DeliveryOriginSocketName`
4. a safe fallback transform

If a presentation socket is missing, MABS falls back safely and emits `PresentationSocketFallbackUsed`.

Impact presentation uses the resolved hit location, or the target actor location if no explicit hit point exists.

## Delivery-mode examples

`Direct`

* startup cast cue
* delivery cue at the owner
* impact cue on the owner or resolved target

`HitTrace`

* startup cue
* muzzle or spawn cue at delivery
* tracer from origin to hit or end point
* impact cue on successful effect application

`Melee`

* startup cue
* swing-origin or hand cue at delivery
* impact cue on successful hit

`Projectile`

* startup cue
* spawn or muzzle cue at delivery
* projectile travel VFX and SFX on the replicated projectile
* impact cue on projectile hit

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Fill `StartupPresentation.Cue` if the ability should show a cast cue.
3. Fill `DeliveryPresentation.Cue` if the ability should show a muzzle, hand, or spawn cue.
4. Fill `DeliveryPresentation.HitTraceTracer` for `HitTrace` travel visuals.
5. Fill `DeliveryPresentation.ProjectileTravel` for `Projectile` travel visuals.
6. Fill `ImpactPresentation.Cue` for direct, hit trace, melee, or projectile impacts.
7. Activate the ability and verify the timing through debug events.

## Example

Example rifle shot:

* `StartupPresentation.Cue.SFX = Rifle_Cast`
* `DeliveryPresentation.Cue.VFX = P_RifleMuzzle`
* `DeliveryPresentation.HitTraceTracer.TracerVFX = P_RifleTracer`
* `ImpactPresentation.Cue.VFX = P_BulletImpact`
* `ImpactPresentation.Cue.SFX = Bullet_Impact`

## Not included

Phase 6 does not add:

* montage-notify-driven presentation as the primary path
* Niagara-specific editor tooling
* melee trail authoring
* advanced camera systems
* advanced audio routing
