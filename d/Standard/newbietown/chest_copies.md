The duplicate chests are almost certainly caused by the room being **reloaded/reset** by the mud’s maintenance/cleanup, and this room’s `create_room()` *always* does:

```c
add_object("/d/Standard/newbietown/obj/chest", 1);
```

So every time the room object is re-created (or the room’s create is run again due to a reload), it will spawn a new chest. If old chest instances weren’t cleaned up (because they’re considered “in use”, not destructed, moved elsewhere, or the cleanup doesn’t touch them), they accumulate over hours.

## What’s happening (likely lifecycle)
Typical mudlibs do one or more of these over time:

1. **Periodic reset**: calls something like `reset_room()` / `reset()` to repopulate items/mobs.
2. **Auto-reload of code**: an admin update or an automated “reclaim memory” mechanism destructs and reloads clones/rooms.
3. **Room caching/hibernation**: unused rooms get destructed; when a player re-enters, the room is loaded fresh (running `create_room()` again).

In any of those cases, if the chest object isn’t reliably removed first, you’ll get another one.

## Evidence in this file
The room currently does *no* guarding around the chest spawn—no check for an existing chest, and no “only do this on first time” logic.

File: `d/Standard/newbietown/arena_of_tests.c`, line 47:

```c
add_object("/d/Standard/newbietown/obj/chest", 1);
```

Permalink:
```c name=arena_of_tests.c url=https://github.com/bengibollen/sblib/blob/febdb4372dec7d013caebd6125ea41c8a83e3987/d/Standard/newbietown/arena_of_tests.c#L26-L53
void create_room() {
    ...
    add_object("/d/Standard/newbietown/obj/chest", 1);
    ...
}
```

## How it *should* be done
You generally want **repopulation to happen in a reset hook**, *and* you want it to be **idempotent** (i.e., “ensure exactly one chest exists”):

### Option A (best practice): move spawning to a reset function and guard it
Implement a `reset_room()` / `reset()` / whatever your lib calls, and do:

- if no chest present in room → create one
- else → do nothing

This makes resets safe and prevents multiplication.

### Option B: keep it in `create_room()` but guard it anyway
If your driver can call `create_room()` more than once for the same instance (or room gets reloaded), you can still guard:

- scan `all_inventory()` for an object with that file name
- only `add_object` / `clone_object` if missing

### Option C: make the chest “unique” by design
Some libs support “unique” objects or `add_object(..., 1)` *might* already mean “keep exactly one” — but your observed behavior suggests either:
- `add_object` does **not** enforce uniqueness across reloads/resets, or
- the existing chests are no longer in the room when the maintenance runs (e.g., moved into players), so the repopulator keeps making more.

So you still need the room-side guard if you truly want “only one chest in the room”.

## What I need to be precise about the correct hook in *your* mudlib
Different mudlibs name the repopulation hook differently. In your codebase, can you tell me (or let me look up) which one applies to `/std/room`?

Common ones: `reset()`, `reset_room()`, `reset_room(int arg)`, `reset(int arg)`, `init_room()`, etc.

If you want, I can inspect `/std/room` in your repo and tell you exactly:
- which reset function is called by the driver/lib,
- how `add_object()` behaves (does it track intended objects? does it re-add on reset?),
- and give you the correct patch pattern for this project.
