# PHASE_06.md - Presentation Suite

## What it is

Phase 6 adds the first built-in presentation layer to MABS.

The implemented Phase 6 feature set is:

* startup presentation
* delivery presentation
* hit-trace tracer presentation
* projectile travel presentation
* impact presentation
* sound support
* optional camera shake
* presentation debug events and fallback messages

## Why it exists

MABS already had the gameplay timing and delivery model from earlier phases.

Phase 6 makes that runtime feel closer to a shippable combat framework by attaching cosmetics to the same startup, delivery, and impact moments instead of building a separate cosmetic timeline.

## Implemented data model

`MABSCore` now exposes grouped authored presentation data on `UMABSAbilityDefinition`:

* `StartupPresentation`
* `DeliveryPresentation`
* `ImpactPresentation`

Supporting structs:

* `FMABSPresentationCueData`
* `FMABSPresentationCameraShakeData`
* `FMABSHitTraceTracerPresentationData`
* `FMABSProjectileTravelPresentationData`

## Implemented runtime behavior

`MABSGameplay` now:

* triggers startup presentation when startup begins
* triggers delivery presentation at the delivery moment
* spawns hit-trace tracers from trace start to hit or trace end
* hooks projectile travel VFX and SFX onto the replicated projectile actor
* triggers impact presentation after successful effect application or projectile impact
* uses multicast cosmetic routing while keeping gameplay authority on the server

## Implemented debug visibility

Phase 6 adds these presentation-focused events:

* `StartupPresentationTriggered`
* `DeliveryPresentationTriggered`
* `ImpactPresentationTriggered`
* `TracerSpawned`
* `TracerSpawnFailed`
* `ProjectileTravelPresentationTriggered`
* `PresentationSocketFallbackUsed`

## Authoring notes

Current v1 presentation support uses:

* `UNiagaraSystem` for VFX
* `USoundBase` for SFX
* `UCameraShakeBase` for optional shake

Hit-trace tracers set these vector parameters on the spawned Niagara component when available:

* `User.MABS_TraceStart`
* `User.MABS_TraceEnd`
* `User.MABS_ImpactPoint`

## Not included

Phase 6 still does not add:

* custom Niagara editor tooling
* montage-notify-only presentation paths
* melee trail systems
* advanced audio routing
* advanced camera systems

## Verification checklist

Singleplayer:

* self-heal shows startup and impact presentation
* rifle shot shows muzzle, tracer, and impact presentation
* sword swing shows startup, delivery, and impact presentation
* fireball shows spawn, travel, and impact presentation
* missing sockets fall back safely

Listen server:

* host sees the same presentation timing as authority
* remote client requests still resolve authoritatively
* relevant players receive startup, delivery, and impact cosmetics

Dedicated server:

* gameplay remains authoritative
* dedicated servers do not depend on cosmetic success
* runtime server builds stay free of editor-only presentation code
