# MABS Effects

## What it is

This document explains the current instant effect model in MABS after Phase 4 delivery support.

## Why it exists

Activation is only useful if it leads to a gameplay outcome. Delivery now controls how the target is reached, but the effect layer stays intentionally small.

## Supported instant effects

Current support remains:

* `Damage`
* `Heal`

## How effects apply now

`Direct`, `HitTrace`, and `Melee`:

* apply the effect immediately on authority after successful delivery

`Projectile`:

* applies the effect later on authoritative projectile impact

## Damage

Damage still uses Unreal's generic damage path through `UGameplayStatics::ApplyDamage`.

## Heal

Heal still uses `IMABSInstantEffectReceiver`.

Targets that should accept heal must implement:

* `ApplyMABSHeal(...)`

## Failure behavior

Effect application still fails cleanly if:

* the target does not support the authored effect
* the target rejects the heal
* the damage application returns zero accepted damage
