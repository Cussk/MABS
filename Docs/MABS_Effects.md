# MABS Effects

## What it is

This document explains the current instant effect model for MABS.

## Why it exists

Activation is only useful if it leads to a gameplay outcome. Phase 2 introduced the deliberately small, server-authoritative effect slice, and Phase 3 keeps that slice while adding cooldown and cost rules around it.

## Supported instant effects

Current support:

* `Damage`
* `Heal`

These are one-shot, immediate effects applied on authority after target resolution succeeds. In Phase 3, successful ability usage may also spend cost and start cooldown after the effect succeeds.

## Damage

Damage uses Unreal's generic damage path.

Why this exists:

* it keeps the plugin general-purpose
* it avoids forcing a custom health system into MABS
* it works with actors that already use `TakeDamage`

How to use it:

1. Set `InstantEffectType` to `Damage`.
2. Set `EffectMagnitude` to a positive value.
3. Use `Actor` targeting or `Self` if desired.

Example:

* a fireball test ability targets a traced actor and applies `20` damage

## Heal

Heal uses a minimal optional extension hook: `IMABSInstantEffectReceiver`.

Why this exists:

* MABS does not own a full health framework yet
* users can opt in without inheriting from a required base class

How to use it:

1. Set `InstantEffectType` to `Heal`.
2. Set `EffectMagnitude` to a positive value.
3. Ensure the resolved target implements `IMABSInstantEffectReceiver`.
4. Implement `ApplyMABSHeal` on the receiving actor.

Example:

* the current host-project `AMABSCharacter` implements the interface and accepts self-heal for the current harness

## Failure behavior

If effect application cannot complete:

* the request fails cleanly
* `LastActivationResult` becomes `EffectApplicationFailed`
* an `EffectApplicationFailed` debug event is emitted

Common reasons include:

* heal target does not implement `IMABSInstantEffectReceiver`
* heal target rejects the heal
* damage target returns zero accepted damage

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Set `InstantEffectType` to `Damage` or `Heal`.
3. Set `EffectMagnitude` to a positive value.
4. Grant the ability on authority.
5. Activate it and inspect `EffectApplied` or `EffectApplicationFailed`.
