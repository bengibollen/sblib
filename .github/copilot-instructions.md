# GitHub Copilot Instructions for Silver Bucket Mudlib

## Build and runtime commands

Use the Docker workflow that is already documented in this repo:

```bash
docker-compose up -d
docker-compose logs -f
docker-compose down
```

Rebuild the mud image after changing the driver settings in `src/settings/sblib`, the Docker build, or startup wiring:

```bash
docker-compose build
```

The container starts LDMud through `driver.sh`, which runs:

```bash
/usr/local/bin/ldmud -m /mud/sblib -M secure/master.c --python-script ../ldmud-python/__main__.py
```

## High-level architecture

- The runtime entry point is Docker-based. `Dockerfile` builds a custom LDMud driver with the `src/settings/sblib` settings file, `docker-compose.yml` mounts this repository into `/mud/sblib`, and `driver.sh` starts the driver with `secure/master.c` as the master object.
- `lpc-config.json` mirrors those entry points for editor tooling: master is `/secure/master.c`, simul_efuns come from `/secure/simul_efun.c`, the default player object is `/std/player.c`, and include resolution is centered on `/inc` and `/sys`.
- `secure/master.c` is the mudlib bootstrap object. It owns connection handling, UID/EUID policy, include resolution, and simul_efun loading. Its implementation is intentionally split into `#include`d modules under `secure/master/*.c`, so changes to master behavior usually span both `secure/master.c` and one of those included files.
- New sessions enter through `master::connect()`, which clones `/secure/login`. `secure/login.c` handles the login/menu flow and hands successful sessions off to player objects.
- Player behavior is layered. `/std/player.c` is the public customization layer, but the durable implementation lives in `/std/player_sec.c`, which statically includes modules from `/std/player/*.c` for save state, commands, combat, death handling, paging, and message flow.
- Core world objects are also layered. `/std/object.c` provides the property/reset base, `/std/container.c` builds container behavior on top of it, and `/std/room.c` composes room behavior by `#include`ing `std/room/*.c` modules for exits, descriptions, movement, terrain, links, and room-owned objects.
- Command handling is soul-based. `/std/command_driver.c` inherits `/lib/commands.c`; individual soul objects expose a `query_cmdlist()` mapping, and `/std/living/cmdhooks.c` loads and dispatches them. When debugging missing verbs, trace the soul mapping and the living command hooks, not just the verb implementation.
- Combat and NPC behavior are mixin-heavy. For example, `/std/monster.c` combines `/std/npc`, `/std/combat/humunarmed`, and several `/std/act/*` modules. Use `doc/sblib/concepts/combat_calculations.md` when changing damage, AC, penetration, or aggro behavior; the formulas and practical tuning notes live there.

## Key conventions

- `#pragma strict_types` is common in `secure/`, `std/`, and `lib/`. In those files, remote calls are routinely cast explicitly, for example `({int}) ob->query_prop(...)` or `({string}) player->query_real_name()`. Preserve that style instead of relying on implicit typing.
- Large LPC objects in this mudlib are often split with `#include`d `.c` fragments rather than extra inheritance layers. Before editing a subsystem like players, rooms, living objects, or the master object, check whether the real logic lives in an included fragment.
- Constructor/reset hooks are inheritance-specific. Customize rooms through `create_room()` and `reset_room()`, not `create_container()` or `reset_container()`. The same pattern appears elsewhere with hooks like `create_object()`, `create_npc()`, and `reset_object()`.
- Room reset behavior is part of the design, not an afterthought. `std/room.c` auto-enables resets when `reset_room()` is overridden, runs `reset_auto_objects()`, and tracks room-owned clones through helpers like `clone_here()` and `add_accepted_here()`.
- Souls are command mappings, not ad-hoc command files. New or overridden command sets should follow the existing `query_cmdlist()` pattern and the UID setup path in `open_soul()` / `teleledningsanka()`.
- Prefer modern closures and call scheduling when touching legacy code: use `#'func` or `(: ... :)` instead of old `&` closure syntax, prefer `call_out()` over `set_alarm()`, and keep the modern `member(array, item)` argument order.
