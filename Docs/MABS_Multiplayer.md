# MABS Multiplayer

## What it is

This document explains how Phase 2 handles multiplayer authority for activation, target resolution, and instant effect application.

## Why it exists

Ability systems are easy to make locally and easy to break in multiplayer. MABS keeps the activation decision, target resolution, and effect application on the server so later phases do not need to undo local-only assumptions.

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

This keeps the final result authoritative while still giving the client visibility into what happened.

## Targeting and effects in multiplayer

### `Self`

The server resolves the owner actor directly.

This is the simplest Phase 2 path and is ideal for self-heal testing.

### `Actor`

The server performs the forward trace from the owning actor and resolves a single actor target.

If the trace misses, the request fails cleanly with `TargetResolutionFailed`.

### Damage

The server applies damage through Unreal's generic damage path.

### Heal

The server applies heal only if the target implements `IMABSInstantEffectReceiver`.

## Standalone, listen server, and dedicated server

### Standalone

Standalone uses the authority path directly because the local world is authoritative.

### Listen server

The host player also uses the authority path directly. Remote clients on that session still go through the server RPC path.

### Dedicated server

All player activation requests go to the dedicated server, which validates, resolves targets, applies effects, and sends debug results back to the owning client.

## How to use it

1. Grant abilities on the server.
2. Call `TryActivateAbilityByTag` from client input or server gameplay code.
3. Expect a local `RequestSentToServer` return value on remote clients.
4. Use `OnAbilityDebugEvent` or logs to inspect the authoritative target and effect results.

## Example

Example multiplayer verification flow:

1. Start PIE with one listen server and one client.
2. Grant `Test.Ability.SelfHeal` on the server-owned player character.
3. Press the bound activation input on the remote client.
4. Verify the client records `RequestSentToServer`.
5. Verify the server records `RequestAccepted`, `TargetResolved`, `EffectApplied`, and `CommitSucceeded`.
6. Verify the client receives those authoritative events from the server.

## Test checklist

Singleplayer:

* self-heal resolves the owner and succeeds
* actor damage succeeds when a target actor is traced
* actor damage fails cleanly when no actor is found

Listen server:

* host self-heal succeeds through authority-local execution
* host damage ability succeeds through authority-local execution
* remote client uses the RPC path
* remote client receives the final authoritative target/effect result

Dedicated server:

* remote client requests reach the server
* the server resolves targets and applies effects correctly
* there are no client-only targeting or effect shortcuts
