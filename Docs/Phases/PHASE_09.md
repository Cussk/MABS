# Phase 9 - Refactor, Cleanup, and Pre-v1 Readiness

## What it is

Phase 9 is the structural cleanup pass for MABS before v1 packaging.

This slice keeps the existing gameplay feature set intact and focuses on making the runtime codebase easier to read, maintain, and document.

## What changed

Phase 9 now:

* keeps `UMABSAbilityComponent` as the public orchestration surface
* splits the private component implementation into focused responsibility files
* keeps debug rendering in `MABSDebug`
* keeps runtime state on the component instead of scattering it into new helper objects
* updates the architecture and module docs to match the cleaned ownership split

## Why it exists

By the end of Phase 8, MABS already supported:

* ability sets
* cooldowns and costs
* direct, trace, melee, and projectile delivery
* combos
* AoE
* periodic effects
* the runtime debug harness

The problem was not missing breadth. The problem was that too much of that behavior lived in one monolithic source file.

## Which subsystem owns it

This cleanup is owned by `MABSGameplay`.

`MABSDebug` still owns harness rendering and formatting. `MABSCore` still owns data and shared runtime/debug types.

## What state it uses

The refactor keeps the same authoritative state on `UMABSAbilityComponent`:

* `GrantedAbilities`
* `CooldownGroupStates`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`
* `ActivePeriodicEffects`
* `ActiveAbilityExecutionContexts`
* `ActivePeriodicEffectRuntimes`

## How to use it

Plugin users do not need a new setup flow.

You still:

1. grant abilities or ability sets on authority
2. activate abilities by tag
3. inspect runtime state through the same summary accessors and debug harness

The cleanup is internal structure work, not a gameplay API redesign.

## Example

The activation path is now easier to navigate by concern:

* granting logic lives in `MABSAbilityComponent_Granting.inl`
* startup / combo / recovery logic lives in `MABSAbilityComponent_Activation.inl`
* targeting and delivery live in `MABSAbilityComponent_Delivery.inl`
* effects, periodic runtime, cost, and cooldown live in `MABSAbilityComponent_Effects.inl`
* read-model and event helpers live in `MABSAbilityComponent_Debug.inl`
* cue and montage routing live in `MABSAbilityComponent_Presentation.inl`

## Testing

Recommended regression coverage for Phase 9:

* standalone grant and activation flow
* listen-server activation and owner-facing debug visibility
* dedicated-server activation and owner-only debug replication
* projectile, combo, AoE, and periodic-effect regression passes

Current validation note:

* `MABSEditor Win64 Development` compiled through `MABSAbilityComponent.cpp`
* the final DLL link was blocked because `UnrealEditor-MABSGameplay.dll` was locked by a running `UnrealEditor.exe`

## Not included

Phase 9 still does not add:

* new delivery modes
* new combat systems
* prediction
* deeper stacking rules
* editor customization breadth

It is still a cleanup and readability phase.
