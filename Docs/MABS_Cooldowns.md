# MABS Cooldowns

## What it is

This document explains the current cooldown model in MABS after Phase 4 delivery support.

## Why it exists

Delivery added more ways to commit an ability, but the cooldown rules stay simple and authoritative.

## Supported cooldown behavior

MABS supports:

* per-ability cooldown duration
* optional shared cooldown groups
* authoritative cooldown start after successful commit
* authoritative cooldown denial before delivery execution

## Commit timing

`Direct`, `HitTrace`, and `Melee`:

* cooldown starts after successful delivery and successful effect application

`Projectile`:

* cooldown starts after successful projectile spawn

## Authored data

Cooldown fields remain:

* `CooldownSeconds`
* `CooldownGroupTag`

## Runtime state

Cooldown runtime data remains:

* `FMABSAbilitySpec::CooldownEndTime`
* `UMABSAbilityComponent::CooldownGroupStates`

## How to use it

1. Set `CooldownSeconds`.
2. Optionally set `CooldownGroupTag`.
3. Grant the ability on authority.
4. Activate it successfully.
5. Verify `CooldownStarted` and later `CooldownRejected`.
