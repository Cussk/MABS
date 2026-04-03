# MABS API

## What it is

This document lists the current Phase 6.5 public surface for MABS and shows which module owns authored cue policy data, runtime cue payloads, projectile travel cues, and routing-aware debug helpers.

## Module ownership

### `MABSCore`

Owns shared authored data and runtime types:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSTargetTraceMode`
* `EMABSAbilityActivationPolicy`
* `EMABSDeliveryMode`
* `EMABSInstantEffectType`
* `EMABSPresentationCueVisibilityPolicy`
* `EMABSPresentationCuePhase`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSCooldownGroupState`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `FMABSPresentationCameraShakeData`
* `FMABSPresentationCueData`
* `FMABSHitTraceTracerPresentationData`
* `FMABSProjectileTravelPresentationData`
* `FMABSPresentationCueEvent`
* `FMABSTracerCueEvent`
* `FMABSProjectileTravelCueEvent`
* `FMABSPresentationStartupData`
* `FMABSPresentationDeliveryData`
* `FMABSPresentationImpactData`
* `UMABSAbilityDefinition`

### `MABSGameplay`

Owns gameplay and cue execution:

* `UMABSAbilityComponent`
* `AMABSProjectileBase`
* `IMABSInstantEffectReceiver`
* `IMABSCostReceiver`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Important authored fields

### `FMABSPresentationCueData`

Shared authored cue fields:

* `VFX`
* `SFX`
* `CameraShake`
* `SocketName`
* `LocationOffset`
* `RotationOffset`
* `VisibilityPolicy`

### `FMABSHitTraceTracerPresentationData`

Tracer authored fields:

* `TracerVFX`
* `TracerSFX`
* `VisibilityPolicy`

### `FMABSProjectileTravelPresentationData`

Projectile travel authored fields:

* `TravelVFX`
* `TravelSFX`
* `VisibilityPolicy`

## Important runtime cue structs

### `FMABSPresentationCueEvent`

Used for startup, delivery, and impact cue routing.

Important fields:

* `AbilityTag`
* `AbilityHandle`
* `RuntimeState`
* `Phase`
* `VisibilityPolicy`
* `VFX`
* `SFX`
* `CameraShakeClass`
* `Location`
* `Rotation`

### `FMABSTracerCueEvent`

Used for hit-trace tracer routing.

Important fields:

* `AbilityTag`
* `AbilityHandle`
* `RuntimeState`
* `VisibilityPolicy`
* `VFX`
* `SFX`
* `TraceStart`
* `TraceEnd`

### `FMABSProjectileTravelCueEvent`

Used by `AMABSProjectileBase` for local projectile-travel realization.

Important fields:

* `AbilityTag`
* `AbilityHandle`
* `RuntimeState`
* `VisibilityPolicy`
* `VFX`
* `SFX`

## Important debug events

Phase 6.5 adds or highlights:

* `PresentationCueRouted`
* `PresentationCueSkipped`
* `PresentationCueRealized`
* `PresentationCuePolicyFallbackUsed`
* `TracerCueRouted`
* `TracerCueSkipped`
* `TracerCueRealized`
* `ProjectileTravelPresentationTriggered`
* `PresentationSocketFallbackUsed`
