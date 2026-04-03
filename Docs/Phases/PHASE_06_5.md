# PHASE_06_5 - Lightweight Presentation Cue Routing and Pooling Foundations

## What shipped

Phase 6.5 keeps the Phase 6 authored presentation layout and adds:

* `VisibilityPolicy` on shared cue, tracer, and projectile-travel presentation data
* shared runtime cue payload structs in `MABSCore`
* lightweight cue-routing helpers in `UMABSAbilityComponent`
* owner-only, local-only, and relevant-client cosmetic routing
* projectile-travel cue realization through `AMABSProjectileBase`
* pooling-friendly Niagara reuse for repeated world cues and tracers
* routing-focused debug events

## Why it matters

Gameplay code still decides when startup, delivery, tracer, projectile travel, and impact happen.

Phase 6.5 changes how cosmetics are realized:

* gameplay no longer directly owns every cosmetic spawn decision
* dedicated servers skip local cosmetic realization cleanly
* repeated small Niagara effects have a lightweight reuse path
* visibility decisions are visible in debug output

## Implemented runtime path

Startup, delivery, and impact:

* build `FMABSPresentationCueEvent`
* resolve visibility policy
* route through multicast, owner-only, or local-only handling
* realize locally through centralized helpers

Hit-trace tracer:

* build `FMABSTracerCueEvent`
* route through the same small policy model
* reuse inactive local Niagara tracer components when possible

Projectile travel:

* build `FMABSProjectileTravelCueEvent`
* evaluate the policy on the replicated projectile actor
* realize locally only when that instance should see it

## Debug visibility

Phase 6.5 now emits or highlights:

* `PresentationCueRouted`
* `PresentationCueSkipped`
* `PresentationCueRealized`
* `PresentationCuePolicyFallbackUsed`
* `TracerCueRouted`
* `TracerCueSkipped`
* `TracerCueRealized`

Phase-specific trigger events from Phase 6 still remain.

## Module ownership

### `MABSCore`

Owns:

* cue visibility enums
* cue phase enum
* shared runtime cue payload structs
* authored cue policy fields

### `MABSGameplay`

Owns:

* cue routing helpers
* local realization helpers
* policy handling
* Niagara reuse pools for repeated world cues and tracers
* projectile-travel cue hookup

### `MABSDebug`

Owns:

* display and formatting of routing-focused debug events

## Documentation updated

Phase 6.5 updates or adds:

* `Docs/MABS_Presentation.md`
* `Docs/MABS_Debugging.md`
* `Docs/MABS_Multiplayer.md`
* `Docs/MABS_Modules.md`
* `Docs/MABS_Cues.md`
* `Docs/MABS_PresentationPerformance.md`

## Verification

Code compile verification completed with:

* `Build.bat MABSEditor Win64 Development ...`

Recommended runtime validation:

* singleplayer startup, delivery, tracer, projectile-travel, and impact cues
* listen-server owner-only and relevant-client cue visibility
* dedicated-server skip behavior for local-only cues
* repeated rifle-fire tracer and impact stability
