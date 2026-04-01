# MABS Multiplayer

## What it is

This document explains how Phase 01 handles multiplayer authority for ability activation.

## Why it exists

Ability systems are easy to make locally and easy to break in multiplayer. MABS keeps the activation decision on the server from the first real gameplay slice so later phases do not need to undo local-only assumptions. Phase 1.5 keeps that authority model unchanged while moving the code into plugin modules.

## Server authority

The server owns gameplay approval.

That means:

* grants should be made on authority
* activation success is decided on authority
* clients may request activation, but they do not commit it locally

## Client behavior

When a remote client calls `TryActivateAbilityByTag`:

1. the local component records `RequestStarted`
2. the local component sends `ServerTryActivateAbilityByTag`
3. the local return value is `RequestSentToServer`
4. the server validates and either accepts or rejects the request
5. the server mirrors the authoritative debug event back to the owning client

This keeps the final result authoritative while still giving the client visibility into what happened.

## Standalone, listen server, and dedicated server

### Standalone

Standalone uses the authority path directly because the local world is authoritative.

### Listen server

The host player also uses the authority path directly. Remote clients on that session still go through the server RPC path.

### Dedicated server

All player activation requests go to the dedicated server, which decides success or failure and sends the debug result back to the owning client.

## How to use it

1. Grant abilities on the server.
2. Call `TryActivateAbilityByTag` from client input or server gameplay code.
3. Expect a local `RequestSentToServer` return value on remote clients.
4. Use `OnAbilityDebugEvent` or logs to inspect the final authoritative result.

## Example

Example multiplayer verification flow:

1. Start PIE with one listen server and one client.
2. Grant `Test.Ability.Fireball` on the server-owned player character.
3. Press the bound activation input on the remote client.
4. Verify the client records `RequestSentToServer`.
5. Verify the server records `RequestAccepted` and `CommitSucceeded`.
6. Verify the client receives the matching authoritative event from the server.

## Test checklist

Singleplayer:

* valid granted ability activates successfully
* invalid tag fails with `InvalidAbility`
* ungranted tag fails with `NotGranted`

Listen server:

* host activates through authority-local execution
* remote client uses the RPC path
* remote client receives the final authoritative result

Dedicated server:

* remote client requests reach the server
* the server commits or rejects correctly
* there are no client-only activation shortcuts
