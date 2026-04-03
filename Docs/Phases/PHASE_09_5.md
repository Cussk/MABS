# PHASE_09_5 - True Runtime Refactor and De-Monolith Pass

## What shipped

Phase 9.5 replaces the previous `.inl`-based intermediate split with real private runtime implementation units inside `MABSGameplay`.

The implemented Phase 9.5 result includes:

* `MABSAbilityRuntime_Internal.h`
* `MABSAbilityRuntime_Internal.cpp`
* `MABSAbilityRuntime_Core.cpp`
* `MABSAbilityRuntime_Granting.cpp`
* `MABSAbilityRuntime_Activation.cpp`
* `MABSAbilityRuntime_Delivery.cpp`
* `MABSAbilityRuntime_Effects.cpp`
* `MABSAbilityRuntime_Presentation.cpp`
* `MABSAbilityRuntime_Debug.cpp`
* a much smaller `MABSAbilityComponent.cpp`
* updated architecture and ownership docs that describe the real runtime structure

## Why it matters

The previous Phase 9 pass improved readability, but it was still fundamentally a layout pass around the same owner.

Phase 9.5 matters because:

* the main component source is no longer the place where every runtime concern is compiled together
* future grant, activation, delivery, effect, presentation, and debug work now have obvious homes
* the public API and authoritative state stay stable while the runtime implementation becomes more scalable

## What changed

### `UMABSAbilityComponent`

Still owns:

* public Blueprint and C++ API
* authoritative runtime state
* replication boundaries
* orchestration entry points

No longer relies on:

* a private `.inl` include chain as the primary internal runtime structure

### `MABSGameplay`

Now owns clearer internal runtime areas:

* granting runtime
* activation and combo runtime
* delivery and targeting runtime
* effects, cooldown, cost, and periodic runtime
* presentation routing runtime
* debug summary and event runtime

### `MABSDebug`

Still owns:

* formatting
* harness rendering
* section layout
* event filtering

## What state it uses

Phase 9.5 keeps the same authoritative state on `UMABSAbilityComponent`:

* `GrantedAbilities`
* `CooldownGroupStates`
* `RecentDebugEvents`
* `LatestTargetTraceDebugInfo`
* `ActivePeriodicEffects`
* `ActiveAbilityExecutionContexts`
* `ActivePeriodicEffectRuntimes`
* reusable presentation Niagara pools

## How to use it

Plugin users do not need a new setup flow.

You still:

1. grant abilities or ability sets on authority
2. activate abilities by tag
3. inspect runtime state through the same summary accessors and debug harness

This phase is an internal runtime refactor, not a gameplay API redesign.

## Example

Example future work placement after Phase 9.5:

* new grouped grant handling belongs in `MABSAbilityRuntime_Granting.cpp`
* new combo-window logic belongs in `MABSAbilityRuntime_Activation.cpp`
* new target resolution logic belongs in `MABSAbilityRuntime_Delivery.cpp`
* new periodic status handling belongs in `MABSAbilityRuntime_Effects.cpp`
* new cue routing behavior belongs in `MABSAbilityRuntime_Presentation.cpp`
* new debug summaries belong in `MABSAbilityRuntime_Debug.cpp`

## Validation

Code validation completed with:

* `Build.bat MABSEditor Win64 Development ...`

Current result:

* the Phase 9.5 runtime refactor compiles and links successfully in the local editor target

## Recommended runtime testing

Singleplayer:

* grant and activate direct, hit trace, melee, and projectile abilities
* verify combo queueing, AoE, cooldowns, costs, and periodic effects
* verify the harness still reports readable runtime state

Listen server:

* verify host authority behavior and remote owner visibility
* verify presentation and debug visibility remain correct

Dedicated server:

* verify authority still owns gameplay results
* verify owner-facing debug state still works when enabled
* verify gameplay still works when the harness is disabled
