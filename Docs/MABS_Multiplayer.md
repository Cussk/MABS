# MABS Multiplayer

## What it is

This document explains how Phase 2.5 handles multiplayer authority for activation, target resolution, instant effect application, and runtime debug visibility.

## Why it exists

Ability systems are easy to make locally and easy to break in multiplayer. MABS keeps gameplay results on the server while still giving the owning client enough visibility to understand what the authority path did.

## Server authority

The server owns gameplay approval and gameplay results.

That means:

* grants should be made on authority
* target resolution is decided on authority
* damage and heal application happen on authority
* clients may request activation, but they do not resolve targets or commit gameplay results locally

## Client behavior

When a remote client calls `TryActivateAbilityByTag`:

1. the local component records `RequestStarted`
2. the local component sends `ServerTryActivateAbilityByTag`
3. the local return value is `RequestSentToServer`
4. the server validates, resolves the target, and applies the effect
5. the server mirrors the authoritative debug events back to the owning client
6. the server also mirrors the latest authoritative `FMABSTargetTraceDebugInfo` snapshot when actor targeting is used

This keeps the final result authoritative while still giving the client visibility into what happened.

## Targeting in multiplayer

### `Self`

The server resolves the owner actor directly.

### `Actor`

The server performs the configured actor trace from the authoritative viewpoint, validates the hit result, and stores the latest trace snapshot.

Phase 2.5 improvements that matter in multiplayer:

* configurable line or sphere traces
* clearer rejection reasons
* latest trace snapshot mirrored to the owning client
* optional trace debug draw on the authority path

## Effects in multiplayer

### Damage

The server applies damage through Unreal’s generic damage path.

### Heal

The server applies heal only if the resolved target implements `IMABSInstantEffectReceiver`.

## Runtime overlay in multiplayer

`AMABSDebugHUD` is client-side runtime UI. It does not make gameplay decisions.

It reads:

* local recent debug events
* the latest authoritative target trace snapshot for the local owner

This keeps the overlay runtime-safe for standalone, listen server, and remote clients.

## Standalone, listen server, and dedicated server

### Standalone

Standalone uses the authority path directly because the local world is authoritative.

### Listen server

The host player also uses the authority path directly. Remote clients on that session still go through the server RPC path.

### Dedicated server

All player activation requests go to the dedicated server, which validates, resolves targets, applies effects, and sends debug results back to the owning client.

## Example

Example multiplayer verification flow:

1. Start PIE with one listen server and one client.
2. Grant `Test.Ability.Fireball` on the server-owned player character.
3. Press the bound activation input on the remote client.
4. Verify the client records `RequestSentToServer`.
5. Verify the server records `TargetTraceStarted`, then `TargetTraceHit` or `TargetTraceRejected`.
6. Verify the server applies the effect only on authority.
7. Verify the client receives the final authoritative debug events and latest trace snapshot.

## Test checklist

Singleplayer:

* self-heal resolves the owner and succeeds
* actor damage succeeds when a valid actor is traced
* actor damage fails cleanly when no valid actor is found

Listen server:

* host self-heal succeeds through authority-local execution
* host damage ability succeeds through authority-local execution
* remote client uses the RPC path
* remote client receives the final authoritative target, effect, and trace-debug result

Dedicated server:

* remote client requests reach the server
* the server resolves targets and applies effects correctly
* there are no client-only targeting or effect shortcuts
* the runtime overlay remains client-safe
