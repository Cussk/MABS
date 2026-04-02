# Phase 4 - Delivery Foundation

## What it is

Phase 4 adds the first real delivery layer to MABS.

Added in this phase:

* authored `DeliveryMode`
* `Direct`, `HitTrace`, `Melee`, and `Projectile`
* authority-side hit trace execution
* authority-side melee sweep execution
* authoritative projectile spawn and impact handling
* delivery-specific debug events

## Why it exists

Without built-in delivery, MABS would still feel like a framework slice instead of a practical gameplay product.

Phase 4 adds the common combat paths most users expect while keeping the system small and authoritative.

## How it works

* `Direct` keeps the older target-resolution path
* `HitTrace` resolves a valid actor through an authority-side trace
* `Melee` resolves a valid actor through an authority-side sweep
* `Projectile` commits on successful projectile spawn and applies its effect later on authoritative impact

## Added runtime ownership

### `MABSCore`

* `EMABSDeliveryMode`
* delivery-authored fields on `UMABSAbilityDefinition`

### `MABSGameplay`

* delivery helper flow inside `UMABSAbilityComponent`
* `AMABSProjectileBase`
* projectile impact handling

### `MABSDebug`

* delivery-aware formatting
* overlay summaries that now include delivery mode

## Example verification

Recommended example abilities:

* direct self-heal
* hit-trace rifle shot
* melee sword slash
* projectile fireball

## Not included

Phase 4 still does not include:

* AoE zones
* location placement
* projectile homing
* chain projectiles
* multi-hit melee
* prediction

## Test checklist

Singleplayer:

* each delivery mode works
* cooldown and cost still integrate correctly
* delivery failures emit readable debug output

Listen server:

* remote clients still route through authority
* projectile spawn and impact remain authoritative

Dedicated server:

* delivery logic remains server-authoritative
* no editor-only dependency leaks into runtime code
