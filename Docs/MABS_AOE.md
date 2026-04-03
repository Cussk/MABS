# MABS AoE

## What it is

This document explains the basic Phase 7 AoE model in MABS.

## Why it exists

Common combat abilities need an explosion, pulse, or small impact burst that affects more than one target. Phase 7 adds that breadth with a small authored shape model.

## Authored fields

Use the `AoE` group on an ability:

* `bEnabled`
* `Shape`
* `Radius`
* `BoxExtent`
* `CapsuleRadius`
* `CapsuleHalfHeight`
* `Offset`

Supported shapes are:

* `Sphere`
* `Box`
* `Capsule`

## Runtime behavior

On authority, MABS:

1. resolves the normal delivery impact context
2. builds an AoE center from that impact
3. gathers overlapping actors in the authored shape
4. validates the gathered actors for the authored gameplay effect
5. applies instant and periodic effects to the valid set

The primary target is still included when valid, so AoE does not replace the original hit.

## How to use it

1. Author the normal delivery mode first.
2. Enable `AoE`.
3. Pick a shape and set the shape size.
4. Add an offset only when the burst should not be centered exactly on the hit.
5. Activate the ability on authority.

## Example

Example explosion:

* `DeliveryMode = Projectile`
* `AoE.bEnabled = true`
* `AoE.Shape = Sphere`
* `AoE.Radius = 300`
* `AoE.Offset = (0, 0, 0)`

## Not included

Phase 7 does not add:

* persistent world volumes
* team filtering layers
* placement abilities
* aura systems
