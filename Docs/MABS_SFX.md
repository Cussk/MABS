# MABS SFX

## What it is

Phase 6 sound authoring uses `USoundBase` references on the presentation data structs.

## Why it exists

Sound is one of the lowest-cost ways to make an ability feel responsive, so it belongs in the baseline presentation workflow.

## Supported SFX paths

Startup and delivery:

* `StartupPresentation.Cue.SFX`
* `DeliveryPresentation.Cue.SFX`

Hit trace:

* `DeliveryPresentation.HitTraceTracer.TracerSFX`

Projectile travel:

* `DeliveryPresentation.ProjectileTravel.TravelSFX`

Impact:

* `ImpactPresentation.Cue.SFX`

## How to use it

1. Assign a sound asset to the relevant cue field.
2. Use startup audio for casts and windups.
3. Use delivery audio for muzzle, spawn, or release moments.
4. Use impact audio for direct, melee, hit-trace, or projectile contact.

## Example

Example sword slash:

* `StartupPresentation.Cue.SFX = Sword_Windup`
* `DeliveryPresentation.Cue.SFX = Sword_Swing`
* `ImpactPresentation.Cue.SFX = Sword_Hit`

## Multiplayer note

Presentation sounds are cosmetic.

Gameplay still resolves on authority first, then the sound is routed through the presentation path so relevant players hear the same event timing.
