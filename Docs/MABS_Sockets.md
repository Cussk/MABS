# MABS Sockets

## What it is

This document explains the Phase 6 socket-first origin model for both gameplay delivery and presentation.

## Why it exists

Raw offsets alone are too weak for real combat authoring. Socket-first origins let traces, sweeps, projectile spawns, and presentation cues line up with weapons, hands, or mesh attachment points.

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

Presentation cues also support:

* `SocketName`
* `LocationOffset`
* `RotationOffset`

## Resolution order

For hit trace, melee, and projectile delivery, MABS resolves origins in this order:

1. delivery-specific socket field
2. `DeliveryOriginSocketName`
3. safe fallback transform

Fallback behavior:

* hit trace and projectile prefer viewpoint fallback
* melee prefers actor transform fallback

Offsets are applied after the socket or fallback transform is resolved.

For startup and delivery presentation, MABS resolves origins in this order:

1. the presentation cue socket override
2. the delivery-mode socket
3. `DeliveryOriginSocketName`
4. a safe fallback transform

## Debug visibility

MABS emits:

* `SocketResolved`
* `SocketFallbackUsed`
* `PresentationSocketFallbackUsed`

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
* `DeliveryPresentation.Cue.SocketName = hand_r`

If those sockets are missing, MABS falls back safely and emits a readable debug event.

## Test checklist

Singleplayer:

* valid sockets are used
* missing sockets fall back safely

Listen server:

* the owning client receives socket debug events

Dedicated server:

* socket lookup remains runtime-safe
