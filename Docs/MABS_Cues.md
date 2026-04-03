# MABS Cues

## What it is

MABS uses a lightweight presentation cue model for cosmetics.

A cue in MABS means:

* gameplay has already decided that a presentation moment happened
* a small runtime payload is built from authored data
* a router decides how that cosmetic event should be realized

This is not a full GAS gameplay-cue system.

## Why it exists

The cue layer keeps two concerns separate:

* gameplay authority decides truth
* presentation routing decides who should see the cosmetic result and how it should be played locally

That keeps `UMABSAbilityComponent` readable while making the presentation path easier to scale.

## Cue types

Phase 6.5 uses three runtime cue payloads:

* `FMABSPresentationCueEvent` for startup, delivery, and impact
* `FMABSTracerCueEvent` for hit-trace tracers
* `FMABSProjectileTravelCueEvent` for projectile travel

## Visibility policy

Cue routing uses a very small policy model:

* `RelevantClients`
* `OwnerOnly`
* `LocalOnly`

These policies affect cosmetics only. They do not change gameplay authority, hit results, or effect application.

## How to use it

1. Author the cue in `UMABSAbilityDefinition`.
2. Leave `VisibilityPolicy = RelevantClients` unless the cue is intentionally narrower.
3. Activate the ability normally.
4. Check debug events if the cue is routed, skipped, or falls back.

## Example

Example self-heal:

* `StartupPresentation.Cue.SFX = Heal_Cast`
* `StartupPresentation.Cue.VisibilityPolicy = OwnerOnly`
* `ImpactPresentation.Cue.VFX = NS_HealBurst`

Result:

* authority still decides when startup and impact happen
* startup audio routes only to the owning player
* impact VFX can still use the default relevant-clients path if authored that way
