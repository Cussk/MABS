# MABS Camera Shake

## What it is

Phase 6 lets startup, delivery, and impact cues optionally request a `UCameraShakeBase` class.

## Why it exists

Camera response is often the last small piece needed to make an attack, heal, or impact feel finished.

## Authored fields

Each shared presentation cue can author:

* `CameraShake.CameraShakeClass`
* `CameraShake.InnerRadius`
* `CameraShake.OuterRadius`
* `CameraShake.Falloff`

## Runtime behavior

MABS plays world camera shakes at the same world-space point used for the cue.

That means:

* startup shake happens where the startup cue plays
* delivery shake happens where the delivery cue plays
* impact shake happens at the resolved impact point

Dedicated servers do not play cosmetic camera shakes.

## How to use it

1. Assign a camera shake class on the relevant cue.
2. Set radii and falloff for the intended audience size.
3. Test in standalone and multiplayer to make sure the shake feels appropriate for local players.

## Example

Example heavy hammer swing:

* `DeliveryPresentation.Cue.CameraShake.CameraShakeClass = CS_HammerSwing`
* `DeliveryPresentation.Cue.CameraShake.OuterRadius = 900`
* `ImpactPresentation.Cue.CameraShake.CameraShakeClass = CS_HammerImpact`
* `ImpactPresentation.Cue.CameraShake.OuterRadius = 1400`
