# MABS

MABS is a lightweight, multiplayer-ready, data-driven ability framework for Unreal Engine. The repository contains the reusable MABS plugin, a C++ Unreal project that demonstrates it, sample ability assets, and documentation for the major runtime systems.

The goal of the project is to make common action-game ability behavior authorable through data assets while keeping gameplay authority on the server. Abilities can be granted individually or through ability sets, activated by gameplay tag, and driven through startup, delivery, recovery, cost, cooldown, effect, presentation, and debug flows.

## Current State

MABS is an active Unreal Engine project and framework prototype. The core runtime is implemented as a plugin and the sample project is set up to prove the current behavior in editor, standalone, listen-server, and dedicated-server style workflows.

Currently included:

- Plugin modules for shared data, gameplay execution, runtime debugging, and editor validation.
- `UMABSAbilityComponent` for ability granting, activation, replicated runtime state, cooldowns, costs, combos, AoE, periodic effects, and debug summaries.
- Data assets for ability definitions and grouped ability sets.
- Built-in delivery paths for direct effects, hit traces, melee traces, and projectiles.
- Custom delivery handler support in C++ or Blueprint.
- Presentation hooks for montages, gameplay cues, sounds, VFX, camera shake, tracers, and projectile travel.
- A runtime debug HUD with structured ability events and state summaries.
- Editor validation for common ability authoring mistakes.
- Sample abilities for self-heal, rifle shot, sword swing combos, fireball, AoE, periodic effects, and custom knockback.

This is not a polished game or packaged marketplace release. It is best treated as a source project and plugin testbed for experimenting with multiplayer ability-system architecture.

## Requirements

- Unreal Engine 5.7, matching the project association in `MABS.uproject`.
- A C++ Unreal toolchain such as Visual Studio or Rider.
- Windows is the currently configured desktop target in the project settings.

The project uses Unreal systems and plugins including Gameplay Tags, Enhanced Input, Niagara, Data Validation, StateTree, Gameplay StateTree, UMG, and standard networking support.

## Project Layout

- `Plugins/MABS/Source/MABSCore` - shared ability data, runtime structs, gameplay tags, presentation structs, and debug read models.
- `Plugins/MABS/Source/MABSGameplay` - authoritative runtime execution, `UMABSAbilityComponent`, delivery handlers, projectiles, effects, replication, and presentation routing.
- `Plugins/MABS/Source/MABSDebug` - debug HUD and Blueprint formatting helpers.
- `Plugins/MABS/Source/MABSEditor` - editor-only validation for authored ability assets.
- `Source/MABS` - sample project code, demo HUD, sample character/controller/game mode, and example custom delivery handler.
- `Content/Data` - sample ability definitions and starter ability set.
- `Content/Maps/TestAbilities.umap` - ability test map content.
- `Docs` - detailed documentation for the runtime systems and authoring model.

## Getting Started

1. Clone the repository with the `Content` directory included.
2. Open `MABS.uproject` in Unreal Engine 5.7.
3. Let Unreal rebuild the C++ modules if prompted.
4. Open the default third-person map or `Content/Maps/TestAbilities.umap`.
5. Play in editor and use the sample HUD to inspect granted abilities, cooldowns, costs, targeting, combos, and periodic effects.

Useful runtime toggles:

- `F1` toggles the demo help panel.
- `F2` toggles the technical debug harness.
- `F3` toggles validation notes in the sample HUD.

## Ability Authoring Model

Abilities are authored as `UMABSAbilityDefinition` assets. A definition can specify:

- Ability tag and display name.
- Activation policy.
- Delivery mode and optional custom delivery handler.
- Targeting rules.
- Startup, delivery, and recovery timing.
- Instant damage or healing.
- Cooldown group and resource cost.
- Combo follow-up windows.
- AoE shape and size.
- Periodic damage or healing.
- Montage, cue, sound, VFX, camera shake, tracer, and projectile presentation data.

Multiple abilities can be grouped in a `UMABSAbilitySet` and granted together by authority.

## Multiplayer Model

MABS is server-authoritative. The server owns grants, activation validation, cooldowns, costs, delivery, target resolution, projectile impact, gameplay effects, periodic ticks, and combo timing. Clients can request activation and combo follow-ups, while replicated state and owner-routed debug data make the result inspectable on the owning client.

A dedicated server target is included in `Source/MABSServer.Target.cs`.

## Documentation

Start with:

- `Docs/MABS_Overview.md`
- `Docs/MABS_QuickStart.md`
- `Docs/MABS_API.md`
- `Docs/MABS_Architecture.md`
- `Docs/MABS_Multiplayer.md`
- `Docs/MABS_Debugging.md`
- `Docs/MABS_Validation.md`

Additional docs cover ability sets, targeting, delivery, projectiles, melee, cooldowns, costs, combos, AoE, periodic effects, presentation, VFX, SFX, camera shake, sockets, timing, and module ownership.
