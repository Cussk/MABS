# MABS API

## What it is

This document lists the current Phase 2.5 public surface for MABS and shows which module owns authored data, runtime state, target debugging, execution, and overlay helpers.

## Why it exists

Phase 2.5 keeps the existing multiplayer-safe activation flow, then adds configurable actor targeting, richer target trace visibility, and a runtime-safe debug overlay. Users need a clear map of the new authored fields, runtime state, and helper types.

## Module ownership

### `MABSCore`

Owns foundational runtime data:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSTargetTraceMode`
* `EMABSAbilityActivationPolicy`
* `EMABSInstantEffectType`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSAbilityDebugEvent`
* `FMABSTargetTraceDebugInfo`
* `UMABSAbilityDefinition`
* `LogMABSAbilitySystem`

### `MABSGameplay`

Owns gameplay execution:

* `UMABSAbilityComponent`
* `IMABSInstantEffectReceiver`

### `MABSDebug`

Owns runtime-safe presentation helpers:

* `UMABSDebugBlueprintLibrary`
* `AMABSDebugHUD`

## Enums

### `EMABSAbilityActivationResult`

Describes the result of an activation request or the last known result stored on a granted ability spec.

Current values:

* `None`
* `Success`
* `RequestSentToServer`
* `InvalidAbility`
* `NotGranted`
* `AlreadyActive`
* `Blocked`
* `AuthorityRejected`
* `TargetResolutionFailed`
* `EffectApplicationFailed`

### `EMABSAbilityRuntimeState`

Represents the live runtime state of a granted ability entry.

Current values:

* `None`
* `Idle`
* `Active`
* `Blocked`

### `EMABSTargetType`

Represents the authored target intent for an ability definition.

Current supported values:

* `Self`
* `Actor`

`Location` remains present in the enum but is still out of scope.

### `EMABSTargetTraceMode`

Represents the authored trace shape for `Actor` targeting.

Current values:

* `Line`
* `Sphere`

### `EMABSAbilityActivationPolicy`

Represents the authored activation intent for an ability definition.

Current values:

* `OnInputTriggered`
* `Passive`

### `EMABSInstantEffectType`

Represents the authored instant effect intent for an ability definition.

Current values:

* `None`
* `Damage`
* `Heal`

## Structs

### `FMABSAbilityHandle`

A lightweight runtime identifier assigned when an ability is granted.

### `FMABSAbilitySpec`

The runtime representation of one granted ability.

Current fields:

* `Handle`
* `AbilityDefinition`
* `AbilityTag`
* `RuntimeState`
* `LastActivationResult`

### `FMABSAbilityDebugEvent`

A structured debug payload emitted by the ability component. It records the event name, owner, tag, handle, runtime state, activation result, timestamp, and plain-language message.

### `FMABSTargetTraceDebugInfo`

A structured snapshot of the latest actor-target trace for the owning actor.

Current fields:

* `bHasTraceData`
* `bHit`
* `bAcceptedTarget`
* `AbilityTag`
* `AbilityHandle`
* `TraceMode`
* `TraceStart`
* `TraceEnd`
* `HitLocation`
* `TraceRadius`
* `HitDistance`
* `WorldTimeSeconds`
* `ViewPointDescription`
* `HitActorName`
* `HitComponentName`
* `ResultMessage`

## Classes

### `UMABSAbilityDefinition`

A `UDataAsset` that stores authored ability data.

Current authored fields:

* `AbilityTag`
* `DisplayName`
* `ActivationPolicy`
* `TargetType`
* `InstantEffectType`
* `EffectMagnitude`
* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`
* `bDrawTargetTraceDebug`
* `TargetTraceDebugDuration`
* `CooldownSeconds`
* `ResourceCost`

### `UMABSAbilityComponent`

A `UActorComponent` that owns granted ability specs, routes activation requests, resolves targets, applies instant effects, emits debug events, and stores the latest target trace snapshot.

Current public functions:

* `GrantAbility(UMABSAbilityDefinition* AbilityDefinition)`
* `SetAbilityBlockedByTag(FGameplayTag AbilityTag, bool bBlocked)`
* `TryActivateAbilityByTag(FGameplayTag AbilityTag)`
* `GetGrantedAbilities()`
* `FindGrantedAbilitySpecByTag(FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec)`
* `GetRecentDebugEvents()`
* `GetLatestTargetTraceDebugInfo()`

Current replicated state:

* `GrantedAbilities`

Current public events:

* `OnAbilityDebugEvent`

Important target-related event names:

* `TargetTraceStarted`
* `TargetTraceHit`
* `TargetTraceRejected`
* `TargetResolved`
* `TargetResolutionFailed`

### `IMABSInstantEffectReceiver`

A minimal optional extension hook for receiving MABS heal effects.

Current public function:

* `ApplyMABSHeal(float HealAmount, AActor* SourceActor, UMABSAbilityDefinition* SourceAbility)`

### `UMABSDebugBlueprintLibrary`

A runtime-safe helper library for debug consumers.

Current public functions:

* `FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)`
* `FormatTargetTraceDebugInfo(const FMABSTargetTraceDebugInfo& DebugInfo)`
* `GetAbilityDebugEventColor(const FMABSAbilityDebugEvent& DebugEvent)`
* `GetTargetTraceDebugColor(const FMABSTargetTraceDebugInfo& DebugInfo)`

### `AMABSDebugHUD`

A lightweight runtime HUD overlay that reads the local owner’s `UMABSAbilityComponent` and displays:

* the latest target trace snapshot
* a recent debug event list
* simple color-coded success and failure feedback

Current public functions:

* `SetOverlayEnabled(bool bEnabled)`
* `ToggleOverlayEnabled()`
* `IsOverlayEnabled()`

## How to use it

1. Enable the `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset with a valid `AbilityTag`.
3. Set `TargetType`, `InstantEffectType`, `EffectMagnitude`, and the Phase 2.5 actor-target trace options when using `Actor`.
4. Add `UMABSAbilityComponent` to the owning actor or character.
5. Grant the definition on the authoritative owner with `GrantAbility`.
6. Call `TryActivateAbilityByTag` from input or gameplay code.
7. Inspect `GetRecentDebugEvents()` and `GetLatestTargetTraceDebugInfo()`.
8. Use `AMABSDebugHUD` or the debug library helpers to present the data at runtime.

## Example

Example C++ flow:

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(SelfHealDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(SelfHealTag);

const FMABSTargetTraceDebugInfo TraceInfo =
	AbilityComponent->GetLatestTargetTraceDebugInfo();
```

In standalone or on the listen server host, a valid granted self-heal ability resolves the owner, applies the heal, and returns `Success`. For an actor-targeted damage ability, the authority path also emits `TargetTraceStarted`, records the latest trace snapshot, validates the candidate target, and then applies the effect. On a remote client, the local call still returns `RequestSentToServer`, then the owning client receives the authoritative target, effect, commit, and trace-debug data.
