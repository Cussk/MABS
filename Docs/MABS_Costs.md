# MABS Costs

## What it is

This document explains the current cost model in MABS after Phase 4 delivery support.

## Why it exists

Delivery mode changes how an ability reaches a target, not who owns resource spending. Cost validation and spending still stay authoritative.

## Cost design

Costs are still authored in MABS and stored in the host project:

* `UMABSAbilityDefinition` defines `ResourceCost`
* the owning actor optionally implements `IMABSCostReceiver`
* `UMABSAbilityComponent` validates and spends cost on authority

## Commit timing

`Direct`, `HitTrace`, and `Melee`:

* cost is spent after successful effect application

`Projectile`:

* cost is spent after successful projectile spawn

## How to use it

1. Set `ResourceCost`.
2. Implement `IMABSCostReceiver` on the owner.
3. Grant the ability on authority.
4. Activate it with enough resource.
5. Verify `CostValidated` and `CostSpent`.
