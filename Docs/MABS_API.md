# MABS API

## What it is

This document lists the current Phase 2 public surface for MABS and shows which module owns authored data, runtime state, execution, and debug helpers.

## Why it exists

Phase 2 keeps the existing multiplayer-safe activation flow and extends it with simple targeting and instant effects. Users need a clear view of what changed and where the new types live.

## Module ownership

### `MABSCore`

Owns foundational runtime data:

* `EMABSAbilityActivationResult`
* `EMABSAbilityRuntimeState`
* `EMABSTargetType`
* `EMABSAbilityActivationPolicy`
* `EMABSInstantEffectType`
* `FMABSAbilityHandle`
* `FMABSAbilitySpec`
* `FMABSAbilityDebugEvent`
* `UMABSAbilityDefinition`
* `LogMABSAbilitySystem`

### `MABSGameplay`

Owns gameplay execution:

* `UMABSAbilityComponent`
* `IMABSInstantEffectReceiver`

### `MABSDebug`

Owns runtime-safe debug helpers:

* `UMABSDebugBlueprintLibrary`

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

Phase 2 supports:

* `Self`
* `Actor`

`Location` remains present in the enum but is not implemented in this phase.

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
* `CooldownSeconds`
* `ResourceCost`

### `UMABSAbilityComponent`

A `UActorComponent` that owns granted ability specs, routes activation requests, resolves simple targets, applies instant effects, and emits debug events.

Current public functions:

* `GrantAbility(UMABSAbilityDefinition* AbilityDefinition)`
* `SetAbilityBlockedByTag(FGameplayTag AbilityTag, bool bBlocked)`
* `TryActivateAbilityByTag(FGameplayTag AbilityTag)`
* `GetGrantedAbilities()`
* `FindGrantedAbilitySpecByTag(FGameplayTag AbilityTag, FMABSAbilitySpec& OutAbilitySpec)`
* `GetRecentDebugEvents()`

Current replicated state:

* `GrantedAbilities`

Current public events:

* `OnAbilityDebugEvent`

### `IMABSInstantEffectReceiver`

A minimal optional extension hook for receiving MABS heal effects.

Current public function:

* `ApplyMABSHeal(float HealAmount, AActor* SourceActor, UMABSAbilityDefinition* SourceAbility)`

### `UMABSDebugBlueprintLibrary`

A runtime-safe helper library for debug consumers.

Current public functions:

* `FormatAbilityDebugEvent(const FMABSAbilityDebugEvent& DebugEvent)`

## How to use it

1. Enable the `MABS` plugin.
2. Create a `UMABSAbilityDefinition` asset with a valid `AbilityTag`.
3. Set `TargetType`, `InstantEffectType`, `EffectMagnitude`, and optional `TargetTraceDistance`.
4. Add `UMABSAbilityComponent` to the owning actor or character.
5. Grant the definition on the authoritative owner with `GrantAbility`.
6. Call `TryActivateAbilityByTag` from input or gameplay code.
7. Inspect debug events or formatted debug strings for the outcome.

## Example

Example C++ flow:

```cpp
if (AbilityComponent != nullptr && HasAuthority())
{
	AbilityComponent->GrantAbility(SelfHealDefinition);
}

const EMABSAbilityActivationResult Result =
	AbilityComponent->TryActivateAbilityByTag(SelfHealTag);
```

In standalone or on the listen server host, a valid granted self-heal ability resolves the owner, applies the heal, and returns `Success`. On a remote client, the local call returns `RequestSentToServer`, then the owning client receives the authoritative target, effect, and commit debug events from the server-authoritative path.
