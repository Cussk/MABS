# MABS Sockets

## What it is

This document explains the Phase 5 socket-first origin model.

## Why it exists

Raw offsets alone are too weak for real combat authoring. Socket-first origins let traces, sweeps, and projectile spawns line up with weapons, hands, or mesh attachment points.

## Authored fields

Shared field:

* `DeliveryOriginSocketName`

Delivery-specific fields:

* `HitTraceOriginSocketName`
* `MeleeOriginSocketName`
* `ProjectileSpawnSocketName`

Optional offsets remain available:

* `HitTraceOriginOffset`
* `MeleeOriginOffset`
* `ProjectileSpawnOffset`

## Resolution order

For hit trace, melee, and projectile delivery, MABS resolves origins in this order:

1. delivery-specific socket field
2. `DeliveryOriginSocketName`
3. safe fallback transform

Fallback behavior:

* hit trace and projectile prefer viewpoint fallback
* melee prefers actor transform fallback

Offsets are applied after the socket or fallback transform is resolved.

## Debug visibility

MABS emits:

* `SocketResolved`
* `SocketFallbackUsed`

This makes it clear whether the authored socket was found or a fallback path was used.

For montage-driven abilities, MABS also refreshes the authoritative skeletal pose before reading the socket transform so hit traces, melee sweeps, and projectile spawns follow the current animated socket location instead of a stale server pose.

## Example

Example rifle shot:

* `DeliveryMode = HitTrace`
* `HitTraceOriginSocketName = Muzzle`
* `HitTraceOriginOffset = (10, 0, 0)`

Example fireball:

* `DeliveryMode = Projectile`
* `ProjectileSpawnSocketName = hand_r`

If those sockets are missing, MABS falls back safely and emits a readable debug event.

## Test checklist

Singleplayer:

* valid sockets are used
* missing sockets fall back safely

Listen server:

* the owning client receives socket debug events

Dedicated server:

* socket lookup remains runtime-safe
