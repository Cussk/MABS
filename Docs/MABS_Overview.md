# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

It is a lightweight, multiplayer-ready, data-driven ability framework for Unreal Engine. The framework separates authored data from live runtime state and keeps gameplay approval on the server.

Abilities are split into clear layers:

* authored data in `UMABSAbilityDefinition`
* runtime grant state in `FMABSAbilitySpec`
* execution ownership in `UMABSAbilityComponent`
* observability in `FMABSAbilityDebugEvent`

## Why it exists

Most teams need the same ability building blocks:

* reusable data assets for designers
* a runtime owner component for characters and actors
* server-authoritative multiplayer behavior
* enough telemetry to understand request flow and failures

MABS is meant to provide that foundation in a form that is easy to inspect, extend, and ship.

## Who it is for

MABS is aimed at:

* indie teams
* small multiplayer projects
* programmers who want a clean extension point
* designers who want editable ability data assets

## What Phase 01 delivers

Phase 01 moves MABS past the skeleton stage. It adds:

* granted ability storage on `UMABSAbilityComponent`
* real `FMABSAbilitySpec` runtime tracking
* `TryActivateAbilityByTag` with a client-to-server RPC path
* authority-side validation and commit
* small, explicit failure reasons
* structured debug events for request start, send, accept, reject, and commit

This gives the project a real end-to-end path from an authored data asset to a multiplayer-safe activation request.

## What MABS does not try to solve in V1

MABS does not currently implement:

* client prediction
* advanced stacking rules
* duration-based effects
* complex targeting pipelines
* gameplay costs or cooldown execution
* ability UI packages

Those belong to later phases once the request and runtime state model are stable.

## How to use it

1. Create a `UMABSAbilityDefinition` asset with a valid ability tag.
2. Add `UMABSAbilityComponent` to the owning actor.
3. Grant the definition on the server.
4. Call `TryActivateAbilityByTag`.
5. Inspect `OnAbilityDebugEvent`, `GetRecentDebugEvents`, or the granted ability specs to see the outcome.

## Example

A typical Phase 01 flow is:

1. Create a `MABSAbilityDefinition` asset with tag `Ability.Test.Fireball`.
2. Add `UMABSAbilityComponent` to the player character.
3. Grant the definition on the server during setup.
4. Bind input to `TryActivateAbilityByTag(Ability.Test.Fireball)`.
5. In standalone or as the listen server host, the server accepts and commits immediately.
6. As a remote client, the request is sent to the server and the owning client receives structured acceptance or rejection events.
