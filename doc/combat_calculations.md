# Combat Calculations Breakdown

A practical guide to understanding how combat values translate into actual damage
and what "sensible" ranges look like for NPCs, weapons, and armour.

---

## Table of Contents

1. [Stats and HP](#stats-and-hp)
2. [Combat Round Flow](#combat-round-flow)
3. [To-Hit Calculation](#to-hit-calculation)
4. [Penetration (F_PENMOD)](#penetration-f_penmod)
5. [Armour Class (F_AC_MOD)](#armour-class-f_ac_mod)
6. [Final Damage](#final-damage)
7. [Unarmed Combat (Creatures)](#unarmed-combat-creatures)
8. [Unarmed Combat (Humanoids)](#unarmed-combat-humanoids)
9. [Weapon Configuration](#weapon-configuration)
10. [Armour Configuration](#armour-configuration)
11. [Combat Speed](#combat-speed)
12. [Experience on Kill](#experience-on-kill)
13. [Fleeing, Following, and Aggro](#fleeing-following-and-aggro)
14. [Reference Tables](#reference-tables)
15. [Worked Examples](#worked-examples)

---

## Stats and HP

### Max HP

```
F_MAX_HP(con) = con < 10 ? con * 10 : con * 20 - 100
```

| CON | Max HP |
|-----|--------|
| 1   | 10     |
| 3   | 30     |
| 5   | 50     |
| 10  | 100    |
| 20  | 300    |
| 30  | 500    |
| 50  | 900    |
| 75  | 1400   |
| 100 | 1900   |

**Key insight:** Below CON 10, HP scales at 10/point. Above 10, it scales at
20/point. A CON of 50 (quite strong NPC) gives 900 HP.

### Stats (set_stats)

Stats are set as `({ str, dex, con, int, wis, dis })`.

- **STR** — Affects unarmed penetration, backstab damage
- **DEX** — Affects to-hit rolls, unarmed hit chance, dodge
- **CON** — Determines max HP, healing rate
- **INT** — Memory, identification
- **WIS** — Mana regeneration
- **DIS** — Discipline; affects panic resistance via `F_PANIC_WIMP_LEVEL(dis) = 10 + 3 * dis`

Average stat (mean of all 6) is used for experience calculations.

---

## Combat Round Flow

Each heart_beat, the combat system:

1. **For each attack** where `random(100) < %use`:
   1. `cb_try_hit()` — Can the attack itself work?
   2. `query_not_attack_me()` — Does the defender prevent being attacked?
   3. **To-Hit Roll** (`cb_tohit`) — Did we connect?
   4. If hit: pick a random damage type from the attack's `dt` flags
   5. **Damage roll** (`cb_hit_me`) — Pick hit location, calculate pen vs AC
   6. Apply damage, generate combat messages

The heart_beat fires every beat but only processes attacks every **3rd beat**,
so the effective attack interval is `speed * 3` seconds (minimum ~6 seconds at
base speed).

---

## To-Hit Calculation

The `cb_tohit()` function determines whether an attack lands. Four factors
contribute, each normalized to a −50 to +50 range:

```
tohit = 4 * fixnorm(attack_roll, defense_roll) + tohit_val

tohit_val = 2 * fixnorm(attacker_dex, defender_dex)
          - (attacker_encumbrance - defender_encumbrance / 4)
```

### fixnorm(offence, defence)

```
fixnorm = (100 * offence / (offence + defence)) - 50
```

Returns a value in [−50, +50]. When offence == defence the result is 0.

### Attack and Defense Rolls

```
attack_roll  = random(wchit) + random(wchit) + random(wchit) + random(wchit)
defense_roll = random(defense_skill)
```

`defense_skill` = SS_PARRY (if wielding a weapon) or
`(200 - encumbrance%) * SS_UNARM_COMBAT / 200` (if not) + SS_DEFENSE.

**The wchit is effectively compared against (parry + defense skill).**
A wchit of 20 against a combined defense of 80 will almost never hit.
A wchit of 50 against defense of 50 hits roughly half the time.

### Blindness / Invisibility

If the attacker cannot see, wchit is scaled by `SS_BLIND_COMBAT / 100`.
If the defender cannot see, their defense is similarly scaled.

### Practical To-Hit Guidelines

| wchit | vs Defense 20 | vs Defense 50 | vs Defense 80 |
|-------|--------------|--------------|--------------|
| 10    | ~50%         | ~20%         | ~10%         |
| 20    | ~65%         | ~40%         | ~20%         |
| 40    | ~80%         | ~60%         | ~40%         |
| 60    | ~85%         | ~70%         | ~55%         |
| 80    | ~90%         | ~80%         | ~65%         |

*(Approximate — varies with DEX and encumbrance differences.)*

---

## Penetration (F_PENMOD)

This formula converts raw `wcpen` and `skill` into an effective modified
penetration value used in the damage roll.

```
F_PENMOD(pen, skill) =
    ((min(pen, skill) + 50) * (blend + 50)) / 30 - 80

where blend = skill > pen ? pen + (skill - pen) / 2 : skill
```

**Intuition:** Skill and pen reinforce each other. Having high pen with low
skill (or vice versa) wastes potential. The formula rewards matching pen to
skill level.

### F_PENMOD Reference Table

| pen | skill=10 | skill=20 | skill=30 | skill=50 | skill=80 | skill=100 |
|-----|----------|----------|----------|----------|----------|-----------|
| 3   | 60       | 63       | 63       | 63       | 63       | 63        |
| 5   | 67       | 72       | 72       | 72       | 72       | 72        |
| 10  | 87       | 97       | 100      | 100      | 100      | 100       |
| 20  | 103      | 137      | 150      | 157      | 157      | 157       |
| 30  | 113      | 157      | 187      | 213      | 220      | 220       |
| 50  | 120      | 170      | 220      | 300      | 363      | 383       |
| 80  | 123      | 177      | 230      | 337      | 473      | 547       |
| 100 | 125      | 178      | 233      | 342      | 500      | 600       |

**Key takeaway:** Increasing pen beyond the skill value gives rapidly
diminishing returns. To get the best use of a high weapon pen, the
corresponding weapon skill must also be high.

---

## Armour Class (F_AC_MOD)

Raw AC values from armour are transformed into effective modified AC values
used in the damage roll:

```
F_AC_MOD(ac) = ((ac + 50) * (ac + 50)) / 50 - 50
```

| Raw AC | Modified AC |
|--------|-------------|
| 0      | 0           |
| 1      | 2           |
| 2      | 4           |
| 5      | 10          |
| 10     | 22          |
| 15     | 34          |
| 20     | 48          |
| 25     | 62          |
| 30     | 78          |
| 40     | 112         |
| 50     | 150         |

**Intuition:** AC scales slightly super-linearly. Higher values become
progressively more effective per point.

---

## Final Damage

When a hit lands, the actual damage is determined as:

```
phit = random(mod_pen/4) + random(mod_pen/4) + random(mod_pen/4) + random(mod_pen/4)
ac   = random(modified_ac_for_hitloc)
dam  = max(0, phit - ac)
```

Where `mod_pen` is the **modified penetration** (`F_PENMOD` output).

**MAGIC_DT** ignores armour entirely:
```
phit = mod_pen/4 + random(mod_pen/4) + random(mod_pen/4) + random(mod_pen/4)
```

**Critical hits** (1 in 10,000 chance) multiply effective pen by 5.

### Average Damage vs Modified AC

The 4×random rolls produce a bell-curved distribution centred around
`mod_pen / 4`.

| Modified Pen | Avg Phit | vs ModAC 0 | vs ModAC 22 | vs ModAC 50 | vs ModAC 78 |
|--------------|----------|------------|-------------|-------------|-------------|
| 50           | ~12      | ~12        | ~5          | ~1          | ~0          |
| 100          | ~25      | ~25        | ~14         | ~8          | ~3          |
| 200          | ~50      | ~50        | ~39         | ~25         | ~14         |
| 300          | ~75      | ~75        | ~64         | ~50         | ~36         |
| 400          | ~100     | ~100       | ~89         | ~75         | ~61         |
| 600          | ~150     | ~150       | ~139        | ~125        | ~111        |

*(Average damage per successful hit. Actual values vary due to randomness.)*

---

## Unarmed Combat (Creatures)

For creatures (inheriting `/std/creature` + `/std/combat/unarmed`), attacks
and hit locations are set directly. The skill used for F_PENMOD is
`SS_UNARM_COMBAT`.

### set_attack_unarmed(aid, wchit, wcpen, dt, puse, desc)

| Argument | Description |
|----------|-------------|
| `aid`    | Arbitrary integer ID — define your own (`#define A_CLAW 0`) |
| `wchit`  | To-hit class — compared vs defender's parry + defense |
| `wcpen`  | Raw penetration — fed into F_PENMOD with SS_UNARM_COMBAT |
| `dt`     | Damage type: `W_IMPALE` (1), `W_SLASH` (2), `W_BLUDGEON` (4), or combinations |
| `puse`   | Percent chance this attack fires each round |
| `desc`   | String used in combat messages ("claw", "tail", etc.) |

The `%use` values across all attacks should sum to roughly **100** for one
attack per round on average. A sum of 200 means two attacks per round.

### set_hitloc_unarmed(hid, ac_array, phit, desc)

| Argument    | Description |
|-------------|-------------|
| `hid`       | Arbitrary integer ID — define your own (`#define H_BODY 0`) |
| `ac_array`  | `({ impale_ac, slash_ac, bludgeon_ac })` — natural armour per damage type |
| `phit`      | Percent chance this location is hit — **must sum to exactly 100** across all hit locations |
| `desc`      | String used in combat messages ("head", "body", "tail") |

### Creature Design Guidelines

| Difficulty | Stats | wchit | wcpen | Natural AC | HP      |
|------------|-------|-------|-------|------------|---------|
| Trivial    | 3–5   | 5–10  | 2–5   | 0–5        | 30–50   |
| Easy       | 8–12  | 15–25 | 5–10  | 5–10       | 60–140  |
| Medium     | 15–25 | 25–40 | 10–20 | 10–20      | 200–400 |
| Hard       | 30–50 | 40–60 | 20–35 | 15–30      | 500–900 |
| Very Hard  | 50–75 | 60–80 | 30–50 | 25–40      | 900–1400|
| Boss       | 75–100| 70–90 | 40–60 | 30–50      | 1400–1900|

**Don't forget:** Set `SS_DEFENCE` skill. Without it a creature is trivially
easy to hit regardless of its other stats. A value of 20–40 is reasonable for
most mobs.

---

## Unarmed Combat (Humanoids)

For humanoids (NPCs/players inheriting `/std/npc` etc.), bare-hands combat
defaults are derived from stats and skills:

```
base_wchit = W_HAND_HIT (10) + F_UNARMED_HIT(uskill, dex)
base_wcpen = W_HAND_PEN (10) + F_UNARMED_PEN(uskill, str)

F_UNARMED_HIT(skill, dex) = skill/7 + dex/20
F_UNARMED_PEN(skill, str) = skill/10 + str/20
```

A humanoid with SS_UNARM_COMBAT=50 and DEX/STR=50:
- wchit = 10 + 50/7 + 50/20 ≈ **19**
- wcpen = 10 + 50/10 + 50/20 ≈ **17**

By default, humanoids have 5 attack slots (right hand, left hand, both hands,
right foot, left foot) with 25% use each. The `%use` is redistributed based on
weapon effectiveness when `set_attackuse()` is set.

---

## Weapon Configuration

### Key Functions

| Function       | Description                                           |
|----------------|-------------------------------------------------------|
| `set_hit(n)`   | To-hit class — see W_MAX_HIT limits per weapon type    |
| `set_pen(n)`   | Raw penetration — see W_MAX_PEN limits per type        |
| `set_dt(dt)`   | Damage type(s): `W_IMPALE`, `W_SLASH`, `W_BLUDGEON`  |
| `set_wt(type)` | Weapon type (`W_KNIFE`, `W_POLEARM`, `W_MISSILE`, …) |
| `set_hands(h)` | `W_ANYH`, `W_RIGHT`, `W_LEFT`, `W_BOTH`              |
| `set_pm(arr)`  | Per-damage-type pen modifier `({imp, sla, blg})` — sum should be 0 |

### Weapon Type Max Values

The arrays `W_MAX_HIT` and `W_MAX_PEN` define the hard cap per weapon type:

```
W_MAX_HIT = ({ 100, 50, 90, 30, 60, 30, 40 })
W_MAX_PEN = ({ 100, 50, 90, 30, 60, 30, 40 })
            idx 0   1   2   3   4   ...
```

Indices match weapon types: 0=assault rifle, 1=psi, 2=missile, 3=knife,
4=polearm.

### Weapon Condition Degradation

Weapons degrade through use. The degradation reduces effective pen:

```
effective_pen[impale] = pen + pm[0] - 2*(dull + corr) / 3
effective_pen[slash]  = pen + pm[1] - (dull + corr)
effective_pen[blug]   = pen + pm[2] - (dull + corr) / 3
```

Slash is most affected, bludgeon the least.
- `set_likely_dull(n)` — likelihood of dulling [0–30 recommended]
- `set_likely_break(n)` — likelihood of breaking [0–20 recommended]

### Sensible Weapon Ranges

| Tier      | hit   | pen   | Example               |
|-----------|-------|-------|-----------------------|
| Poor      | 5–10  | 3–8   | Rusty knife, stick    |
| Basic     | 10–20 | 8–15  | Short sword, club     |
| Good      | 20–35 | 15–25 | Longsword, battleaxe  |
| Excellent | 35–50 | 25–40 | Fine enchanted blade  |
| Legendary | 50–70 | 35–55 | Artifact weapon       |

### Two-Weapon Fighting

When wielding 2 weapons, total `attackuse` = `100 + SS_2H_COMBAT / 2` if
the skill is above 20, otherwise `100 + skill - 20`. Below skill 20, two weapons
is actually *worse* than one.

---

## Armour Configuration

### Key Functions

| Function        | Description                                      |
|-----------------|--------------------------------------------------|
| `set_ac(n)`     | Base armour class                                |
| `set_at(type)`  | Armour type/slot — e.g. `A_BODY`, `A_HEAD`      |
| `set_am(arr)`   | Per-damage-type modifier `({imp, sla, blg})`    |

`set_am` shifts the AC up or down per damage type. The sum is typically 0 to
keep overall AC neutral while making armour better vs one type and worse vs
another. Example: chainmail `({ -2, 5, -3 })` — good vs slash, weaker vs
impale and bludgeon.

### Common Armour Slots (set_at)

| Constant   | Covers            |
|------------|-------------------|
| `A_TORSO`  | Chest + back      |
| `A_HEAD`   | Head              |
| `A_LEGS`   | Legs              |
| `A_R_ARM`  | Right arm         |
| `A_L_ARM`  | Left arm          |
| `A_ROBE`   | Robe (torso area) |
| `A_SHIELD` | Shield (no slot)  |

### Armour Condition

```
effective_ac = arm_ac - condition + repair
```

`set_likely_cond(n)` — how fast condition worsens [0–30 recommended].
`set_likely_break(n)` — how likely armour is to break [0–20 recommended].

### Sensible Armour Ranges

| Tier       | AC    | Description            |
|------------|-------|------------------------|
| Cloth      | 1–3   | Minimal protection     |
| Leather    | 4–8   | Light armour           |
| Chain      | 8–15  | Medium armour          |
| Plate      | 15–25 | Heavy armour           |
| Magical    | 20–35 | Enchanted armour       |
| Legendary  | 30–50 | Artifact armour        |

---

## Combat Speed

```
speed = max(2.0, 5.0 * F_SPEED_MOD(quickness))
F_SPEED_MOD(quickness) = (5.0 - quickness / 50.0) / 5.0
```

Attacks only process every **3rd beat**, so effective attack interval =
`speed * 3` seconds.

| LIVE_I_QUICKNESS | Speed (sec/beat) | Effective Interval |
|------------------|------------------|--------------------|
| 0                | 5.0              | 15.0 sec           |
| 25               | 4.5              | 13.5 sec           |
| 50               | 4.0              | 12.0 sec           |
| 100              | 3.0              | 9.0 sec            |
| 150              | 2.0              | 6.0 sec            |
| 200+             | 2.0 (capped)     | 6.0 sec            |

The default quickness for NPCs is 0, giving ~15 seconds between attacks.
Set `add_prop(LIVE_I_QUICKNESS, n)` to speed a creature up.

---

## Experience on Kill

```
F_EXP_ON_KILL(killer_avg_stat, victim_avg_stat):

  if victim_avg > killer_avg:
      4800 * va^3 / (va + 11*ka) / (va + 50)
  else:
      800  * va^3 / (va + ka)    / (ka + 50)
```

| Victim Avg Stat | Exp (vs equal killer) | Exp (vs stat-10 killer) |
|-----------------|-----------------------|------------------------|
| 5               | ~9                    | ~73                    |
| 10              | ~53                   | ~267                   |
| 20              | ~320                  | ~1143                  |
| 30              | ~900                  | ~2700                  |
| 50              | ~4000                 | ~10000                 |
| 75              | ~12000                | ~30600                 |
| 100             | ~26700                | ~64000                 |

`set_exp_factor(pct)` on an NPC multiplies the base exp by `pct / 100`.
Valid range: 50–200%.

### Team Bonus

```
team_exp_per_member = (100 + team_size * 10) / 100 * base_exp / team_size
```

A team of 3 each gets 130% / 3 ≈ 43% of solo exp. More total exp, less per
person, which rewards grouping without overwhelming scaling.

---

## Fleeing, Following, and Aggro

### Can the Mob Flee? (Whimpy)

Any living can be made to automatically flee when badly hurt:

```lpc
set_whimpy(40);           // flee when below 40% HP
set_whimpy_dir("north");  // try this direction first when fleeing
```

`set_whimpy(0)` disables fleeing (the default). The valid range is 0–99.

When the HP threshold is crossed, `run_away()` is called after a 1-second
delay. The function tries the preferred direction first, then works through
the room's exits randomly. If none succeed, it reports and stays put.

**Prevent a mob from fleeing at all** regardless of panic or whimpy — useful
for guards and boss mobs:

```lpc
add_prop(NPC_I_NO_RUN_AWAY, 1);
```

### Panic and Automatic Fleeing

Even without `set_whimpy`, a mob that accumulates enough panic **may** bolt
on its own (see `cb_may_panic()`). Panic is suppressed by `NPC_I_NO_RUN_AWAY`
and by `NPC_I_NO_FEAR`.

Panic increases when the mob is hit and decreases over time (every 20 sec)
based on DIS. The check to flee fires each attack round:

```
F_PANIC_WIMP_LEVEL(dis) = 10 + 3 * dis
if random(panic) > F_PANIC_WIMP_LEVEL(dis)  ->  mob runs
```

A mob with DIS 1 starts panicking with ~13+ accumulated panic.
A mob with DIS 10 needs ~40+ before it bolts.

To make a mob that **never panics** no matter the circumstance:

```lpc
add_prop(NPC_I_NO_FEAR,    1);   // ignore discipline check; always attacks
add_prop(NPC_I_NO_RUN_AWAY, 1); // never flees even if panic fires
```

`NPC_I_NO_FEAR` also allows the mob to attack targets that are much stronger
than it would normally dare to challenge (see `F_DARE_ATTACK`).

### Does the Mob Follow the Player?

When a player moves out of a room during combat, each enemy in the enemy list
decides independently:

- **If the enemy has `LIVE_O_ENEMY_CLING` set to point at the fleeing player:**
  it is dragged along automatically into the next room. The player cannot
  escape it by simply walking away.

- **Otherwise:** the enemy stays behind, is told "You are now hunting X",
  and the combat link is dropped. *However*, the enemy remains in the mob's
  enemy list. If the mob later enters a room where the player is, it will
  immediately attack again (see `combat_init`).

**To make a mob cling to and follow a specific target:**

```lpc
// In the mob's special_attack() or cr_got_hit() or similar:
add_prop(LIVE_O_ENEMY_CLING, target_object);
```

Setting `LIVE_O_ENEMY_CLING` to point to an enemy object causes that enemy
to be dragged into the next room if they walk away while the mob is in the
same room. The cling is automatically cleared when the target is no longer
the active combatant.

**To make all enemies follow by default**, override `cb_adjust_combat_on_move`
or set the cling property on aggressors when combat starts via a hook.

### How Long Does Aggro Last?

There is no automatic time-based aggro decay for mobs. Once a living is in
a mob's enemy list, it **stays there until one of the following happens**:

1. The enemy dies.
2. `stop_fight(enemy)` is called on the mob.
3. The combat object is destructed (mob resets/destructs).
4. The mob's heart-beat finds no reachable enemies for **30 consecutive
   seconds** and stops; the enemy list is NOT cleared, only the active fight
   ends — the mob will re-engage on next contact.

The enemy list holds up to `MAX_ENEMIES` (10) entries. Oldest entries are
dropped when the list overflows.

### Player "Relaxed from Combat" Timer

For players, the game considers them relaxed from combat (able to quit/linkdie)
once enough time has passed since the last hit:

```
F_RELAX_TIME_AFTER_COMBAT(last_hit_time) = last_hit_time + 60 + 6 * (last_hit_time % 10)
```

This is roughly **60–120 seconds** after the last hit landed (on or by the
player). It does not depend on the mob; the timer is per-player.

### Summary: Behaviour Control Properties

| Property              | Set on   | Effect                                                |
|-----------------------|----------|-------------------------------------------------------|
| `NPC_I_NO_RUN_AWAY`   | mob      | Mob never flees regardless of panic or whimpy         |
| `NPC_I_NO_FEAR`       | mob      | Mob always attacks; ignores the dare-attack check     |
| `LIVE_O_ENEMY_CLING`  | mob      | Mob drags the targeted enemy along when that enemy moves |
| `set_whimpy(pct)`     | any living | Flee automatically when HP drops below `pct`%       |
| `set_whimpy_dir(dir)` | any living | Preferred direction to run in when fleeing          |

### Typical Mob Recipes

**Harmless ambient creature** (runs at first scratch):
```lpc
add_prop(NPC_I_NO_FEAR, 0);   // default — uses discipline check
set_whimpy(80);               // flee at 80% HP
```

**Normal mob** (fights back, runs when badly hurt):
```lpc
set_whimpy(20);               // flee below 20% HP (panic may trigger earlier)
```

**Guard / boss** (never retreats):
```lpc
add_prop(NPC_I_NO_RUN_AWAY, 1);
add_prop(NPC_I_NO_FEAR,     1);
```

**Pursuing mob** (follows the player who attacked it):
```lpc
// In cr_got_hit() or special_attack():
add_prop(LIVE_O_ENEMY_CLING, attacker);
add_prop(NPC_I_NO_RUN_AWAY, 1);
```

---

## Reference Tables

### Damage Types

| Constant   | Value | am[] index | Notes                       |
|------------|-------|------------|-----------------------------|
| W_IMPALE   | 1     | 0          | Piercing; bypasses blunt AC |
| W_SLASH    | 2     | 1          | Cutting; affected by condition most |
| W_BLUDGEON | 4     | 2          | Crushing; least affected by dull |
| MAGIC_DT   | 8     | —          | Ignores all AC              |

Multiple damage types can be combined (e.g. `W_IMPALE | W_SLASH`). Each
combat round, one type is randomly chosen from those set.

### Healing Rates (out of combat)

| Stat    | Interval | Rate formula                               |
|---------|----------|--------------------------------------------|
| HP      | 20 sec   | `(con * 5 + intox * 2 + 100) / 20`        |
| Mana    | 30 sec   | Based on spellcraft + wisdom               |
| Fatigue | 60 sec   | `5 + stuffed * 45 / max_stuffed`           |
| Panic   | 20 sec   | Decays by a DIS-relative percentage        |

HP heal formula example: CON 20, no intox → `(100 + 100) / 20 = 10 HP/interval`.

### Combat-Relevant Skills

| Skill           | Constant      | How it helps in combat                           |
|-----------------|---------------|--------------------------------------------------|
| Defense         | SS_DEFENCE    | Added to all dodge/parry rolls                   |
| Parry           | SS_PARRY      | Chance to parry incoming hits (requires weapon)  |
| Unarmed Combat  | SS_UNARM_COMBAT| Dodge when unarmed; increases unarmed hit/pen   |
| 2H Combat       | SS_2H_COMBAT  | Makes dual-wielding worthwhile above skill 20    |
| Blind Combat    | SS_BLIND_COMBAT| Reduces penalty for fighting blind              |

---

## Worked Examples

### Example 1: The Fluffy Sheep (w/debug/sheep.c)

```lpc
set_stats(({ 3, 3, 3, 1, 1, 2 }));      // str dex con int wis dis
set_skill(SS_DEFENCE, 30);
set_attack_unarmed(A_HEADBUTT, 20, 3, W_BLUDGEON, 30, "headbutt");
set_attack_unarmed(A_KICK,     40, 2, W_BLUDGEON, 40, "kick");
set_attack_unarmed(A_BITE,     40, 2, W_BLUDGEON, 30, "teeth");
set_hitloc_unarmed(H_HEAD, ({ 15, 25, 20, 20 }), 20, "head");
set_hitloc_unarmed(H_BODY, ({ 10, 15, 30, 20 }), 80, "body");
```

| Property       | Value        | Notes                                     |
|----------------|-------------|-------------------------------------------|
| Max HP         | 30          | CON 3 — very fragile                     |
| Defense total  | 30          | Moderate dodge                           |
| Headbutt wchit | 20          | Moderate hit chance                      |
| Headbutt mod_pen | ~63       | F_PENMOD(3, unarm_skill) — negligible    |
| Kick/Bite wchit | 40         | Good hit chance, but wcpen=2 is useless  |
| %use total     | 100         | One attack per round on average          |
| Body AC (blg)  | F_AC_MOD(30) = 78 | Surprisingly tough hide vs bludgeon |

**Verdict:** Sheep has a decent chance to land kicks but deals almost no damage.
The wchit=40 on kick is overkill for a mob with wcpen=2. A wchit of 15–20 would
be more proportionate.

---

### Example 2: A Medium Humanoid Guard

```lpc
set_stats(({ 25, 25, 25, 15, 15, 20 }));
set_skill(SS_DEFENCE, 40);
set_skill(SS_PARRY, 35);
// Wields a longsword: set_hit(25), set_pen(20), set_dt(W_SLASH)
// Wears chain hauberk: set_ac(12), set_at(A_TORSO)
```

| Property        | Value           |
|-----------------|-----------------|
| Max HP          | 400 (CON 25)    |
| Combined defense| 75 (parry+def)  |
| Weapon mod_pen  | ~150 (F_PENMOD(20, weapon_skill)) |
| Avg phit        | ~37             |
| Chain mod_ac    | ~27 (F_AC_MOD(12)) |
| Avg damage/hit  | ~37 − ~13 = ~24 |
| Avg stat        | ~21             |
| Exp on kill     | ~320            |

---

### Example 3: Checklist — Does My NPC Make Sense?

1. **Can it be hit?**
   Its combined defense (SS_PARRY + SS_DEFENCE) should be proportionate to
   the expected attacker wchit. Defense > 80 makes a mob nearly untouchable.

2. **Can it hit back?**
   Its wchit vs the player's typical defense. Players often have 40–70 combined
   defense at mid-levels.

3. **Does it do meaningful damage?**
   Check: `F_PENMOD(wcpen, SS_UNARM_COMBAT)` → divide by 4 for avg phit →
   subtract avg player armour AC (`F_AC_MOD(ac)`).

4. **How durable is it?**
   `max_hp / avg_damage_per_hit`. Account for the 15-sec attack interval.
   A mob that dies in 2 hits is trivial even if those hits looked dangerous.

5. **Is `%use` balanced?**
   Sum should be ~100 for one attack per round. Values above 100 give multiple
   attacks. Keep it proportionate to the mob's threat level.

6. **Does it give appropriate experience?**
   `average_stat = (str+dex+con+int+wis+dis) / 6`. Use the table above to
   check if the exp reward feels right, then adjust with `set_exp_factor()`.
