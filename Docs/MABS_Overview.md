# MABS Overview

## What MABS is

MABS stands for **Multiplayer Ability System**.

It is a lightweight, multiplayer-ready, data-driven ability framework for Unreal Engine. The framework is intended to cover the common building blocks teams need for gameplay abilities without forcing them into a much larger system than their project needs.

Abilities are split into clear layers:

* authored data in `UMABSAbilityDefinition`
* runtime grant state in `FMABSAbilitySpec`
* execution ownership in `UMABSAbilityComponent`
* observability in `FMABSAbilityDebugEvent`

## Why it exists

Most teams need the same core ability concepts:

* a reusable definition asset
* a runtime owner component
* multiplayer-safe authority rules
* enough debug visibility to understand what the system is doing

MABS is meant to provide that foundation in a form that is easier to inspect, extend, and ship than a one-off ability prototype.

## Who it is for

MABS is aimed at:

* indie teams
* small multiplayer projects
* programmers who want a clean extension point
* designers who want editable ability data assets

## What Phase 00 delivers

Phase 00 establishes the foundation only. It adds:

* core ability enums and runtime structs
* a `UMABSAbilityDefinition` data asset
* a `UMABSAbilityComponent` that can grant abilities and accept stub activation requests
* a dedicated ability log category and structured debug events

This gives the project a usable architecture baseline without introducing full activation, effects, cooldown systems, or prediction.

## What MABS does not try to solve in V1

MABS does not currently implement:

* client prediction
* advanced stacking rules
* duration-based effects
* complex targeting pipelines
* ability UI packages

Those belong to later phases once the runtime foundation is stable.

## Example

A typical Phase 00 flow is:

1. Create a `MABSAbilityDefinition` asset with an ability tag.
2. Add `UMABSAbilityComponent` to the owning actor.
3. Grant the definition on the server.
4. Call `TryActivateAbilityByTag`.
5. Inspect `OnAbilityDebugEvent` or `GetRecentDebugEvents` to see the structured result.
