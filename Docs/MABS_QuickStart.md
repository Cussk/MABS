# MABS Quick Start

## Purpose

This guide covers the current Phase 00 setup. At this stage, MABS establishes the authored data, runtime component, and debug flow. It does not execute full gameplay ability logic yet.

## Step 1: Build the project

Compile the `MABS` module so the new ability types are available in the editor.

Verify that the project opens without module load errors.

## Step 2: Add the ability component

Add `UMABSAbilityComponent` to the actor or character that should own abilities.

The owning actor should be the authoritative source for grants and later activation, which usually means the server-owned pawn or character.

## Step 3: Create an ability definition asset

Create a `MABSAbilityDefinition` data asset and set:

* `AbilityTag`
* `TargetType`
* `CooldownSeconds`
* `ResourceCost`

In Phase 00, cooldown and cost are placeholders. They exist so authored data already has the right home.

## Step 4: Grant an ability on the server

Call `GrantAbility` on the component with the definition asset.

This stores a runtime `FMABSAbilitySpec` with a lightweight `FMABSAbilityHandle`. If the call runs on a non-authoritative owner, the component rejects it and emits a debug event.

## Step 5: Request activation and inspect debug output

Call `TryActivateAbilityByTag` with the definition's tag.

Phase 00 returns `NotImplemented` for a valid granted ability, but it still records a structured debug event and log entry so later phases can reuse the same observability path.

## Example

Example setup for a character blueprint:

1. Add `MABSAbilityComponent`.
2. Expose a `MABSAbilityDefinition` asset reference.
3. On server `BeginPlay`, call `GrantAbility`.
4. Bind to `OnAbilityDebugEvent` to print or inspect the result.
5. Trigger `TryActivateAbilityByTag` from an input event or test button.

## Validation checklist

Singleplayer:

* project compiles
* `UMABSAbilityComponent` can be added to an actor
* `MABSAbilityDefinition` assets can be created
* `GrantAbility` creates a runtime entry

Listen server:

* grant requests succeed on the server owner
* client-side calls report `RequiresAuthority`

Dedicated server:

* `MABSServer.Target.cs` is included for dedicated server builds
* actual server build verification requires a source-built Unreal Engine because Epic's installed binary distribution does not support server target compilation
* authoritative grant and activation stub paths avoid editor-only dependencies
