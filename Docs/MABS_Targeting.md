# MABS Targeting

## What it is

This document explains the current target-intent model for MABS after Phase 4.

## Why it exists

Phase 4 adds delivery, but target intent and delivery are still separate concepts. Keeping them separate makes authored data easier to reason about.

## Current target intent

MABS still authors target intent through `TargetType`.

Current supported target intent values:

* `Self`
* `Actor`

`Location` remains out of scope.

## How target intent works now

### `Self`

Used with `Direct` delivery for simple self-targeted abilities such as self-heal.

### `Actor`

Used for:

* direct actor-targeted abilities
* hit-trace delivery
* melee delivery
* projectile delivery

## Direct targeting

`DeliveryMode = Direct` keeps the existing target-resolution path.

For direct actor targeting, authored fields are still:

* `TargetTraceDistance`
* `ActorTargetTraceMode`
* `TargetTraceRadius`
* `bRequireValidActorTarget`
* `bIgnoreNonTargetWorldHits`

## Delivery targeting

When `DeliveryMode` is not `Direct`, the delivery step resolves the final actor:

* `HitTrace` resolves the first valid trace hit
* `Melee` resolves the first valid short-range sweep hit
* `Projectile` resolves the actor on authoritative impact

## Validation rules

Current valid-target rules remain lightweight:

* self is rejected for actor-targeted hostile delivery
* damage accepts damageable actors or pawns
* heal accepts actors implementing `IMABSInstantEffectReceiver`

## Debug visibility

Use:

* `GetLatestTargetTraceDebugInfo()`
* `TargetTraceStarted`, `TargetTraceHit`, `TargetTraceRejected`
* `HitTraceHit`, `HitTraceRejected`
* `MeleeHit`, `MeleeRejected`

## Not included

This phase still does not add:

* location targeting
* AoE targeting
* team filters
* advanced target query objects
