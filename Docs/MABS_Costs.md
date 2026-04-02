# MABS Costs

## What it is

This document explains the Phase 3 cost model for MABS.

## Why it exists

Many abilities need a simple payment rule before they can activate. Phase 3 adds the smallest useful authoritative cost model without forcing MABS to own a full stat or attribute framework.

## Cost design

Costs are authored in MABS, but the actual resource storage stays in the host project.

That means:

* `UMABSAbilityDefinition` defines `ResourceCost`
* the owning actor optionally implements `IMABSCostReceiver`
* `UMABSAbilityComponent` validates and spends cost on authority

This keeps MABS flexible across:

* mana
* stamina
* energy
* ammo-like resource pools
* custom project-specific resource models

## Authored cost data

Relevant authored field on `UMABSAbilityDefinition`:

* `ResourceCost`

If `ResourceCost <= 0`, MABS skips cost validation and cost spending.

## Resource interface

Phase 3 adds `IMABSCostReceiver`.

Current functions:

* `CanAffordMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility)`
* `SpendMABSCost(float Cost, UMABSAbilityDefinition* SourceAbility)`

This interface is intentionally small.

## Cost behavior

### On activation attempt

Before target/effect execution, MABS:

* checks whether `ResourceCost > 0`
* validates that the owner implements `IMABSCostReceiver`
* calls `CanAffordMABSCost(...)`

If the owner cannot afford cost:

* activation fails cleanly
* `LastActivationResult` becomes `InsufficientResources`
* `CostRejected` explains why

### On successful commit

After the effect succeeds, MABS spends cost on authority through `SpendMABSCost(...)`.

When spending succeeds:

* `CostSpent` is emitted

## Host-project example

The current host project `AMABSCharacter` implements `IMABSCostReceiver` with a small example resource pool:

* `MaxExampleResource`
* `CurrentExampleResource`

This is host-project sample code, not plugin-mandated gameplay design.

## How to use it

1. Open a `UMABSAbilityDefinition`.
2. Set `ResourceCost` to a positive number.
3. Implement `IMABSCostReceiver` on the owning actor.
4. Grant the ability on authority.
5. Activate it with enough resource.
6. Reduce the owner resource below the authored cost.
7. Activate it again and inspect `CostRejected`.

## Example

Example self-heal cost setup:

1. Create `DA_Test_SelfHeal`.
2. Set `ResourceCost` to `20`.
3. Grant it to `AMABSCharacter`.
4. Verify the first activation emits `CostValidated` and `CostSpent`.
5. Repeatedly use abilities until the example resource pool drops below `20`.
6. Verify the next activation emits `CostRejected`.

## Not included in this phase

Phase 3 costs do not yet include:

* a built-in stat framework
* regeneration systems
* complex cost formulas
* cooldown reduction or discount stats
* refund systems
* multi-resource payments
