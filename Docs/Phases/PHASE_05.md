# Phase 5 - Timing, Sockets, and Montage Integration

## What it is

Phase 5 adds:

* authored startup, delivery, and recovery timing
* socket-first origins for hit trace, melee, and projectile delivery
* optional activation montage requests
* clearer runtime state and debug visibility

## Why it exists

Phase 4 proved the delivery modes. Phase 5 makes them feel authorable and inspectable by adding explicit timing and better origin control.

## What changed

### `MABSCore`

* added timing fields on `UMABSAbilityDefinition`
* added socket fields on `UMABSAbilityDefinition`
* added optional montage fields on `UMABSAbilityDefinition`
* expanded `FMABSAbilitySpec` with timing timestamps
* expanded `EMABSAbilityRuntimeState` with `Startup` and `Recovery`

### `MABSGameplay`

* activation now moves through startup, delivery, and recovery timers
* hit trace, melee, and projectile delivery now resolve socket-first origins
* projectile spawn timing now follows the authored delivery time
* optional montage requests can fire during startup

### `MABSDebug`

* added formatting for timing and socket events
* runtime summaries now show startup and recovery timing

## Example verification

Recommended example abilities:

* self-heal with startup and recovery timing
* rifle shot with a muzzle socket hit trace
* sword slash with a hand or weapon socket melee origin
* fireball with a projectile spawn socket and optional montage

## Not included

Phase 5 does not add:

* notify-driven execution as the main path
* combo systems
* AoE
* DOT or HOT
* advanced montage branching

## Test checklist

Singleplayer:

* authored timing works for every delivery mode
* socket fallbacks work
* montage requests do not replace timing

Listen server:

* authority still controls timing
* the owning client receives timing, socket, and delivery events

Dedicated server:

* delivery timing remains authoritative
* server-safe runtime modules still build without editor-only dependencies
