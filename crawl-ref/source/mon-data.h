#define AT_NO_ATK       {AT_NONE, AF_PLAIN, 0}

#include "enum.h"

/* ******************************************************************

   (see "mon-util.h" for the gory details)

 - ordering does not matter, because seekmonster() searches the entire
   array ... probably not to most efficient thing to do, but so it goes

 - Here are the rows:
    - row 1: monster id, display character, display colour, name
    - row 2: monster flags
    - row 3: monster resistance flags
    - row 4: mass, experience modifier, genus, species, holiness, resist magic
    - row 5: damage for each of four attacks
    - row 6: hit dice, described by four parameters
    - row 7: AC, evasion, sec(spell), corpse_thingy, zombie size, shouts
    - row 8: intel, habitat, flight class, speed, energy_usage
    - row 9: gmon_use class, gmon_eat class, body size, body shape

 - Some further explanations:

    - colour: if COLOUR_UNDEF, a random colour will be chosen upon
              creation. Some monsters set their colour during initialization,
              and if a default colour isn't meaningful, they should also use
              COLOUR_UNDEF.
    - name: if an empty string, name generated automagically (see moname)
    - mass: if zero, the monster never leaves a corpse (also corpse_thingy)
    - genus: base monster "type" for a classed monsters (i.e. jackal as hound)
    - species: corpse type of monster (i.e. orc for orc wizard)
    - holiness:
       MH_HOLY       - irritates some gods when killed, immunity from
                        holy wrath weapons
       MH_NATURAL    - baseline monster type
       MH_UNDEAD     - immunity from draining, pain, torment; resistance
                        to poison; extra damage from holy wrath;
                        affected by holy word
       MH_DEMONIC    - similar to undead, but no poison resistance and
                        *no* automatic hellfire resistance
       MH_NONLIVING  - golems and other constructs
       MH_PLANT      - plants

   exp_mod: see give_adjusted_experience() in mon-death.cc
   - the experience given for killing this monster is calculated something
   like this:

    experience = (16 + maxhp) * HD * HD * exp_mod * (100 + diff. score) * speed
                 / 100000
    with a minimum of 1, and maximum 15000 (jpeg)

   resist_magic: see mons_resist_magic() in mon-util.cc
   - If -x calculate (-x * hit dice * 4/3), else simply x

   damage [4]
   - up to 4 different attacks

   hp_dice [4]
   - hit dice, min hp per HD, extra random hp per HD, fixed HP (unique mons)

    Further explanations copied from mon-util.h:
        hpdice[4]: [0]=HD [1]=min_hp [2]=rand_hp [3]=add_hp
        min hp = [0]*[1] + [3]
        max hp = [0]*([1]+[2]) + [3]
        hp     = [0] *times_do* { [1] + random2(1+[2]) }, *then* + [3]
        example: the Iron Golem, hpdice={15,7,4,0}
           15*7 < hp < 15*(7+4),
           105 < hp < 165
        hp will be around 135 each time.

   sec: if the monster has only one possible spellbook, sec is set to that book.
     If a monster has multiple possible books, sec is set to MST_NO_SPELLS. Then
     the function _mons_spellbook_list in mon-util.cc handles the books.

   corpse_thingy
   - err, bad name. Describes effects of eating corpses.
     CE_NOCORPSE,        leaves no corpse
     CE_CLEAN,           can be healthily eaten by non-Ghouls
     CE_POISONOUS,       inedible to characters without poison resistance
     CE_ROT,             causes rotting in non-Ghouls
     CE_MUTAGEN,         mutagenic

   shouts
   - various things monsters can do upon seeing you

   intel explanation:
   - How smart it is:
   I_PLANT < I_INSECT < I_REPTILE < I_ANIMAL < I_NORMAL < I_HIGH.
   So far, differences here have little effects except for monster's chance
   of seeing you if stealthy and rudimentary trap handling; really stupid
   monsters will walk through clouds.
   I_REPTILE is are lower vertebrates (fish, amphibians, non-draconic reptiles),
   smarter reptiles could be I_ANIMAL.

   speed
   - Increases the store of energy that the monster uses for doing things.
   less = slower. 5 = half speed, 10 = normal, 20 = double speed.

   energy usage
   - How quickly the energy granted by speed is used up.  Most monsters
   should just use DEFAULT_ENERGY, where all the different types of actions
   use 10 energy units.

   gmon_use explanation:
     MONUSE_NOTHING,
     MONUSE_OPEN_DOORS,
     MONUSE_STARTING_EQUIPMENT,
     MONUSE_WEAPONS_ARMOUR

    From MONUSE_STARTING_EQUIPMENT on, monsters are capable of handling
    items.  Contrary to what one might expect, MONUSE_WEAPONS_ARMOUR
    also means a monster is capable of using wands and will also pick
    them up, something that those with MONUSE_STARTING_EQUIPMENT won't
    do.

   gmon_eat explanation:
     MONEAT_ITEMS,
     MONEAT_CORPSES,
     MONEAT_DOORS,

    Monsters with MONEAT_ITEMS are capable of eating most items,
    and monsters with MONEAT_CORPSES are capable of eating corpses.
    Monsters with MONEAT_DOORS will destroy doors instead of opening them.

   size:
     SIZE_TINY,              // rats/bats
     SIZE_LITTLE,            // spriggans
     SIZE_SMALL,             // halflings/kobolds
     SIZE_MEDIUM,            // humans/elves/dwarves
     SIZE_LARGE,             // trolls/ogres/centaurs/nagas
     SIZE_BIG,               // large quadrupeds
     SIZE_GIANT,             // giants

*/

#define MOVE_ENERGY(x)     { x,  x, 10, 10, 10, 10, 10, 100}
#define ACTION_ENERGY(x)   {10, 10,  x,  x,  x,  x,  x, x * 10}
#define ATTACK_ENERGY(x)   {10, 10,  x, 10, 10, 10, 10, 100}
#define MISSILE_ENERGY(x)  {10, 10, 10,  x, 10, 10, 10, 100}
#define SPELL_ENERGY(x)    {10, 10, 10, 10,  x, 10, 10, 100}
#define SWIM_ENERGY(x)     {10,  x, 10, 10, 10, 10, 10, 100}

static monsterentry mondata[] =
{

// The Thing That Should Not Be(tm)
// NOTE: Do not remove, or seekmonster will crash on unknown mc request!
// It is also a good prototype for new monsters.
{
    // id, glyph, colour, name
    MONS_PROGRAM_BUG, 'B', LIGHTRED, "program bug",
    // monster flags
    M_NO_EXP_GAIN | M_CANT_SPAWN,
    // resistance flags
    MR_NO_FLAGS,
    // mass, xp modifier, genus, species, holiness, magic resistance
    0, 10, MONS_PROGRAM_BUG, MONS_PROGRAM_BUG, MH_NATURAL, 10,
    // up to four attacks
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    // hit points
    { 0, 0, 0, 0 },
    // AC, EV, spells, corpse type, zombie size, shout type, intelligence
    0, 0, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    // intelligence, habitat, speed, energy usage, use type
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    // use type, eat type, body size, body shape
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC,
},

// Use this to replace removed monsters, to retain save compatibility.
// Please put it in #if TAG_MAJOR_VERSION == X, so they will go away
// after save compat is broken.
#define AXED_MON(id) \
{ \
    id, 'X', LIGHTRED, "removed "#id, \
    M_NO_EXP_GAIN | M_CANT_SPAWN | M_UNFINISHED, \
    MR_NO_FLAGS, \
    0, 10, MONS_PROGRAM_BUG, MONS_PROGRAM_BUG, MH_NONLIVING, 0, \
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK }, \
    { 0, 0, 0, 0 }, \
    0, 0, MST_NO_SPELLS, CE_CLEAN, S_SILENT, \
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY, \
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC \
},

// Axed monsters.
// AXED_MON(MONS_MOTHER_IN_LAW)
#if TAG_MAJOR_VERSION == 34
    AXED_MON(MONS_BUMBLEBEE)
    AXED_MON(MONS_WOOD_GOLEM)
    AXED_MON(MONS_ANT_LARVA)
    AXED_MON(MONS_LABORATORY_RAT)
    AXED_MON(MONS_WAR_DOG)
    AXED_MON(MONS_SPIRIT)
    AXED_MON(MONS_PALADIN)
    AXED_MON(MONS_DEEP_ELF_SOLDIER)
    AXED_MON(MONS_PAN)
    AXED_MON(MONS_LAMIA)
    AXED_MON(MONS_DEEP_DWARF_SCION)
    AXED_MON(MONS_DEEP_DWARF_ARTIFICER)
    AXED_MON(MONS_DEEP_DWARF_NECROMANCER)
    AXED_MON(MONS_CHAOS_BUTTERFLY)
    AXED_MON(MONS_POLYMOTH)
    AXED_MON(MONS_MOTH_OF_SUPPRESSION)
    AXED_MON(MONS_ROCK_WORM)
    AXED_MON(MONS_FORMICID_DRONE)
    AXED_MON(MONS_SPIRIT_WOLF)
    AXED_MON(MONS_LAVA_FISH)
    AXED_MON(MONS_LEMURE)
    AXED_MON(MONS_JELLYFISH)
    AXED_MON(MONS_GREY_RAT)
    AXED_MON(MONS_SPINY_WORM)
    AXED_MON(MONS_ROCK_TROLL)
    AXED_MON(MONS_MONSTROUS_ITEM_MIMIC)
    AXED_MON(MONS_GIANT_AMOEBA)
    AXED_MON(MONS_DEEP_DWARF_BERSERKER)
    AXED_MON(MONS_AGATE_SNAIL)
    AXED_MON(MONS_GIANT_CENTIPEDE)
    AXED_MON(MONS_ANCIENT_BEAR)
    AXED_MON(MONS_ROTTING_DEVIL)
    AXED_MON(MONS_IGNIS)
    AXED_MON(MONS_DJINNI)
    AXED_MON(MONS_RAKSHASA_FAKE)
    AXED_MON(MONS_MARA_FAKE)
    // GNOME_NO_MORE
    AXED_MON(MONS_GNOME)
    // GOLEM_NO_MORE
    AXED_MON(MONS_CLAY_GOLEM)
    AXED_MON(MONS_STONE_GOLEM)
    AXED_MON(MONS_FORMICID_VENOM_MAGE)
    AXED_MON(MONS_SPRIGGAN_ASSASSIN)
    AXED_MON(MONS_VAPOUR)
    AXED_MON(MONS_THORN_LOTUS)
    AXED_MON(MONS_GIANT_GOLDFISH)
    AXED_MON(MONS_SILVER_STAR)
    AXED_MON(MONS_FLAMING_CORPSE)
    AXED_MON(MONS_GRIZZLY_BEAR)
    AXED_MON(MONS_SPRIGGAN_ENCHANTER)
    AXED_MON(MONS_PHOENIX)
    AXED_MON(MONS_SHEDU)
    AXED_MON(MONS_PLAGUE_SHAMBLER)
    AXED_MON(MONS_GIANT_SLUG)
    AXED_MON(MONS_FIREFLY)
    AXED_MON(MONS_BROWN_OOZE)
    AXED_MON(MONS_PULSATING_LUMP)
    AXED_MON(MONS_BIG_FISH)
    AXED_MON(MONS_LAVA_WORM)
    AXED_MON(MONS_SHARK)
    AXED_MON(MONS_INEPT_ITEM_MIMIC)
    AXED_MON(MONS_ITEM_MIMIC)
    AXED_MON(MONS_RAVENOUS_ITEM_MIMIC)
    AXED_MON(MONS_INEPT_FEATURE_MIMIC)
    AXED_MON(MONS_FEATURE_MIMIC)
    AXED_MON(MONS_RAVENOUS_FEATURE_MIMIC)
    AXED_MON(MONS_MNOLEG_TENTACLE)
    AXED_MON(MONS_MNOLEG_TENTACLE_SEGMENT)
#endif

// Used for genus monsters (which are used for grouping monsters by how they
// work and in comes-into-view messages.
#define DUMMY(id, glyph, colour, name) \
{ \
    (id), (glyph), (colour), (name), \
    M_CANT_SPAWN, \
    MR_NO_FLAGS, \
    0, 10, (id), (id), MH_NONLIVING, 10, \
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK }, \
    { 0, 0, 0, 0 }, \
    0, 0, MST_NO_SPELLS, CE_CLEAN, S_SILENT, \
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY, \
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC \
},


// Real monsters begin here {dlb}:

// ants and formicids ('a')
{
    MONS_WORKER_ANT, 'a', RED, "worker ant",
    M_NO_SKELETON,
    MR_VUL_POISON,
    450, 10, MONS_WORKER_ANT, MONS_WORKER_ANT, MH_NATURAL, 10,
    { {AT_BITE, AF_POISON, 8}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    4, 10, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_INSECT
},

{
    MONS_QUEEN_ANT, 'a', LIGHTRED, "queen ant",
    M_NO_SKELETON,
    MR_VUL_POISON,
    900, 10, MONS_WORKER_ANT, MONS_QUEEN_ANT, MH_NATURAL, 60,
    { {AT_STING, AF_POISON_STRONG, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 3, 5, 0 },
    14, 3, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_INSECT
},

{
    MONS_SOLDIER_ANT, 'a', WHITE, "soldier ant",
    M_NO_SKELETON,
    MR_VUL_POISON,
    600, 10, MONS_WORKER_ANT, MONS_SOLDIER_ANT, MH_NATURAL, 20,
    { {AT_STING, AF_POISON, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    8, 10, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_INSECT
},

// dummy for recolouring
{
    MONS_FORMICID, 'a', GREEN, "formicid",
    M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_NO_SKELETON | M_BURROWS
        | M_NO_POLY_TO,
    MR_NO_FLAGS,
    600, 10, MONS_FORMICID, MONS_FORMICID, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 4, 6, 0 },
    3, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// batty monsters ('b')
{
    MONS_BAT, 'b', LIGHTGREY, "bat",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BATTY,
    MR_NO_FLAGS,
    150, 4, MONS_BAT, MONS_BAT, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 1}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 3, 0 },
    1, 14, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT
},

{
    MONS_BUTTERFLY, 'b', ETC_JEWEL, "butterfly",
    M_CONFUSED | M_NO_EXP_GAIN,
    MR_VUL_POISON,
    150, 10, MONS_BUTTERFLY, MONS_BUTTERFLY, MH_NATURAL, 10,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 1 },
    0, 25, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_INSECT, HT_LAND, FL_WINGED, 25, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_INSECT_WINGED
},

{ // one vault + player transform (Vp ability)
    MONS_VAMPIRE_BAT, 'b', MAGENTA, "vampire bat",
    M_SEE_INVIS | M_WARM_BLOOD | M_BATTY | M_NO_POLY_TO,
    MR_NO_FLAGS,
    150, 8, MONS_BAT, MONS_VAMPIRE_BAT, MH_UNDEAD, 10,
    { {AT_BITE, AF_VAMPIRIC, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 2, 3, 0 },
    1, 14, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT
},

{
    MONS_FIRE_BAT, 'b', LIGHTRED, "fire bat",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BATTY,
    MR_RES_HELLFIRE | MR_VUL_COLD | MR_VUL_WATER,
    150, 8, MONS_BAT, MONS_FIRE_BAT, MH_NATURAL, 10,
    { {AT_BITE, AF_FIRE, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 3, 0 },
    1, 14, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT
},

{
    MONS_RAVEN, 'b', BLUE, "raven",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD,
    MR_NO_FLAGS,
    250, 9, MONS_RAVEN, MONS_RAVEN, MH_NATURAL, 20,
    { {AT_PECK, AF_PLAIN, 14}, {AT_CLAW, AF_PLAIN, 11}, AT_NO_ATK, AT_NO_ATK },
    { 6, 4, 3, 0 },
    1, 10, MST_NO_SPELLS, CE_CLEAN, S_CAW,
    I_ANIMAL, HT_LAND, FL_WINGED, 20, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT // ...
},

{
    MONS_BENNU, 'b', YELLOW, "bennu",
    M_SEE_INVIS | M_WARM_BLOOD | M_FAST_REGEN | M_GLOWS_LIGHT |
        M_NO_POLY_TO | M_SPEAKS,
    mrd(MR_RES_FIRE | MR_RES_NEG, 3) | MR_RES_POISON |
        MR_RES_ROTTING | MR_VUL_WATER,
    550, 6, MONS_BENNU, MONS_BENNU, MH_NATURAL, 140,
    { {AT_PECK, AF_HOLY, 27}, {AT_CLAW, AF_DRAIN_XP, 18},
      {AT_CLAW, AF_STICKY_FLAME, 18}, AT_NO_ATK },
    { 14, 3, 5, 0 },
    6, 16, MST_NO_SPELLS, CE_NOCORPSE, S_SCREECH,
    I_NORMAL, HT_LAND, FL_WINGED, 16, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_BAT
},

{
    MONS_CAUSTIC_SHRIKE, 'b', LIGHTGREEN, "caustic shrike",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD,
    MR_RES_COLD | MR_RES_FIRE,
    150, 15, MONS_CAUSTIC_SHRIKE, MONS_CAUSTIC_SHRIKE, MH_NATURAL, 80,
    { {AT_CLAW, AF_ACID, 36}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 5, 0 },
    10, 18, MST_NO_SPELLS, CE_CLEAN, S_SCREECH,
    I_ANIMAL, HT_LAND, FL_WINGED, 20, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT
},

{
    MONS_SHARD_SHRIKE, 'b', LIGHTBLUE, "shard shrike",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BATTY,
    mrd(MR_RES_COLD, 2),
    150, 12, MONS_SHARD_SHRIKE, MONS_SHARD_SHRIKE, MH_NATURAL, 80,
    { {AT_CLAW, AF_COLD, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 21, 3, 4, 0 },
    2, 18, MST_SHARD_SHRIKE, CE_CLEAN, S_CAW,
    I_ANIMAL, HT_LAND, FL_WINGED, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_BAT
},

// centaurs ('c')
{
    MONS_CENTAUR, 'c', BROWN, "centaur",
    M_WARM_BLOOD | M_ARCHER | M_SPEAKS,
    MR_NO_FLAGS,
    1500, 10, MONS_CENTAUR, MONS_CENTAUR, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 5, 0 },
    3, 7, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_CENTAUR
},

{
    MONS_CENTAUR_WARRIOR, 'c', YELLOW, "centaur warrior",
    M_WARM_BLOOD | M_FIGHTER | M_ARCHER | M_SPEAKS,
    MR_NO_FLAGS,
    1500, 12, MONS_CENTAUR, MONS_CENTAUR, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 16}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    4, 8, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_CENTAUR
},

{
    MONS_YAKTAUR, 'c', RED, "yaktaur",
    M_WARM_BLOOD | M_ARCHER | M_SPEAKS,
    MR_NO_FLAGS,
    1900, 9, MONS_YAKTAUR, MONS_YAKTAUR, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    4, 4, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_CENTAUR
},

{
    MONS_YAKTAUR_CAPTAIN, 'c', LIGHTRED, "yaktaur captain",
    M_WARM_BLOOD | M_FIGHTER | M_ARCHER | M_SPEAKS,
    MR_NO_FLAGS,
    1900, 9, MONS_YAKTAUR, MONS_YAKTAUR, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    5, 5, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_CENTAUR
},

{
    MONS_FAUN, 'c', GREEN, "faun",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 8, MONS_FAUN, MONS_FAUN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 4, 0 },
    2, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_SATYR, 'c', LIGHTGREEN, "satyr",
    M_WARM_BLOOD | M_SPEAKS | M_ARCHER | M_DONT_MELEE,
    MR_NO_FLAGS,
    550, 10, MONS_SATYR, MONS_SATYR, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 5, 0 },
    2, 12, MST_SATYR, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

// draconians ('d')
{   // Base draconian - for use like MONS_HUMAN, MONS_ELF although we
    // now store the draconian subspecies in base_monster for those
    // listed as species MONS_DRACONIAN.
    MONS_DRACONIAN, 'd', BROWN, "draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 6, 4, 0 },
    10, 11, MST_NO_SPELLS, CE_CLEAN, S_ROAR, I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_BLACK_DRACONIAN, 'd', BLUE, "black draconian",
    M_COLD_BLOOD | M_SPEAKS,
    mrd(MR_RES_ELEC, 2),
    900, 10, MONS_DRACONIAN, MONS_BLACK_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED_TAILED
},

{
    MONS_YELLOW_DRACONIAN, 'd', YELLOW, "yellow draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_ACID,
    900, 10, MONS_DRACONIAN, MONS_YELLOW_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    // Colours are used for picking the right tile for Tiamat,
    // so this needs to be different from the grey draconian. (jpeg)
    MONS_PALE_DRACONIAN, 'd', CYAN, "pale draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_STEAM,
    900, 10, MONS_DRACONIAN, MONS_PALE_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 14, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_GREEN_DRACONIAN, 'd', GREEN, "green draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    900, 10, MONS_DRACONIAN, MONS_GREEN_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, {AT_TAIL_SLAP, AF_POISON, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_POISONOUS, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_PURPLE_DRACONIAN, 'd', MAGENTA, "purple draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_PURPLE_DRACONIAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    8, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_RED_DRACONIAN, 'd', LIGHTRED, "red draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_FIRE,
    900, 10, MONS_DRACONIAN, MONS_RED_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_WHITE_DRACONIAN, 'd', WHITE, "white draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_COLD,
    900, 10, MONS_DRACONIAN, MONS_WHITE_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_GREY_DRACONIAN, 'd', LIGHTGREY, "grey draconian",
    M_COLD_BLOOD | M_UNBREATHING | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_GREY_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, {AT_TAIL_SLAP, AF_PLAIN, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 14, 5, 4, 0 },
    16, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_MOTTLED_DRACONIAN, 'd', LIGHTMAGENTA, "mottled draconian",
    M_COLD_BLOOD | M_SPEAKS,
    MR_RES_FIRE | MR_RES_STICKY_FLAME,
    900, 10, MONS_DRACONIAN, MONS_MOTTLED_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 5, 4, 0 },
    9, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

// nonbase draconians ('q')
// Nonbase draconian AC and EV is additive with the base type.
{
    MONS_DRACONIAN_CALLER, 'q', BROWN, "draconian caller",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 3, 0 },
    0, 0, MST_DRAC_CALLER, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_MONK, 'q', BLUE, "draconian monk",
    M_FIGHTER | M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 35}, {AT_KICK, AF_PLAIN, 20},
      {AT_TAIL_SLAP, AF_PLAIN, 15}, AT_NO_ATK },
    { 16, 6, 3, 0 },
    -3, 10, MST_NO_SPELLS, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_ZEALOT, 'q', LIGHTGREEN, "draconian zealot",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 2, 0 },
    3, 0, MST_DRACONIAN_ZEALOT, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_SHIFTER, 'q', LIGHTCYAN, "draconian shifter",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 4, 0 },
    -1, 6, MST_DRAC_SHIFTER, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_ANNIHILATOR, 'q', LIGHTBLUE, "draconian annihilator",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 2, 0 },
    -1, 0, MST_DRACONIAN_ANNIHILATOR, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_KNIGHT, 'q', CYAN, "draconian knight",
    M_FIGHTER | M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 6, 2, 0 },
    3, 2, MST_DRACONIAN_KNIGHT, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_DRACONIAN_SCORCHER, 'q', LIGHTRED, "draconian scorcher",
    M_COLD_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 2, 0 },
    -1, 2, MST_DRAC_SCORCHER, CE_CLEAN, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_TAILED
},

// elves ('e')
//mv: have to exist because it's (and should be) a valid polymorph target.
{
    MONS_ELF, 'e', LIGHTRED, "elf",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 3, 0 },
    3, 15, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_FIGHTER, 'e', LIGHTRED, "deep elf fighter",
    M_WARM_BLOOD | M_FIGHTER | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 3, 0 },
    0, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_KNIGHT, 'e', CYAN, "deep elf knight",
    M_WARM_BLOOD | M_FIGHTER | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 3, 0 },
    0, 15, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_BLADEMASTER, 'e', LIGHTCYAN, "deep elf blademaster",
    M_WARM_BLOOD | M_FIGHTER | M_TWO_WEAPONS | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 25}, {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK },
    { 16, 5, 3, 0 },
    0, 25, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_MASTER_ARCHER, 'e', LIGHTGREY, "deep elf master archer",
    M_WARM_BLOOD | M_ARCHER | M_DONT_MELEE | M_SPEAKS,
    MR_NO_FLAGS,
    450, 30, MONS_ELF, MONS_ELF, MH_NATURAL, 100,
    // Attack damage gets rolled into their ranged attacks.
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 2, 0 },
    0, 15, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, MISSILE_ENERGY(5),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_MAGE, 'e', MAGENTA, "deep elf mage",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    450, 12, MONS_ELF, MONS_ELF, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 3, 0 },
    0, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_SUMMONER, 'e', BROWN, "deep elf summoner",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    450, 13, MONS_ELF, MONS_ELF, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 4, 3, 0 },
    0, 13, MST_DEEP_ELF_SUMMONER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_CONJURER, 'e', BLUE, "deep elf conjurer",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    450, 12, MONS_ELF, MONS_ELF, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_CONJURER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_PRIEST, 'e', GREEN, "deep elf priest",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    450, 11, MONS_ELF, MONS_ELF, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_PRIEST, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_HIGH_PRIEST, 'e', LIGHTGREEN, "deep elf high priest",
    M_SPEAKS | M_WARM_BLOOD | M_SEE_INVIS,
    MR_NO_FLAGS,
    450, 15, MONS_ELF, MONS_ELF, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 3, 0 },
    3, 13, MST_DEEP_ELF_HIGH_PRIEST, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_DEMONOLOGIST, 'e', YELLOW, "deep elf demonologist",
    M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    450, 20, MONS_ELF, MONS_ELF, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_DEMONOLOGIST, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_ANNIHILATOR, 'e', LIGHTBLUE, "deep elf annihilator",
    M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_ANNIHILATOR, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_SORCERER, 'e', LIGHTMAGENTA, "deep elf sorcerer",
    M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    450, 17, MONS_ELF, MONS_ELF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_SORCERER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_ELF_DEATH_MAGE, 'e', WHITE, "deep elf death mage",
    M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 3, 3, 0 },
    0, 13, MST_DEEP_ELF_DEATH_MAGE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// Mobile plants and fungi ('f')
{
    MONS_WANDERING_MUSHROOM, 'f', BROWN, "wandering mushroom",
    M_NO_FLAGS,
    MR_RES_POISON,
    150, 10, MONS_FUNGUS, MONS_WANDERING_MUSHROOM, MH_PLANT, 40,
    { {AT_SPORE, AF_CONFUSE, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    5, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_DEATHCAP, 'f', LIGHTMAGENTA, "deathcap",
    M_NO_FLAGS,
    MR_RES_COLD,
    150, 6, MONS_FUNGUS, MONS_WANDERING_MUSHROOM, MH_UNDEAD, 80,
    { {AT_SPORE, AF_CONFUSE, 33}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 2, 4, 0 },
    5, 0, MST_SOUL_EATER, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_THORN_HUNTER, 'f', WHITE, "thorn hunter",
    M_SEE_INVIS,
    MR_RES_POISON | MR_VUL_FIRE,
    1300, 14, MONS_PLANT, MONS_THORN_HUNTER, MH_PLANT, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 27}, {AT_HIT, AF_PLAIN, 23}, AT_NO_ATK,
       AT_NO_ATK },
    { 15, 4, 5, 0 },
    9, 9, MST_THORN_HUNTER, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_NONE, 12, SWIM_ENERGY(12),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_PLANT
},

{
    MONS_SHAMBLING_MANGROVE, 'f', LIGHTRED, "shambling mangrove",
    M_NO_FLAGS,
    MR_RES_POISON,
    1500, 12, MONS_SHAMBLING_MANGROVE, MONS_SHAMBLING_MANGROVE, MH_PLANT, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 41}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 5, 4, 0 },
    13, 3, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_PLANT
},

{
    MONS_VINE_STALKER, 'f', GREEN, "vine stalker",
    M_SPEAKS | M_NO_POLY_TO | M_FAST_REGEN,
    MR_NO_FLAGS,
    550, 10, MONS_VINE_STALKER, MONS_VINE_STALKER, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 10}, {AT_BITE, AF_ANTIMAGIC, 10}, AT_NO_ATK,
       AT_NO_ATK },
    { 8, 3, 3, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// goblins and other small humanoids ('g')
{
    MONS_GOBLIN, 'g', LIGHTGREY, "goblin",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    400, 10, MONS_GOBLIN, MONS_GOBLIN, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 4, 0 },
    0, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_HOBGOBLIN, 'g', BROWN, "hobgoblin",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    500, 10, MONS_GOBLIN, MONS_HOBGOBLIN, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 4, 3, 0 },
    2, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GNOLL, 'g', YELLOW, "gnoll",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    680, 10, MONS_GNOLL, MONS_GNOLL, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 4, 5, 0 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GNOLL_SHAMAN, 'g', WHITE, "gnoll shaman",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    680, 10, MONS_GNOLL, MONS_GNOLL, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 4, 5, 0 },
    2, 9, MST_GNOLL_SHAMAN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GNOLL_SERGEANT, 'g', CYAN, "gnoll sergeant",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    680, 10, MONS_GNOLL, MONS_GNOLL, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 11}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 4, 5, 0 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_BOGGART, 'g', MAGENTA, "boggart",
    M_SEE_INVIS | M_WARM_BLOOD,
    MR_NO_FLAGS,
    300, 14, MONS_BOGGART, MONS_BOGGART, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 4, 0 },
    0, 12, MST_BOGGART, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{ // Another dummy monster.  Zombies and poly allowed.
    MONS_DWARF, 'g', GREEN, "dwarf",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_DWARF, MONS_DWARF, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_DWARF, 'g', LIGHTBLUE, "deep dwarf",
    M_WARM_BLOOD | M_SPEAKS | M_NO_REGEN,
    MR_NO_FLAGS,
    600, 10, MONS_DWARF, MONS_DEEP_DWARF, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// carnivorous quadrupeds ('h')
{
    MONS_JACKAL, 'h', BROWN, "jackal",
    M_WARM_BLOOD | M_BLOOD_SCENT,
    MR_NO_FLAGS,
    360, 10, MONS_HOUND, MONS_JACKAL, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 3, 5, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_HOUND, 'h', YELLOW, "hound",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BLOOD_SCENT,
    MR_NO_FLAGS,
    300, 10, MONS_HOUND, MONS_HOUND, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    2, 13, MST_NO_SPELLS, CE_CLEAN, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_WARG, 'h', WHITE, "warg",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BLOOD_SCENT,
    MR_RES_POISON,
    750, 12, MONS_HOUND, MONS_WARG, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 4, 5, 0 },
    4, 12, MST_NO_SPELLS, CE_CLEAN, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_WOLF, 'h', LIGHTGREY, "wolf",
    M_SEE_INVIS | M_UNBLINDABLE | M_WARM_BLOOD | M_BLOOD_SCENT,
    MR_NO_FLAGS,
    450, 19, MONS_HOUND, MONS_WOLF, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 5, 0 },
    4, 15, MST_NO_SPELLS, CE_CLEAN, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 17, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED
},

{
    MONS_HOG, 'h', LIGHTMAGENTA, "hog",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    450, 10, MONS_HOG, MONS_HOG, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_HELL_HOUND, 'h', CYAN, "hell hound",
    M_SEE_INVIS | M_UNBLINDABLE | M_BLOOD_SCENT,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD,
    450, 10, MONS_HOUND, MONS_HELL_HOUND, MH_DEMONIC, 20,
    { {AT_BITE, AF_PLAIN, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    6, 13, MST_HELL_HOUND, CE_POISONOUS, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED
},

{
    MONS_RAIJU, 'h', LIGHTCYAN, "raiju",
    M_SEE_INVIS | M_UNBLINDABLE | M_BLOOD_SCENT,
    mrd(MR_RES_ELEC, 3),
    450, 12, MONS_HOUND, MONS_RAIJU, MH_NATURAL, 20,
    { {AT_BITE, AF_ELEC, 11}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    4, 14, MST_RAIJU, CE_CLEAN, S_BARK,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED
},

{
    MONS_HELL_HOG, 'h', LIGHTRED, "hell hog",
    M_NO_FLAGS,
    MR_NO_FLAGS,
    450, 10, MONS_HOG, MONS_HELL_HOG, MH_DEMONIC, 40,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    2, 9, MST_HELL_HOG, CE_POISONOUS, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED
},

{   // effect of porkalator cast on holies
    MONS_HOLY_SWINE, 'h', YELLOW, "holy swine",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    450, 10, MONS_HOG, MONS_HOLY_SWINE, MH_HOLY, 40,
    { {AT_BITE, AF_HOLY, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{ // a dummy monster for recolouring
    MONS_FELID, 'h', MAGENTA, "felid",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    200, 10, MONS_FELID, MONS_FELID, MH_NATURAL, 40,
    { {AT_CLAW, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 2, 3, 0 },
    2, 18, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(8),
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

DUMMY(MONS_BEAR, 'h', LIGHTGREY, "bear")

{
    MONS_POLAR_BEAR, 'h', LIGHTBLUE, "polar bear",
    M_WARM_BLOOD | M_FLEES,
    MR_RES_COLD,
    1200, 12, MONS_BEAR, MONS_POLAR_BEAR, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 5}, {AT_CLAW, AF_PLAIN, 5},
       AT_NO_ATK },
    { 7, 5, 3, 0 },
    7, 8, MST_BERSERK_ESCAPE, CE_CLEAN, S_GROWL,
    I_ANIMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_TAILLESS
},

{
    MONS_BLACK_BEAR, 'h', BLUE, "black bear",
    M_WARM_BLOOD | M_FLEES,
    MR_NO_FLAGS,
    800, 9, MONS_BEAR, MONS_BLACK_BEAR, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 9}, {AT_CLAW, AF_PLAIN, 5}, {AT_CLAW, AF_PLAIN, 5},
       AT_NO_ATK },
    { 6, 3, 3, 0 },
    2, 8, MST_BERSERK_ESCAPE, CE_CLEAN, S_GROWL,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

// spriggans ('i')
{
    MONS_SPRIGGAN, 'i', LIGHTGREY, "spriggan",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS,
    MR_NO_FLAGS,
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 2, 3, 0 },
    1, 18, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{   // both the guy and his ride as one monster
    MONS_SPRIGGAN_RIDER, 'i', LIGHTBLUE, "spriggan rider",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_FIGHTER,
    MR_VUL_POISON, // the mount
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 4, 3, 0 },
    1, 18, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_WINGED, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID // ??
},

{
    MONS_SPRIGGAN_DRUID, 'i', GREEN, "spriggan druid",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS,
    MR_RES_POISON,
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 2, 0 },
    1, 18, MST_SPRIGGAN_DRUID, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    MONS_SPRIGGAN_BERSERKER, 'i', LIGHTRED, "spriggan berserker",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_FIGHTER,
    MR_NO_FLAGS,
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 3, 0 },
    2, 18, MST_BK_TROG, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    MONS_SPRIGGAN_AIR_MAGE, 'i', LIGHTCYAN, "spriggan air mage",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS,
    mrd(MR_RES_ELEC, 2),
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 16}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 2, 3, 0 },
    1, 22, MST_SPRIGGAN_AIR_MAGE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    MONS_SPRIGGAN_DEFENDER, 'i', YELLOW, "spriggan defender",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_FIGHTER,
    MR_NO_FLAGS,
    200, 10, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 3, 3, 0 },
    3, 22, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 16, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

// drakes ('k')
DUMMY(MONS_DRAKE, 'k', LIGHTGREY, "drake")

{
    MONS_SWAMP_DRAKE, 'k', BROWN, "swamp drake",
    M_WARM_BLOOD,
    MR_RES_POISON,
    900, 20, MONS_DRAKE, MONS_SWAMP_DRAKE, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 5, 5, 0 },
    3, 11, MST_SWAMP_DRAKE, CE_POISONOUS, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_FIRE_DRAKE, 'k', LIGHTRED, "fire drake",
    M_WARM_BLOOD,
    MR_RES_FIRE,
    1000, 20, MONS_DRAKE, MONS_FIRE_DRAKE, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 8}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    3, 12, MST_FIRE_BREATH, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_WIND_DRAKE, 'k', WHITE, "wind drake",
    M_WARM_BLOOD,
    MR_RES_WIND,
    1000, 6, MONS_DRAKE, MONS_WIND_DRAKE, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 5, 5, 0 },
    3, 12, MST_WIND_DRAKE, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_LINDWURM, 'k', LIGHTGREEN, "lindwurm",
    M_WARM_BLOOD | M_GLOWS_LIGHT,
    MR_NO_FLAGS,
    950, 13, MONS_DRAKE, MONS_LINDWURM, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 10},
      {AT_CLAW, AF_PLAIN, 10}, AT_NO_ATK },
    { 9, 3, 5, 0 },
    8, 6, MST_FIRE_BREATH, CE_CLEAN, S_ROAR,
    I_REPTILE, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_DEATH_DRAKE, 'k', LIGHTGREY, "death drake",
    M_COLD_BLOOD,
    MR_RES_POISON | MR_RES_ROTTING,
    900, 10, MONS_DRAKE, MONS_DEATH_DRAKE, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 5, 7, 0 },
    6, 14, MST_DEATH_DRAKE, CE_ROT, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

// lizards ('l')
DUMMY(MONS_GIANT_LIZARD, 'l', LIGHTGREY, "giant lizard")

{
    MONS_GIANT_NEWT, 'l', GREEN, "giant newt",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    170, 10, MONS_GIANT_LIZARD, MONS_GIANT_NEWT, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 2, 0 },
    0, 15, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_QUADRUPED
},

{
    MONS_GIANT_GECKO, 'l', YELLOW, "giant gecko",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    250, 16, MONS_GIANT_LIZARD, MONS_GIANT_GECKO, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 3, 5, 0 },
    1, 14, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

{
    MONS_IGUANA, 'l', BLUE, "iguana",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    400, 13, MONS_GIANT_LIZARD, MONS_IGUANA, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    5, 9, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_REPTILE, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_BASILISK, 'l', MAGENTA, "basilisk",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    250, 15, MONS_GIANT_LIZARD, MONS_BASILISK, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 4, 4, 0 },
    3, 12, MST_BASILISK, CE_POISONOUS, S_HISS,
    I_REPTILE, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

{
    MONS_KOMODO_DRAGON, 'l', LIGHTRED, "komodo dragon",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    800, 10, MONS_GIANT_LIZARD, MONS_KOMODO_DRAGON, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 34}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    7, 8, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

// merfolk ('m')
{
    MONS_MERFOLK, 'm', LIGHTRED, "merfolk",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    500, 4, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 22}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 4, 3, 0 },
    4, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MERFOLK_IMPALER, 'm', YELLOW, "merfolk impaler",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    500, 13, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 5, 3, 0 },
    // Impalers prefer light armour, and are dodging experts.
    0, 18, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, {10, 6, 6, 10, 10, 10, 10, 100},
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MERFOLK_JAVELINEER, 'm', WHITE, "merfolk javelineer",
    M_WARM_BLOOD | M_ARCHER | M_DONT_MELEE | M_SPEAKS,
    MR_NO_FLAGS,
    500, 12, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 5, 2, 0 },
    0, 14, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MERFOLK_AQUAMANCER, 'm', GREEN, "merfolk aquamancer",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS,
    MR_NO_FLAGS,
    500, 8, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 3, 0 },
    0, 12, MST_MERFOLK_AQUAMANCER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SIREN, 'm', CYAN, "siren",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    500, 10, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 19}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 3, 0 },
    4, 12, MST_SIREN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MERFOLK_AVATAR, 'm', LIGHTCYAN, "merfolk avatar",
    M_WARM_BLOOD | M_SPEAKS | M_NO_GEN_DERIVED,
    MR_NO_FLAGS,
    500, 10, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 120,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 5, 3, 0 },
    4, 12, MST_MERFOLK_AVATAR, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_WATER_NYMPH, 'm', MAGENTA, "water nymph",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_FEMALE,
    MR_NO_FLAGS,
    500, 10, MONS_WATER_NYMPH, MONS_WATER_NYMPH, MH_NATURAL, 100,
    { {AT_TOUCH, AF_DROWN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 4, 0 },
    4, 13, MST_WATER_NYMPH, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// rotting monsters ('n')
{
    MONS_BOG_BODY, 'n', GREEN, "bog body",
    M_NO_ZOMBIE,
    MR_RES_FIRE | MR_RES_COLD,
    500, 16, MONS_GHOUL, MONS_BOG_BODY, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 25}, {AT_TOUCH, AF_COLD, 4}, AT_NO_ATK, AT_NO_ATK },
    { 6, 5, 3, 0 },
    1, 9, MST_BOG_BODY, CE_ROT, S_SILENT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(14),
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_NECROPHAGE, 'n', LIGHTGREY, "necrophage",
    M_NO_ZOMBIE,
    MR_RES_COLD,
    500, 12, MONS_GHOUL, MONS_NECROPHAGE, MH_UNDEAD, 40,
    { {AT_HIT, AF_ROT, 8}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 10, MST_NO_SPELLS, CE_ROT, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_CORPSES, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GHOUL, 'n', RED, "ghoul",
    M_NO_ZOMBIE,
    MR_RES_COLD,
    500, 10, MONS_GHOUL, MONS_GHOUL, MH_UNDEAD, 80,
    { {AT_HIT, AF_ROT, 30}, {AT_CLAW, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK },
    { 14, 8, 5, 0 },
    4, 10, MST_NO_SPELLS, CE_ROT, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_CORPSES, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// orcs ('o')
{
    MONS_ORC, 'o', LIGHTRED, "orc",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 15, MONS_ORC, MONS_ORC, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 4, 6, 0 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_WIZARD, 'o', MAGENTA, "orc wizard",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 10, MONS_ORC, MONS_ORC, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 4, 0 },
    1, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_PRIEST, 'o', GREEN, "orc priest",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 10, MONS_ORC, MONS_ORC, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 4, 0 },
    1, 10, MST_ORC_PRIEST, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_WARRIOR, 'o', YELLOW, "orc warrior",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 16, MONS_ORC, MONS_ORC, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 4, 6, 0 },
    0, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_KNIGHT, 'o', CYAN, "orc knight",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 9, MONS_ORC, MONS_ORC, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 4, 7, 0 },
    2, 13, MST_BATTLECRY, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_WARLORD, 'o', LIGHTCYAN, "orc warlord",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    600, 11, MONS_ORC, MONS_ORC, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 32}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 7, 0 },
    3, 10, MST_BATTLECRY, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_SORCERER, 'o', LIGHTMAGENTA, "orc sorcerer",
    M_SEE_INVIS | M_SPEAKS | M_WARM_BLOOD,
    MR_NO_FLAGS,
    600, 12, MONS_ORC, MONS_ORC, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 2, 3, 0 },
    5, 12, MST_ORC_SORCERER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORC_HIGH_PRIEST, 'o', LIGHTGREEN, "orc high priest",
    M_SEE_INVIS | M_SPEAKS | M_WARM_BLOOD,
    MR_NO_FLAGS,
    600, 10, MONS_ORC, MONS_ORC, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 4, 0 },
    1, 12, MST_ORC_HIGH_PRIEST, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

#if TAG_MAJOR_VERSION == 34
{
    MONS_LAVA_ORC, 'o', RED, "lava orc",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    mrd(MR_RES_FIRE, 3),
    600, 15, MONS_ORC, MONS_LAVA_ORC, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 4, 6, 0 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},
#endif

// humans ('p')
{
    MONS_HUMAN, 'p', LIGHTGREY, "human",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    3, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SLAVE, 'p', WHITE, "slave",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_HELL_KNIGHT, 'p', RED, "hell knight",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_HELLFIRE,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 4, 5, 0 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DEATH_KNIGHT, 'p', GREEN, "death knight",
    M_WARM_BLOOD | M_FIGHTER | M_SPEAKS,
    MR_NO_FLAGS,
    600, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 28}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 5, 6, 0 },
    2, 12, MST_BK_YREDELEMNUL, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_NECROMANCER, 'p', WHITE, "necromancer",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 2, 4, 0 },
    0, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_WIZARD, 'p', MAGENTA, "wizard",
    M_SPEAKS | M_WARM_BLOOD,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 2, 4, 0 },
    0, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_VAULT_GUARD, 'p', CYAN, "vault guard",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    550, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 3, 5, 0 },
    1, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_VAULT_SENTINEL, 'p', LIGHTBLUE, "vault sentinel",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 4, 5, 0 },
    1, 13, MST_VAULT_SENTINEL, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_VAULT_WARDEN, 'p', LIGHTRED, "vault warden",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    MR_NO_FLAGS,
    550, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 36}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 3, 5, 0 },
    1, 13, MST_VAULT_WARDEN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_IRONBRAND_CONVOKER, 'p', YELLOW, "ironbrand convoker",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    0, 10, MST_IRONBRAND_CONVOKER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_IRONHEART_PRESERVER, 'p', LIGHTGREEN, "ironheart preserver",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 4, 5, 0 },
    0, 10, MST_IRONHEART_PRESERVER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SHAPESHIFTER, 'p', LIGHTRED, "shapeshifter",
    M_NO_SKELETON | M_NO_ZOMBIE,
    MR_NO_FLAGS,
    600, 10, MONS_SHAPESHIFTER, MONS_SHAPESHIFTER, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    0, 10, MST_NO_SPELLS, CE_MUTAGEN, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GLOWING_SHAPESHIFTER, 'p', LIGHTRED, "glowing shapeshifter",
    M_NO_SKELETON | M_NO_ZOMBIE | M_GLOWS_RADIATION,
    MR_NO_FLAGS,
    600, 10, MONS_SHAPESHIFTER, MONS_GLOWING_SHAPESHIFTER, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    0, 10, MST_NO_SPELLS, CE_MUTAGEN, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_KILLER_KLOWN, 'p', ETC_RANDOM, "Killer Klown",
    M_SEE_INVIS | M_SPEAKS | M_WARM_BLOOD | M_FAST_REGEN | M_NO_POLY_TO,
    MR_NO_FLAGS,
    550, 17, MONS_HUMAN, MONS_KILLER_KLOWN, MH_NATURAL, 160,
    { {AT_HIT, AF_KLOWN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 5, 5, 0 },
    10, 15, MST_BLINKER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{ // dummy; spawns in a single vault.
    MONS_DEMIGOD, 'p', YELLOW, "demigod",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_DEMIGOD, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 5, 5, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{ // dummy... literally; single vault
    MONS_HALFLING, 'p', LIGHTGREY, "halfling",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    400, 10, MONS_HALFLING, MONS_HALFLING, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 2, 3, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

// rodents ('r')
{
    MONS_RAT, 'r', BROWN, "rat",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    200, 1, MONS_RAT, MONS_RAT, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 3, 0 },
    1, 10, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_QUADRUPED
},

{
    MONS_QUOKKA, 'r', WHITE, "quokka",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    300, 10, MONS_QUOKKA, MONS_QUOKKA, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 3, 5, 0 },
    2, 13, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

{
    MONS_RIVER_RAT, 'r', LIGHTGREEN, "river rat",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    220, 13, MONS_RAT, MONS_RIVER_RAT, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    5, 11, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_QUADRUPED
},

{
    MONS_HELL_RAT, 'r', LIGHTRED, "hell rat",
    M_WARM_BLOOD | M_GLOWS_LIGHT,
    MR_NO_FLAGS,
    300, 10, MONS_RAT, MONS_HELL_RAT, MH_DEMONIC, 20,
    { {AT_BITE, AF_DRAIN_XP, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    7, 10, MST_NO_SPELLS, CE_POISONOUS, S_ROAR,
    I_ANIMAL, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

{
    MONS_PORCUPINE, 'r', BLUE, "porcupine",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    220, 26, MONS_RAT, MONS_PORCUPINE, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 2, 4, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_QUADRUPED
},

// spiders and insects ('s')
{
    MONS_GIANT_COCKROACH, 's', BROWN, "giant cockroach",
    M_NO_SKELETON,
    MR_VUL_POISON,
    250, 10, MONS_GIANT_COCKROACH, MONS_GIANT_COCKROACH, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 2}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 3, 4, 0 },
    3, 10, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_INSECT
},

{
    MONS_GIANT_MITE, 's', LIGHTRED, "giant mite",
    M_NO_SKELETON,
    MR_VUL_POISON,
    300, 10, MONS_GIANT_MITE, MONS_GIANT_MITE, MH_NATURAL, 0,
    { {AT_BITE, AF_POISON, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    1, 7, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ARACHNID
},

{
    MONS_SCORPION, 's', YELLOW, "scorpion",
    M_NO_SKELETON,
    MR_VUL_POISON,
    320, 13, MONS_SCORPION, MONS_SCORPION, MH_NATURAL, 20,
    { {AT_STING, AF_POISON, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 3, 0 },
    5, 10, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ARACHNID
},

{
    MONS_EMPEROR_SCORPION, 's', LIGHTGREY, "emperor scorpion",
    M_NO_SKELETON,
    MR_RES_POISON,
    900, 13, MONS_SCORPION, MONS_EMPEROR_SCORPION, MH_NATURAL, 60,
    { {AT_STING, AF_POISON, 30}, {AT_CLAW, AF_PLAIN, 15},
      {AT_CLAW, AF_PLAIN, 15}, AT_NO_ATK },
    { 14, 6, 5, 0 },
    20, 12, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_ARACHNID
},

DUMMY(MONS_SPIDER, 's', CYAN, "spider")

{
    MONS_TARANTELLA, 's', LIGHTMAGENTA, "tarantella",
    M_NO_SKELETON | M_WEB_SENSE,
    MR_VUL_POISON,
    300, 6, MONS_SPIDER, MONS_TARANTELLA, MH_NATURAL, 20,
    { {AT_BITE, AF_CONFUSE, 19}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 2, 3, 0 },
    3, 14, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_INSECT, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ARACHNID
},

{
    MONS_JUMPING_SPIDER, 's', LIGHTBLUE, "jumping spider",
    M_NO_SKELETON | M_WEB_SENSE | M_SEE_INVIS | M_UNBLINDABLE,
    MR_VUL_POISON,
    300, 8, MONS_SPIDER, MONS_JUMPING_SPIDER, MH_NATURAL, 20,
    { {AT_POUNCE, AF_ENSNARE, 20}, {AT_BITE, AF_PLAIN, 8}, AT_NO_ATK,
       AT_NO_ATK },
    { 8, 2, 4, 0 },
    6, 12, MST_JUMPING_SPIDER, CE_POISONOUS, S_HISS,
    I_INSECT, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ARACHNID
},

{
    MONS_WOLF_SPIDER, 's', WHITE, "wolf spider",
    M_NO_SKELETON | M_WEB_SENSE,
    MR_VUL_POISON,
    900, 4, MONS_SPIDER, MONS_WOLF_SPIDER, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 25}, {AT_BITE, AF_POISON, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 11, 3, 4, 0 },
    3, 10, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_INSECT, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ARACHNID
},

{
    MONS_TRAPDOOR_SPIDER, 's', LIGHTCYAN, "trapdoor spider",
    M_NO_SKELETON | M_SUBMERGES | M_WEB_SENSE,
    MR_VUL_POISON,
    240, 5, MONS_SPIDER, MONS_TRAPDOOR_SPIDER, MH_NATURAL, 20,
    { {AT_BITE, AF_POISON, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 4, 0 },
    3, 10, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_INSECT, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ARACHNID
},

{
    MONS_REDBACK, 's', RED, "redback",
    M_NO_SKELETON | M_WEB_SENSE,
    MR_VUL_POISON,
    130, 5, MONS_SPIDER, MONS_REDBACK, MH_NATURAL, 20,
    { {AT_BITE, AF_POISON_STRONG, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 2, 2, 0 },
    2, 12, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_ARACHNID
},

{
    MONS_DEMONIC_CRAWLER, 's', LIGHTGREEN, "demonic crawler",
    M_NO_SKELETON | M_SEE_INVIS,
    MR_RES_ELEC | MR_RES_POISON | MR_RES_COLD | MR_RES_FIRE,
    900, 4, MONS_DEMONIC_CRAWLER, MONS_DEMONIC_CRAWLER, MH_DEMONIC, 100,
    { {AT_HIT, AF_PLAIN, 13}, {AT_HIT, AF_PLAIN, 13}, {AT_HIT, AF_PLAIN, 13},
       AT_NO_ATK },
    { 9, 4, 5, 0 },
    10, 6, MST_NO_SPELLS, CE_POISONOUS, S_SCREAM,
    I_INSECT, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_CENTIPEDE
},

{
    MONS_ORB_SPIDER, 's', MAGENTA, "orb spider",
    M_NO_SKELETON | M_WEB_SENSE | M_MAINTAIN_RANGE,
    MR_VUL_POISON,
    300, 20, MONS_SPIDER, MONS_ORB_SPIDER, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 4, 4, 0 },
    3, 10, MST_ORB_SPIDER, CE_POISONOUS, S_HISS,
    I_INSECT, HT_LAND, FL_NONE, 12, SPELL_ENERGY(20),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ARACHNID
},

// testudines and crocodiles ('t')
{
    MONS_CROCODILE, 't', BROWN, "crocodile",
    M_COLD_BLOOD | M_SUBMERGES,
    MR_NO_FLAGS,
    800, 10, MONS_CROCODILE, MONS_CROCODILE, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    4, 10, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_BABY_ALLIGATOR, 't', BLUE, "baby alligator",
    M_COLD_BLOOD | M_SUBMERGES,
    MR_NO_FLAGS,
    300, 10, MONS_CROCODILE, MONS_BABY_ALLIGATOR, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    1, 11, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 12, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_ALLIGATOR, 't', LIGHTBLUE, "alligator",
    M_COLD_BLOOD | M_SUBMERGES,
    MR_NO_FLAGS,
    850, 10, MONS_CROCODILE, MONS_ALLIGATOR, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 30}, {AT_TAIL_SLAP, AF_PLAIN, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 12, 3, 6, 0 },
    5, 9, MST_ALLIGATOR, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, {10, 6, 8, 8, 8, 8, 8, 80},
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_SNAPPING_TURTLE, 't', GREEN, "snapping turtle",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    600, 10, MONS_SNAPPING_TURTLE, MONS_SNAPPING_TURTLE, MH_NATURAL, 40,
    { {AT_BITE, AF_REACH, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 6, 0 },
    16, 5, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 8, {10, 6, 8, 8, 8, 8, 8, 80},
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

{
    MONS_ALLIGATOR_SNAPPING_TURTLE, 't', LIGHTGREEN, "alligator snapping turtle",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    1100, 10, MONS_SNAPPING_TURTLE, MONS_ALLIGATOR_SNAPPING_TURTLE,
        MH_NATURAL, 60,
    { {AT_BITE, AF_REACH, 50}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 7, 4, 0 },
    19, 1, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 8, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_TAILLESS
},

DUMMY(MONS_CRAB, 't', LIGHTGREY, "crab")

{
    MONS_FIRE_CRAB, 't', LIGHTRED, "fire crab",
    M_NO_SKELETON,
    MR_VUL_POISON | mrd(MR_RES_FIRE, 3),
    320, 25, MONS_CRAB, MONS_FIRE_CRAB, MH_NATURAL, 40,
    { {AT_BITE, AF_FIRE, 15}, {AT_CLAW, AF_FIRE, 15}, AT_NO_ATK, AT_NO_ATK },
    { 8, 4, 5, 0 },
    9, 6, MST_FIRE_CRAB, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED // ??
},

{
    MONS_GHOST_CRAB, 't', LIGHTGREY, "ghost crab",
    M_NO_SKELETON,
    MR_VUL_POISON | mrd(MR_RES_NEG, 3) | MR_RES_ROTTING,
    320, 25, MONS_CRAB, MONS_GHOST_CRAB, MH_NATURAL, 40,
    { {AT_BITE, AF_DRAIN_XP, 20}, {AT_CLAW, AF_DRAIN_XP, 15}, AT_NO_ATK, AT_NO_ATK },
    { 9, 4, 5, 0 },
    9, 6, MST_GHOST_CRAB, CE_ROT, S_SILENT,
    I_INSECT, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

{
    MONS_APOCALYPSE_CRAB, 't', WHITE, "apocalypse crab",
    M_NO_SKELETON | M_SEE_INVIS,
    MR_VUL_POISON | mrd(MR_RES_FIRE | MR_RES_COLD, 2),
    320, 13, MONS_CRAB, MONS_APOCALYPSE_CRAB, MH_DEMONIC, 60,
    { {AT_BITE, AF_CHAOS, 15}, {AT_CLAW, AF_CHAOS, 15}, AT_NO_ATK, AT_NO_ATK },
    { 8, 4, 5, 0 },
    11, 6, MST_APOCALYPSE_CRAB, CE_MUTAGEN, S_SILENT,
    I_INSECT, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_QUADRUPED
},

// ugly things ('u')
// NOTE: ugly things are ghost demons and thus their stats vary treacherously
// from those below. beware!
{
    MONS_UGLY_THING, 'u', COLOUR_UNDEF, "ugly thing",
    M_WARM_BLOOD | M_GLOWS_RADIATION | M_HERD | M_NO_GEN_DERIVED,
    MR_NO_FLAGS,
    600, 6, MONS_UGLY_THING, MONS_UGLY_THING, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 5, 0 },
    4, 10, MST_NO_SPELLS, CE_MUTAGEN, S_GURGLE,
    I_ANIMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(9),
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_VERY_UGLY_THING, 'u', COLOUR_UNDEF, "very ugly thing",
    M_WARM_BLOOD | M_GLOWS_RADIATION | M_HERD | M_NO_GEN_DERIVED,
    MR_NO_FLAGS,
    830, 10, MONS_UGLY_THING, MONS_VERY_UGLY_THING, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 3, 5, 0 },
    6, 10, MST_NO_SPELLS, CE_MUTAGEN, S_GURGLE,
    I_ANIMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(9),
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

// vortices ('v')
{
    MONS_FIRE_VORTEX, 'v', RED, "fire vortex",
    M_CONFUSED | M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_CONJURED,
    MR_RES_POISON | mrd(MR_RES_FIRE, 3) | MR_VUL_COLD | MR_RES_ELEC,
    0, 5, MONS_FIRE_VORTEX, MONS_FIRE_VORTEX, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PURE_FIRE, 0}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 2, 2, 0 },
    0, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_SPATIAL_VORTEX, 'v', ETC_RANDOM, "spatial vortex",
    M_CONFUSED | M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_CONJURED,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    0, 5, MONS_FIRE_VORTEX, MONS_SPATIAL_VORTEX, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_DISTORT, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 6, 6, 0 },
    0, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_SPATIAL_MAELSTROM, 'v', YELLOW, "spatial maelstrom",
    M_BATTY | M_INSUBSTANTIAL | M_GLOWS_LIGHT,
    MR_RES_POISON | mrd(MR_RES_FIRE, 2) | mrd(MR_RES_COLD, 2) | MR_RES_ELEC,
    0, 5, MONS_SPATIAL_MAELSTROM, MONS_SPATIAL_MAELSTROM,
        MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_DISTORT, 20}, {AT_HIT, AF_DISTORT, 20}, AT_NO_ATK,
       AT_NO_ATK },
    { 10, 6, 3, 0 },
    0, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 16, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_INSUBSTANTIAL_WISP, 'v', LIGHTGREY, "insubstantial wisp",
    M_INSUBSTANTIAL,
    mrd(MR_RES_ELEC | MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD, 2),
    0, 8, MONS_INSUBSTANTIAL_WISP, MONS_INSUBSTANTIAL_WISP,
        MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_BLINK, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 2, 1, 0 },
    4, 18, MST_INSUBSTANTIAL_WISP, CE_NOCORPSE, S_MOAN,
    I_PLANT, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

{ // miscast only
    MONS_TWISTER, 'v', ETC_AIR, "twister",
    M_CONFUSED | M_INSUBSTANTIAL | M_BATTY | M_NO_EXP_GAIN | M_NO_POLY_TO,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | mrd(MR_RES_ELEC, 3)
        | MR_RES_WIND,
    0, 5, MONS_FIRE_VORTEX, MONS_TWISTER, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 0, 0, 10000 },
    0, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

// worms and slugs ('w')
{
    MONS_WORM, 'w', LIGHTRED, "worm",
    M_NO_SKELETON,
    MR_NO_FLAGS,
    400, 3, MONS_WORM, MONS_WORM, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    1, 5, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_SNAKE
},

{
    MONS_BRAIN_WORM, 'w', LIGHTMAGENTA, "brain worm",
    M_NO_SKELETON,
    MR_NO_FLAGS,
    280, 10, MONS_WORM, MONS_BRAIN_WORM, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 3, 0 },
    1, 5, MST_BRAIN_WORM, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_SNAKE
},

{
    MONS_SWAMP_WORM, 'w', BROWN, "swamp worm",
    M_NO_SKELETON | M_SUBMERGES,
    MR_NO_FLAGS,
    450, 3, MONS_WORM, MONS_SWAMP_WORM, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 2, 3, 0 },
    3, 12, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_INSECT, HT_WATER, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

{
    MONS_GIANT_LEECH, 'w', RED, "giant leech",
    M_NO_SKELETON | M_BLOOD_SCENT,
    MR_NO_FLAGS,
    1000, 8, MONS_GIANT_LEECH, MONS_GIANT_LEECH, MH_NATURAL, 40,
    { {AT_BITE, AF_VAMPIRIC, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 4, 0 },
    5, 15, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_INSECT, HT_AMPHIBIOUS, FL_NONE, 8, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_TORPOR_SNAIL, 'w', GREEN, "torpor snail",
    M_NO_SKELETON,
    MR_NO_FLAGS,
    1200, 20, MONS_ELEPHANT_SLUG, MONS_TORPOR_SNAIL, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 5, 2, 0 },
    8, 1, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_AMPHIBIOUS, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAIL
},

// dummy genus monster; not using DUMMY since it's also a dummy species
{
    MONS_ELEPHANT_SLUG, 'w', WHITE, "elephant slug",
    M_NO_SKELETON | M_CANT_SPAWN,
    MR_VUL_POISON,
    1800, 2, MONS_ELEPHANT_SLUG, MONS_ELEPHANT_SLUG, MH_NATURAL, 80,
    { {AT_BITE, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 5, 3, 0 },
    2, 1, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 4, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_SNAIL
},

// small abominations ('x')
{
    MONS_UNSEEN_HORROR, 'x', MAGENTA, "unseen horror",
    M_SEE_INVIS | M_INVIS | M_BATTY,
    MR_NO_FLAGS,
    550, 12, MONS_UNSEEN_HORROR, MONS_UNSEEN_HORROR, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    5, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    // A demonically controlled mass of undead flesh theme-wise, this makes
    // them MH_DEMONIC|MH_UNDEAD, but this is mostly the same as just
    // MH_UNDEAD (save for some god interactions).
    MONS_ABOMINATION_SMALL, 'x', LIGHTRED, "small abomination",
    M_NO_REGEN,
    MR_NO_FLAGS,
    1300, 10, MONS_ABOMINATION_SMALL, MONS_ABOMINATION_SMALL, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 2, 5, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    // See comment under MONS_ABOMINATION_SMALL regarding holiness.
    MONS_CRAWLING_CORPSE, 'x', BROWN, "crawling corpse",
    M_NO_EXP_GAIN | M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    150, 8, MONS_MACABRE_MASS, MONS_CRAWLING_CORPSE, MH_UNDEAD, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

{
    // See comment under MONS_ABOMINATION_SMALL regarding holiness.
    MONS_MACABRE_MASS, 'x', BROWN, "macabre mass",
    M_NO_EXP_GAIN | M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    425, 8, MONS_MACABRE_MASS, MONS_MACABRE_MASS, MH_UNDEAD, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 2, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_ANCIENT_ZYME, 'x', GREEN, "ancient zyme",
    M_SEE_INVIS,
    MR_RES_POISON,
    550, 8, MONS_ANCIENT_ZYME, MONS_ANCIENT_ZYME, MH_NONLIVING, 60,
    { {AT_HIT, AF_DRAIN_STR, 16}, {AT_HIT, AF_DRAIN_DEX, 16}, AT_NO_ATK,
       AT_NO_ATK },
    { 8, 4, 5, 0 },
    6, 6, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_WORLDBINDER, 'x', CYAN, "worldbinder",
    M_SEE_INVIS,
    MR_RES_POISON,
    425, 13, MONS_WORLDBINDER, MONS_WORLDBINDER, MH_NONLIVING, 60,
    { {AT_HIT, AF_PLAIN, 8}, {AT_HIT, AF_PLAIN, 8},
       AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 4, 0 },
    12, 4, MST_WORLDBINDER, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_LEVITATE, 14, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

// flying insects ('y')
{
    MONS_KILLER_BEE, 'y', BROWN, "killer bee",
    M_NO_SKELETON,
    MR_VUL_POISON,
    150, 11, MONS_KILLER_BEE, MONS_KILLER_BEE, MH_NATURAL, 10,
    { {AT_STING, AF_POISON, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    2, 18, MST_NO_SPELLS, CE_POISONOUS, S_BUZZ,
    I_INSECT, HT_LAND, FL_WINGED, 20, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_INSECT_WINGED
},

{
    MONS_QUEEN_BEE, 'y', LIGHTMAGENTA, "queen bee",
    M_NO_SKELETON,
    MR_VUL_POISON,
    300, 14, MONS_KILLER_BEE, MONS_QUEEN_BEE, MH_NATURAL, 20,
    { {AT_STING, AF_POISON, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    10, 10, MST_QUEEN_BEE, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_INSECT_WINGED
},

{
    MONS_YELLOW_WASP, 'y', YELLOW, "yellow wasp",
    M_NO_SKELETON,
    MR_VUL_POISON,
    170, 15, MONS_YELLOW_WASP, MONS_YELLOW_WASP, MH_NATURAL, 20,
    { {AT_STING, AF_PARALYSE, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 5, 0 },
    5, 14, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_INSECT_WINGED
},

{
    MONS_VAMPIRE_MOSQUITO, 'y', GREEN, "vampire mosquito",
    M_NO_SKELETON | M_BLOOD_SCENT,
    MR_NO_FLAGS,
    200, 10, MONS_VAMPIRE_MOSQUITO, MONS_VAMPIRE_MOSQUITO, MH_UNDEAD, 20,
    { {AT_BITE, AF_VAMPIRIC, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 15, MST_NO_SPELLS, CE_ROT, S_BUZZ,
    I_INSECT, HT_LAND, FL_WINGED, 19, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_INSECT_WINGED
},

{
    MONS_RED_WASP, 'y', RED, "red wasp",
    M_NO_SKELETON,
    MR_VUL_POISON,
    180, 12, MONS_RED_WASP, MONS_RED_WASP, MH_NATURAL, 40,
    { {AT_STING, AF_PARALYSE, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    7, 14, MST_NO_SPELLS, CE_POISONOUS, S_BUZZ,
    I_INSECT, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_INSECT_WINGED
},

DUMMY(MONS_MOTH, 'y', WHITE, "moth")

{
    MONS_GHOST_MOTH, 'y', MAGENTA, "ghost moth",
    M_NO_SKELETON | M_INVIS,
    MR_RES_POISON | MR_RES_COLD,
    600, 18, MONS_MOTH, MONS_GHOST_MOTH, MH_NATURAL, 100,
    { {AT_HIT, AF_DRAIN_STAT, 18}, {AT_HIT, AF_DRAIN_STAT, 18},
      {AT_STING, AF_POISON_STRONG, 12}, AT_NO_ATK },
    { 13, 3, 5, 0 },
    8, 10, MST_DRAIN_GAZE, CE_MUTAGEN, S_SILENT,
    I_INSECT, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_INSECT_WINGED
},

{
    MONS_MOTH_OF_WRATH, 'y', LIGHTRED, "moth of wrath",
    M_NO_SKELETON,
    MR_NO_FLAGS,
    300, 10, MONS_MOTH, MONS_MOTH_OF_WRATH, MH_NATURAL, 40,
    { {AT_BITE, AF_RAGE, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    0, 10, MST_MOTH_OF_WRATH, CE_CLEAN, S_SILENT,
    I_INSECT, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_INSECT_WINGED
},

// corporeal (semi-)humanoid undead ('z')
{
    MONS_WIGHT, 'z', GREEN, "wight",
    M_NO_FLAGS,
    mrd(MR_RES_COLD, 2),
    550, 16, MONS_WIGHT, MONS_WIGHT, MH_UNDEAD, 20,
    { {AT_HIT, AF_DRAIN_XP, 8}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    4, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SKELETAL_WARRIOR, 'z', CYAN, "skeletal warrior",
    M_FIGHTER,
    MR_RES_COLD,
    275, 10, MONS_SKELETAL_WARRIOR, MONS_SKELETAL_WARRIOR, MH_UNDEAD, 100,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 5, 3, 0 },
    15, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ANCIENT_CHAMPION, 'z', LIGHTMAGENTA, "ancient champion",
    M_FIGHTER,
    MR_RES_COLD,
    275, 24, MONS_ANCIENT_CHAMPION, MONS_ANCIENT_CHAMPION, MH_UNDEAD, 120,
    { {AT_HIT, AF_PLAIN, 32}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 4, 2, 0 },
    15, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FLYING_SKULL, 'z', WHITE, "flying skull",
    M_NO_FLAGS,
    MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    75, 10, MONS_FLYING_SKULL, MONS_FLYING_SKULL, MH_UNDEAD, 20,
    { {AT_HIT, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 2, 2, 0 },
    10, 17, MST_NO_SPELLS, CE_NOCORPSE, S_SCREAM,
    I_ANIMAL, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

{
    MONS_CURSE_SKULL, 'z', LIGHTCYAN, "curse skull",
    M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2),
    75, 20, MONS_CURSE_SKULL, MONS_CURSE_SKULL, MH_UNDEAD, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 0, 0, 55 },
    35, 3, MST_CURSE_SKULL, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

{
    MONS_CURSE_TOE, 'z', YELLOW, "curse toe",
    M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_ELEC, 2) | MR_RES_HELLFIRE | MR_RES_COLD,
    75, 60, MONS_LICH, MONS_CURSE_TOE, MH_UNDEAD, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 100 },
    25, 1, MST_CURSE_TOE, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(14),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

// angelic beings ('A')
{
    MONS_ANGEL, 'A', WHITE, "angel",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_GLOWS_LIGHT,
    MR_RES_POISON | mrd(MR_RES_ELEC, 2),
    550, 10, MONS_ANGEL, MONS_ANGEL, MH_HOLY, 120,
    { {AT_HIT, AF_PLAIN, 25}, {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK,
       AT_NO_ATK },
    { 12, 6, 5, 0 },
    10, 20, MST_ANGEL, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_CHERUB, 'A', LIGHTBLUE, "cherub",
    M_FIGHTER | M_ARCHER | M_SEE_INVIS | M_SPEAKS | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_ELEC | MR_RES_FIRE,
    550, 10, MONS_ANGEL, MONS_CHERUB, MH_HOLY, 100,
    { {AT_HIT, AF_PLAIN, 15}, {AT_CHERUB, AF_PLAIN, 8}, AT_NO_ATK,
       AT_NO_ATK },
    { 9, 6, 5, 0 },
    10, 20, MST_BATTLECRY, CE_NOCORPSE, S_CHERUB,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_SERAPH, 'A', LIGHTMAGENTA, "seraph",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_ELEC | mrd(MR_RES_FIRE, 3),
    550, 10, MONS_ANGEL, MONS_SERAPH, MH_HOLY, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 50}, {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK,
       AT_NO_ATK },
    { 25, 6, 5, 0 },
    10, 20, MST_SERAPH, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_DAEVA, 'A', YELLOW, "daeva",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_GLOWS_LIGHT,
    MR_RES_POISON,
    550, 12, MONS_ANGEL, MONS_DAEVA, MH_HOLY, 140,
    { {AT_HIT, AF_PLAIN, 25}, {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK,
       AT_NO_ATK },
    { 14, 6, 5, 0 },
    10, 13, MST_DAEVA, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_PROFANE_SERVITOR, 'A', RED, "profane servitor",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS,
    MR_RES_COLD | mrd(MR_RES_ELEC, 2),
    550, 10, MONS_ANGEL, MONS_PROFANE_SERVITOR, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_VAMPIRIC, 25}, {AT_HIT, AF_DRAIN_XP, 10}, AT_NO_ATK,
       AT_NO_ATK },
    { 18, 6, 5, 0 },
    10, 20, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

// beetles ('B')
{
    MONS_GOLIATH_BEETLE, 'B', BLUE, "goliath beetle",
    M_NO_SKELETON,
    MR_VUL_POISON,
    800, 12, MONS_GOLIATH_BEETLE, MONS_GOLIATH_BEETLE, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 7, 6, 0 },
    10, 3, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_INSECT
},

{
    MONS_BOULDER_BEETLE, 'B', LIGHTGREY, "boulder beetle",
    M_NO_SKELETON,
    MR_VUL_POISON,
    2050, 14, MONS_GOLIATH_BEETLE, MONS_BOULDER_BEETLE, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 6, 5, 0 },
    20, 2, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_INSECT
},

{
    MONS_BORING_BEETLE, 'B', BROWN, "boring beetle",
    M_NO_SKELETON | M_BURROWS,
    MR_VUL_POISON,
    1300, 10, MONS_GOLIATH_BEETLE, MONS_BORING_BEETLE, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    13, 4, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_INSECT
},

{
    MONS_DEATH_SCARAB, 'B', MAGENTA, "death scarab",
    M_NO_SKELETON | M_BATTY,
    MR_NO_FLAGS,
    300, 12, MONS_DEATH_SCARAB, MONS_DEATH_SCARAB, MH_UNDEAD, 60,
    { {AT_BITE, AF_SCARAB, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 2, 3, 0 },
    7, 14, MST_NO_SPELLS, CE_ROT, S_SILENT,
    I_INSECT, HT_LAND, FL_NONE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_INSECT
},

// cyclopes and giants ('C')
DUMMY(MONS_GIANT, 'C', LIGHTGREY, "giant")

{
    MONS_HILL_GIANT, 'C', LIGHTRED, "hill giant",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    1700, 7, MONS_GIANT, MONS_HILL_GIANT, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    3, 4, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_CYCLOPS, 'C', YELLOW, "cyclops",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    2200, 14, MONS_GIANT, MONS_CYCLOPS, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    5, 3, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_ETTIN, 'C', BROWN, "ettin",
    M_WARM_BLOOD | M_TWO_WEAPONS | M_SPEAKS,
    MR_NO_FLAGS,
    2500, 12, MONS_GIANT, MONS_ETTIN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 45}, {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 5, 0 },
    9, 4, MST_NO_SPELLS, CE_CLEAN, S_SHOUT2,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_FIRE_GIANT, 'C', RED, "fire giant",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_FIRE, 2),
    2000, 13, MONS_GIANT, MONS_FIRE_GIANT, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 3, 6, 0 },
    8, 4, MST_FIRE_GIANT, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_FROST_GIANT, 'C', LIGHTBLUE, "frost giant",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_COLD, 2),
    2100, 11, MONS_GIANT, MONS_FROST_GIANT, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 4, 5, 0 },
    9, 3, MST_FROST_GIANT, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_STONE_GIANT, 'C', LIGHTGREY, "stone giant",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    2700, 9, MONS_GIANT, MONS_STONE_GIANT, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 3, 5, 0 },
    12, 2, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_TITAN, 'C', MAGENTA, "titan",
    M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_ELEC, 2),
    3200, 12, MONS_GIANT, MONS_TITAN, MH_NATURAL, 180,
    { {AT_HIT, AF_PLAIN, 55}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 3, 5, 0 },
    10, 3, MST_TITAN, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

// dragons ('D')
DUMMY(MONS_DRAGON, 'D', GREEN, "dragon")

{
    MONS_WYVERN, 'D', LIGHTRED, "wyvern",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    1200, 15, MONS_WYVERN, MONS_WYVERN, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    5, 10, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_FIRE_DRAGON, 'D', GREEN, "fire dragon",
    M_WARM_BLOOD,
    MR_RES_POISON | mrd(MR_RES_FIRE, 2) | MR_VUL_COLD,
    2400, 12, MONS_DRAGON, MONS_FIRE_DRAGON, MH_NATURAL, 60,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 13},
      {AT_TRAMPLE, AF_PLAIN, 13}, AT_NO_ATK },
    { 12, 5, 5, 0 },
    10, 8, MST_FIRE_DRAGON_BREATH, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_HYDRA, 'D', LIGHTGREEN, "hydra",
    M_COLD_BLOOD | M_FAST_REGEN,
    MR_RES_POISON,
    1800, 11, MONS_HYDRA, MONS_HYDRA, MH_NATURAL, 60,
    { {AT_BITE, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 3, 5, 0 },
    0, 5, MST_NO_SPELLS, CE_POISONOUS, S_ROAR,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED
},

{
    MONS_ICE_DRAGON, 'D', WHITE, "ice dragon",
    M_COLD_BLOOD,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 2),
    2400, 10, MONS_DRAGON, MONS_ICE_DRAGON, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 17}, {AT_CLAW, AF_PLAIN, 17},
      {AT_TRAMPLE, AF_PLAIN, 17}, AT_NO_ATK },
    { 12, 5, 5, 0 },
    10, 8, MST_ICE_DRAGON_BREATH, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_STEAM_DRAGON, 'D', BLUE, "steam dragon",
    M_WARM_BLOOD,
    MR_RES_STEAM,
    1500, 29, MONS_DRAGON, MONS_STEAM_DRAGON, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 12}, {AT_CLAW, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK },
    { 4, 5, 5, 0 },
    5, 10, MST_STEAM_DRAGON, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SWAMP_DRAGON, 'D', BROWN, "swamp dragon",
    M_WARM_BLOOD,
    MR_RES_POISON,
    2200, 11, MONS_DRAGON, MONS_SWAMP_DRAGON, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 11},
      {AT_TRAMPLE, AF_PLAIN, 11}, AT_NO_ATK },
    { 9, 5, 5, 0 },
    7, 7, MST_SWAMP_DRAGON, CE_POISONOUS, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_MOTTLED_DRAGON, 'D', LIGHTMAGENTA, "mottled dragon",
    M_WARM_BLOOD,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_STICKY_FLAME,
    1300, 16, MONS_DRAGON, MONS_MOTTLED_DRAGON, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 15}, {AT_CLAW, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    5, 10, MST_MOTTLED_DRAGON, CE_POISONOUS, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_QUICKSILVER_DRAGON, 'D', LIGHTCYAN, "quicksilver dragon",
    M_SEE_INVIS | M_WARM_BLOOD,
    MR_NO_FLAGS,
    7600, 14, MONS_DRAGON, MONS_QUICKSILVER_DRAGON, MH_NATURAL, 140,
    { {AT_BITE, AF_PLAIN, 25}, {AT_CLAW, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK },
    { 16, 3, 5, 0 },
    10, 15, MST_QUICKSILVER_DRAGON, CE_CLEAN, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_IRON_DRAGON, 'D', CYAN, "iron dragon",
    M_SEE_INVIS | M_WARM_BLOOD | M_UNBREATHING,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD,
    8400, 14, MONS_DRAGON, MONS_IRON_DRAGON, MH_NATURAL, 160,
    { {AT_BITE, AF_PLAIN, 25}, {AT_CLAW, AF_PLAIN, 25},
      {AT_TRAMPLE, AF_PLAIN, 25}, AT_NO_ATK },
    { 18, 5, 3, 0 },
    20, 6, MST_IRON_DRAGON, CE_CLEAN, S_ROAR,
    I_ANIMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

{
    MONS_STORM_DRAGON, 'D', LIGHTBLUE, "storm dragon",
    M_WARM_BLOOD,
    mrd(MR_RES_ELEC, 3) | MR_RES_COLD,
    2700, 13, MONS_DRAGON, MONS_STORM_DRAGON, MH_NATURAL, 100,
    { {AT_BITE, AF_PLAIN, 25}, {AT_CLAW, AF_PLAIN, 15},
      {AT_TRAMPLE, AF_PLAIN, 15}, AT_NO_ATK },
    { 14, 5, 5, 0 },
    13, 10, MST_STORM_DRAGON, CE_CLEAN, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_GOLDEN_DRAGON, 'D', YELLOW, "golden dragon",
    M_SEE_INVIS | M_WARM_BLOOD,
    MR_RES_ELEC | MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD,
    3000, 26, MONS_DRAGON, MONS_GOLDEN_DRAGON, MH_NATURAL, 180,
    { {AT_BITE, AF_PLAIN, 40}, {AT_CLAW, AF_PLAIN, 20},
      {AT_TRAMPLE, AF_PLAIN, 20}, AT_NO_ATK },
    { 18, 4, 4, 0 },
    15, 7, MST_GOLDEN_DRAGON, CE_POISONOUS, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SHADOW_DRAGON, 'D', MAGENTA, "shadow dragon",
    M_SHADOW | M_SEE_INVIS | M_COLD_BLOOD,
    MR_RES_POISON | mrd(MR_RES_COLD, 2) | mrd(MR_RES_NEG, 3),
    1800, 12, MONS_DRAGON, MONS_SHADOW_DRAGON, MH_NATURAL, 120,
    { {AT_BITE, AF_DRAIN_XP, 20}, {AT_CLAW, AF_PLAIN, 15},
      {AT_CLAW, AF_PLAIN, 15}, AT_NO_ATK },
    { 17, 5, 5, 0 },
    15, 10, MST_SHADOW_DRAGON, CE_ROT, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_BONE_DRAGON, 'D', LIGHTGREY, "bone dragon",
    M_SEE_INVIS,
    MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    900, 12, MONS_DRAGON, MONS_BONE_DRAGON, MH_UNDEAD, 100,
    { {AT_BITE, AF_PLAIN, 30}, {AT_CLAW, AF_PLAIN, 20},
      {AT_TRAMPLE, AF_PLAIN, 20}, AT_NO_ATK },
    { 20, 6, 6, 0 },
    20, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_PEARL_DRAGON, 'D', ETC_HOLY, "pearl dragon",
    M_SEE_INVIS | M_WARM_BLOOD | M_GLOWS_LIGHT,
    MR_NO_FLAGS,
    1900, 16, MONS_DRAGON, MONS_PEARL_DRAGON, MH_HOLY, 160,
    { {AT_BITE, AF_HOLY, 35}, {AT_CLAW, AF_HOLY, 20}, AT_NO_ATK, AT_NO_ATK },
    { 18, 4, 5, 0 },
    10, 15, MST_PEARL_DRAGON, CE_CLEAN, S_ROAR,
    I_ANIMAL, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

// elementals (E)
DUMMY(MONS_ELEMENTAL, 'E', LIGHTGREY, "elemental")

{
    MONS_EARTH_ELEMENTAL, 'E', ETC_EARTH, "earth elemental",
    M_NO_FLAGS,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 3)
        | MR_RES_PETRIFY,
    3000, 13, MONS_ELEMENTAL, MONS_EARTH_ELEMENTAL, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 5, 5, 0 },
    14, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_FIRE_ELEMENTAL, 'E', ETC_FIRE, "fire elemental",
    M_INSUBSTANTIAL | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD | MR_RES_ELEC | MR_VUL_WATER,
    1500, 10, MONS_ELEMENTAL, MONS_FIRE_ELEMENTAL, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PURE_FIRE, 0}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    4, 12, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_WATER_ELEMENTAL, 'E', ETC_WATER, "water elemental",
    M_NO_FLAGS,
    MR_RES_POISON | MR_VUL_FIRE | MR_RES_ELEC,
    1500, 12, MONS_ELEMENTAL, MONS_WATER_ELEMENTAL, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_ENGULF, 22}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 5, 4, 0 },
    4, 7, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_AIR_ELEMENTAL, 'E', ETC_AIR, "air elemental",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_GLOWS_LIGHT,
    mrd(MR_RES_ELEC, 3) | MR_RES_POISON | MR_RES_WIND,
    1500, 6, MONS_ELEMENTAL, MONS_AIR_ELEMENTAL, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    2, 18, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 25, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_IRON_ELEMENTAL, 'E', ETC_IRON, "iron elemental",
    M_NO_FLAGS,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 3),
    1500, 13, MONS_ELEMENTAL, MONS_IRON_ELEMENTAL, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 6, 6, 0 },
    20, 2, MST_IRON_ELEMENTAL, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_ELEMENTAL_WELLSPRING, 'E', LIGHTCYAN, "elemental wellspring",
    M_FAST_REGEN,
    MR_RES_POISON | MR_RES_ELEC,
    1500, 15, MONS_ELEMENTAL, MONS_ELEMENTAL_WELLSPRING, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 2, 0 },
    8, 8, MST_ELEMENTAL_WELLSPRING, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_WATER, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG
},

// frogs ('F')
{
    MONS_GIANT_FROG, 'F', GREEN, "giant frog",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    600, 10, MONS_GIANT_FROG, MONS_GIANT_FROG, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 5, 0 },
    0, 12, MST_NO_SPELLS, CE_CLEAN, S_CROAK,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 15, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

{
    MONS_SPINY_FROG, 'F', YELLOW, "spiny frog",
    M_COLD_BLOOD,
    MR_RES_POISON,
    700, 16, MONS_GIANT_FROG, MONS_SPINY_FROG, MH_NATURAL, 20,
    { {AT_STING, AF_POISON, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    6, 9, MST_NO_SPELLS, CE_POISONOUS, S_CROAK,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 12, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

{
    MONS_BLINK_FROG, 'F', LIGHTGREEN, "blink frog",
    M_COLD_BLOOD | M_PHASE_SHIFT,
    MR_NO_FLAGS,
    450, 13, MONS_BLINK_FROG, MONS_BLINK_FROG, MH_NATURAL, 40,
    { {AT_HIT, AF_BLINK, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    0, 16, MST_BLINKER, CE_CLEAN, S_CROAK,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 14, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

// eyes ('G')
{
    MONS_GIANT_EYEBALL, 'G', WHITE, "giant eyeball",
    M_NO_FLAGS,
    MR_RES_ASPHYX,
    550, 10, MONS_GIANT_EYEBALL, MONS_GIANT_EYEBALL, MH_NATURAL, 10,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    0, 1, MST_GIANT_EYEBALL, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 3, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

{
    MONS_EYE_OF_DRAINING, 'G', LIGHTGREY, "eye of draining",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_ASPHYX,
    550, 10, MONS_GIANT_EYEBALL, MONS_EYE_OF_DRAINING, MH_NATURAL, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    3, 1, MST_DRAIN_GAZE, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

{
    MONS_GIANT_ORANGE_BRAIN, 'G', LIGHTRED, "giant orange brain",
    M_WARM_BLOOD | M_SEE_INVIS,
    MR_RES_ASPHYX,
    550, 13, MONS_GIANT_ORANGE_BRAIN, MONS_GIANT_ORANGE_BRAIN, MH_NATURAL, 100,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    2, 4, MST_GIANT_ORANGE_BRAIN, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

{
    MONS_GREAT_ORB_OF_EYES, 'G', LIGHTGREEN, "great orb of eyes",
    M_SEE_INVIS,
    MR_RES_POISON,
    1300, 13, MONS_GIANT_EYEBALL, MONS_GREAT_ORB_OF_EYES, MH_NATURAL, MAG_IMMUNE,
    { {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 5, 0 },
    10, 3, MST_GREAT_ORB_OF_EYES, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_ORB
},

{
    MONS_SHINING_EYE, 'G', LIGHTMAGENTA, "shining eye",
    M_SEE_INVIS | M_GLOWS_RADIATION,
    MR_RES_ASPHYX,
    550, 14, MONS_GIANT_EYEBALL, MONS_SHINING_EYE, MH_NATURAL, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    3, 1, MST_SHINING_EYE, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

{
    MONS_EYE_OF_DEVASTATION, 'G', YELLOW, "eye of devastation",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_ASPHYX,
    550, 11, MONS_GIANT_EYEBALL, MONS_EYE_OF_DEVASTATION,
        MH_NATURAL, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    12, 1, MST_EYE_OF_DEVASTATION, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

{
    MONS_GOLDEN_EYE, 'G', ETC_GOLD, "golden eye",
    M_BATTY | M_GLOWS_LIGHT,
    MR_RES_ASPHYX,
    150, 17, MONS_GIANT_EYEBALL, MONS_GOLDEN_EYE, MH_NATURAL, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 1, 2, 0 },
    0, 20, MST_GOLDEN_EYE, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_ORB
},

{
    MONS_OPHAN, 'G', RED, "ophan",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_ASPHYX,
    550, 14, MONS_ANGEL, MONS_OPHAN, MH_HOLY, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 5, 0 },
    10, 10, MST_OPHAN, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_ORB
},

// hybrids ('H')
{
    MONS_HIPPOGRIFF, 'H', BROWN, "hippogriff",
    M_WARM_BLOOD | M_HYBRID,
    MR_NO_FLAGS,
    1150, 8, MONS_HIPPOGRIFF, MONS_HIPPOGRIFF, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 10}, {AT_PECK, AF_PLAIN, 8}, {AT_CLAW, AF_PLAIN, 8},
       AT_NO_ATK },
    { 7, 3, 5, 0 },
    2, 7, MST_NO_SPELLS, CE_CLEAN, S_SCREECH,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_MANTICORE, 'H', RED, "manticore",
    M_WARM_BLOOD | M_HYBRID,
    MR_NO_FLAGS,
    1200, 10, MONS_MANTICORE, MONS_MANTICORE, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 26}, {AT_CLAW, AF_PLAIN, 14}, {AT_CLAW, AF_PLAIN, 14},
       AT_NO_ATK },
    { 9, 3, 5, 0 },
    5, 7, MST_MANTICORE, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_MINOTAUR, 'H', LIGHTRED, "minotaur",
    M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    900, 10, MONS_MINOTAUR, MONS_MINOTAUR, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 35}, {AT_GORE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK },
    { 13, 3, 5, 0 },
    5, 7, MST_NO_SPELLS, CE_CLEAN, S_BELLOW,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GRIFFON, 'H', YELLOW, "griffon",
    M_WARM_BLOOD | M_HYBRID,
    MR_NO_FLAGS,
    1700, 5, MONS_GRIFFON, MONS_GRIFFON, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 18}, {AT_PECK, AF_PLAIN, 10},
      {AT_CLAW, AF_PLAIN, 10}, AT_NO_ATK },
    { 12, 3, 5, 0 },
    4, 6, MST_NO_SPELLS, CE_CLEAN, S_SCREECH,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SPHINX, 'H', LIGHTGREY, "sphinx",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    1800, 12, MONS_SPHINX, MONS_SPHINX, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 25}, {AT_HIT, AF_PLAIN, 12}, {AT_HIT, AF_PLAIN, 12},
       AT_NO_ATK },
    { 16, 3, 5, 0 },
    5, 5, MST_SPHINX, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_WINGED, 11, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_HARPY, 'H', GREEN, "harpy",
    M_WARM_BLOOD | M_BATTY | M_HYBRID | M_FEMALE,
    MR_RES_POISON,
    480, 9, MONS_HARPY, MONS_HARPY, MH_NATURAL, 20,
    { {AT_CLAW, AF_PLAIN, 19}, {AT_CLAW, AF_PLAIN, 14}, AT_NO_ATK,
       AT_NO_ATK },
    { 7, 3, 5, 0 },
    2, 10, MST_NO_SPELLS, CE_CLEAN, S_SCREECH,
    I_NORMAL, HT_LAND, FL_WINGED, 25, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID_WINGED
},

// Chimera - composed of three other animals
{
    MONS_CHIMERA, 'H', MAGENTA, "chimera",
    M_NO_POLY_TO | M_HYBRID,
    MR_NO_FLAGS,
    550, 11, MONS_CHIMERA, MONS_CHIMERA, MH_NATURAL, -3,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    8, 5, MST_NO_SPELLS, CE_NOCORPSE, S_RANDOM,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED
},

{
    MONS_ANUBIS_GUARD, 'H', CYAN, "anubis guard",
    M_WARM_BLOOD | M_FIGHTER | M_SEE_INVIS,
    mrd(MR_RES_NEG, 3) | MR_RES_TORMENT,
    550, 10, MONS_ANUBIS_GUARD, MONS_ANUBIS_GUARD, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 30}, {AT_BITE, AF_DRAIN_XP, 20},
      AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    3, 13, MST_ANUBIS_GUARD, CE_CLEAN, S_BARK,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// jellies ('J')
{
    MONS_OOZE, 'J', LIGHTGREY, "ooze",
    M_SEE_INVIS | M_UNBLINDABLE,
    MR_RES_POISON | MR_RES_ASPHYX | mrd(MR_RES_ACID, 3),
    300, 3, MONS_JELLY, MONS_OOZE, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    1, 3, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_BLOB
},

{
    MONS_JELLY, 'J', LIGHTRED, "jelly",
    M_SEE_INVIS | M_SPLITS | M_ACID_SPLASH,
    MR_RES_POISON | MR_RES_ASPHYX | mrd(MR_RES_ACID, 3),
    425, 15, MONS_JELLY, MONS_JELLY, MH_NATURAL, 10,
    { {AT_HIT, AF_ACID, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 4, 3, 0 },
    0, 2, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_SMALL, MON_SHAPE_BLOB
},

{
    MONS_SLIME_CREATURE, 'J', GREEN, "slime creature",
    M_FAST_REGEN | M_HERD,
    MR_RES_POISON | MR_RES_ASPHYX,
    425, 3, MONS_JELLY, MONS_SLIME_CREATURE, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 22}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    1, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_BLOB
},

DUMMY(MONS_MERGED_SLIME_CREATURE, 'J', LIGHTGREEN, "merged slime creature")

{
    MONS_AZURE_JELLY, 'J', LIGHTBLUE, "azure jelly",
    M_SEE_INVIS | M_UNBLINDABLE,
    MR_RES_POISON | MR_RES_ASPHYX | MR_RES_COLD | MR_VUL_FIRE | MR_RES_ELEC
        | mrd(MR_RES_ACID, 3),
    425, 14, MONS_JELLY, MONS_AZURE_JELLY, MH_NATURAL, 80,
    { {AT_HIT, AF_COLD, 12}, {AT_HIT, AF_COLD, 12}, {AT_HIT, AF_PLAIN, 12},
      {AT_HIT, AF_PLAIN, 12} },
    { 15, 3, 5, 0 },
    5, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_SMALL, MON_SHAPE_BLOB
},

{
    MONS_DEATH_OOZE, 'J', MAGENTA, "death ooze",
    M_SEE_INVIS,
    MR_RES_COLD | mrd(MR_RES_ACID, 3),
    300, 15, MONS_JELLY, MONS_DEATH_OOZE, MH_UNDEAD, 120,
    { {AT_HIT, AF_ROT, 32}, {AT_HIT, AF_PLAIN, 32}, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 3, 0 },
    2, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_LITTLE, MON_SHAPE_BLOB
},

{
    MONS_ACID_BLOB, 'J', LIGHTCYAN, "acid blob",
    M_SEE_INVIS | M_UNBLINDABLE | M_ACID_SPLASH,
    MR_RES_POISON | MR_RES_ASPHYX | mrd(MR_RES_ACID, 3),
    425, 12, MONS_JELLY, MONS_ACID_BLOB, MH_NATURAL, 160,
    { {AT_HIT, AF_ACID, 42}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 3, 5, 0 },
    1, 3, MST_ACID_SPIT, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_SMALL, MON_SHAPE_BLOB
},

// kobolds ('K')
{
    MONS_KOBOLD, 'K', BROWN, "kobold",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    400, 10, MONS_KOBOLD, MONS_KOBOLD, MH_NATURAL, 0,
    { {AT_HIT, AF_PLAIN, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 3, 0 },
    2, 12, MST_NO_SPELLS, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_BIG_KOBOLD, 'K', YELLOW, "big kobold",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    500, 7, MONS_KOBOLD, MONS_BIG_KOBOLD, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    3, 12, MST_NO_SPELLS, CE_POISONOUS, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_KOBOLD_DEMONOLOGIST, 'K', MAGENTA, "kobold demonologist",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    400, 18, MONS_KOBOLD, MONS_KOBOLD, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 4, 5, 0 },
    2, 13, MST_KOBOLD_DEMONOLOGIST, CE_POISONOUS, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

// liches ('L')
{
    MONS_LICH, 'L', LIGHTGREY, "lich",
    M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_COLD, 2),
    275, 18, MONS_LICH, MONS_LICH, MH_UNDEAD, MAG_IMMUNE,
    { {AT_TOUCH, AF_DRAIN_XP, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 2, 4, 0 },
    10, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ANCIENT_LICH, 'L', WHITE, "ancient lich",
    M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_COLD, 2) | MR_RES_FIRE | MR_RES_ELEC,
    275, 24, MONS_LICH, MONS_LICH, MH_UNDEAD, MAG_IMMUNE,
    { {AT_TOUCH, AF_DRAIN_XP, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 27, 2, 4, 0 },
    20, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_REVENANT, 'L', CYAN, "revenant",
    M_SEE_INVIS | M_SPEAKS,
    mrd(MR_RES_COLD, 2),
    275, 18, MONS_REVENANT, MONS_REVENANT, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 3, 3, 0 },
    8, 12, MST_REVENANT, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_UNBORN, 'L', BROWN, "unborn",
    M_SPEAKS,
    MR_RES_COLD,
    550, 16, MONS_UNBORN, MONS_UNBORN, MH_UNDEAD, 120,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 5, 4, 0 },
    2, 10, MST_UNBORN, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// mummies ('M')
{
    MONS_MUMMY, 'M', LIGHTGREY, "mummy",
    M_NO_FLAGS,
    MR_VUL_FIRE | MR_RES_COLD,
    550, 21, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, 20,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 5, 3, 0 },
    3, 6, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GUARDIAN_MUMMY, 'M', YELLOW, "guardian mummy",
    M_FIGHTER | M_SEE_INVIS,
    MR_RES_COLD,
    1300, 13, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 5, 3, 0 },
    6, 9, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_GREATER_MUMMY, 'M', WHITE, "greater mummy",
    M_SEE_INVIS | M_SPEAKS,
    MR_RES_COLD | MR_RES_ELEC,
    550, 24, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 5, 3, 100 },
    10, 6, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MUMMY_PRIEST, 'M', RED, "mummy priest",
    M_SEE_INVIS | M_SPEAKS,
    MR_RES_COLD | MR_RES_ELEC,
    550, 20, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 5, 3, 0 },
    8, 7, MST_MUMMY_PRIEST, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// nagas ('N')
{
    MONS_NAGA, 'N', GREEN, "naga",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    1000, 13, MONS_NAGA, MONS_NAGA, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 17}, {AT_CONSTRICT, AF_CRUSH, 3},
       AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    6, 10, MST_NAGA, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

{
    MONS_NAGA_MAGE, 'N', MAGENTA, "naga mage",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    1000, 14, MONS_NAGA, MONS_NAGA, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 17}, {AT_CONSTRICT, AF_CRUSH, 4},
       AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    6, 10, MST_NAGA_MAGE, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

{
    MONS_NAGA_SHARPSHOOTER, 'N', LIGHTGRAY, "naga sharpshooter",
    M_SEE_INVIS | M_WARM_BLOOD
        | M_ARCHER | M_DONT_MELEE | M_SPEAKS,
    MR_RES_POISON,
    1000, 14, MONS_NAGA, MONS_NAGA, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 17}, {AT_CONSTRICT, AF_CRUSH, 5}, AT_NO_ATK, AT_NO_ATK },
    { 9, 6, 4, 0 },
    6, 10, MST_NAGA_SNIPER, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

{
    MONS_NAGA_RITUALIST, 'N', BROWN, "naga ritualist",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    1000, 14, MONS_NAGA, MONS_NAGA, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 14}, {AT_CONSTRICT, AF_CRUSH, 4},
       AT_NO_ATK, AT_NO_ATK },
    { 8, 5, 5, 0 },
    6, 10, MST_NAGA_RITUALIST, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

{
    MONS_NAGA_WARRIOR, 'N', BLUE, "naga warrior",
    M_FIGHTER | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    1000, 11, MONS_NAGA, MONS_NAGA, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 28}, {AT_CONSTRICT, AF_CRUSH, 6},
       AT_NO_ATK, AT_NO_ATK },
    { 10, 9, 2, 0 },
    6, 10, MST_NAGA, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

{
    MONS_GREATER_NAGA, 'N', LIGHTMAGENTA, "greater naga",
    M_FIGHTER | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    1000, 15, MONS_NAGA, MONS_NAGA, MH_NATURAL, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 27}, {AT_CONSTRICT, AF_CRUSH, 7},
       AT_NO_ATK, AT_NO_ATK },
    { 15, 3, 5, 0 },
    6, 10, MST_NAGA_MAGE, CE_POISONOUS, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

// ogres ('O')
{
    MONS_OGRE, 'O', BROWN, "ogre",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    1300, 9, MONS_OGRE, MONS_OGRE, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    1, 6, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// These guys get understated because the experience code can't see
// that they wield two weapons... I'm raising their xp modifier. - bwr
{
    MONS_TWO_HEADED_OGRE, 'O', LIGHTRED, "two-headed ogre",
    M_WARM_BLOOD | M_TWO_WEAPONS | M_SPEAKS,
    MR_NO_FLAGS,
    1390, 15, MONS_OGRE, MONS_TWO_HEADED_OGRE, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 17}, {AT_HIT, AF_PLAIN, 13}, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    1, 4, MST_NO_SPELLS, CE_CLEAN, S_SHOUT2,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_OGRE_MAGE, 'O', MAGENTA, "ogre mage",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    1300, 16, MONS_OGRE, MONS_OGRE, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    1, 7, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// immobile plants and fungi ('P')
{
    MONS_PLANT, 'P', GREEN, "plant",
    M_STATIONARY | M_NO_EXP_GAIN,
    MR_RES_POISON,
    425, 10, MONS_PLANT, MONS_PLANT, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    10, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_PLANT
},

{
    MONS_OKLOB_SAPLING, 'P', LIGHTCYAN, "oklob sapling",
    M_STATIONARY,
    MR_RES_POISON | mrd(MR_RES_ACID, 3),
    425, 10, MONS_PLANT, MONS_OKLOB_PLANT, MH_PLANT, 20,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 3, 5, 0 },
    10, 0, MST_OKLOB_SAPLING, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_PLANT
},

{
    MONS_OKLOB_PLANT, 'P', LIGHTGREEN, "oklob plant",
    M_STATIONARY,
    MR_RES_POISON | mrd(MR_RES_ACID, 3),
    425, 10, MONS_PLANT, MONS_OKLOB_PLANT, MH_PLANT, 40,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    10, 0, MST_ACID_SPIT, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_PLANT
},

{
    MONS_BUSH, 'P', BROWN, "bush",
    M_STATIONARY | M_NO_EXP_GAIN,
    MR_RES_POISON | MR_VUL_FIRE,
    1500, 10, MONS_PLANT, MONS_BUSH, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 3, 5, 0 },
    15, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_PLANT
},

{
    MONS_BURNING_BUSH, 'P', RED, "burning bush",
    M_STATIONARY | M_SEE_INVIS,
    MR_RES_POISON | MR_RES_FIRE,
    1500, 10, MONS_PLANT, MONS_BUSH, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    10, 0, MST_BURNING_BUSH, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_PLANT
},

{
    MONS_TOADSTOOL, 'P', COLOUR_UNDEF, "toadstool",
    M_NO_EXP_GAIN | M_STATIONARY,
    MR_RES_POISON,
    150, 10, MONS_FUNGUS, MONS_TOADSTOOL, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 2, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_FUNGUS, 'P', LIGHTGREY, "fungus",
    M_NO_EXP_GAIN | M_STATIONARY,
    MR_RES_POISON,
    150, 10, MONS_FUNGUS, MONS_FUNGUS, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_BALLISTOMYCETE, 'P', MAGENTA, "ballistomycete",
    M_NO_EXP_GAIN | M_STATIONARY,
    MR_RES_POISON,
    150, 10, MONS_FUNGUS, MONS_BALLISTOMYCETE, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 5, 3, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_HYPERACTIVE_BALLISTOMYCETE, 'P', LIGHTRED, "hyperactive ballistomycete",
    M_STATIONARY | M_NO_POLY_TO,
    MR_RES_POISON,
    150, 10, MONS_FUNGUS, MONS_BALLISTOMYCETE, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    // expected explosion damage: 25, expected HP: 60
    { 6, 5, 10, 0 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_FUNGUS
},

{
    MONS_BRIAR_PATCH, 'P', YELLOW, "briar patch",
    M_STATIONARY | M_NO_EXP_GAIN,
    MR_RES_POISON | MR_VUL_FIRE,
    425, 10, MONS_PLANT, MONS_BRIAR_PATCH, MH_PLANT, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 1, 1, 0 },
    10, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_AMPHIBIOUS, FL_NONE, 0, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_PLANT
},

// tengu ('Q')
{
    MONS_TENGU, 'Q', LIGHTBLUE, "tengu",
    M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    550, 10, MONS_TENGU, MONS_TENGU, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, {AT_PECK, AF_PLAIN, 5}, {AT_CLAW, AF_PLAIN, 5},
       AT_NO_ATK },
    { 5, 3, 3, 0 },
    2, 12, MST_NO_SPELLS, CE_CLEAN /*chicken*/, S_SHOUT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(9),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_TENGU_CONJURER, 'Q', BLUE, "tengu conjurer",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    550, 13, MONS_TENGU, MONS_TENGU, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, {AT_PECK, AF_PLAIN, 5}, {AT_CLAW, AF_PLAIN, 5},
       AT_NO_ATK },
    { 7, 3, 3, 0 },
    2, 17, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(9),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_TENGU_WARRIOR, 'Q', CYAN, "tengu warrior",
    M_FIGHTER | M_ARCHER | M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    550, 13, MONS_TENGU, MONS_TENGU, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 16}, {AT_PECK, AF_PLAIN, 8}, {AT_CLAW, AF_PLAIN, 8},
       AT_NO_ATK },
    { 10, 4, 4, 0 },
    2, 17, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(9),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_TENGU_REAVER, 'Q', LIGHTMAGENTA, "tengu reaver",
    M_FIGHTER | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_HYBRID,
    MR_NO_FLAGS,
    550, 13, MONS_TENGU, MONS_TENGU, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 27}, {AT_PECK, AF_PLAIN, 11}, {AT_CLAW, AF_PLAIN, 11},
       AT_NO_ATK },
    { 17, 3, 4, 0 },
    2, 17, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(9),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// spiritual beings ('R')
{
    MONS_EFREET, 'R', RED, "efreet",
    M_SPEAKS | M_GLOWS_LIGHT,
    MR_RES_POISON | mrd(MR_RES_FIRE, 3) | MR_VUL_COLD | MR_VUL_WATER,
    1300, 12, MONS_EFREET, MONS_EFREET, MH_DEMONIC, 20,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    10, 5, MST_EFREET, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_RAKSHASA, 'R', YELLOW, "rakshasa",
    M_SEE_INVIS,
    MR_RES_POISON,
    550, 15, MONS_RAKSHASA, MONS_RAKSHASA, MH_DEMONIC, 140,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 6, 0 },
    6, 14, MST_RAKSHASA, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DRYAD, 'R', LIGHTGREEN, "dryad",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_VUL_FIRE,
    500, 10, MONS_DRYAD, MONS_DRYAD, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 4, 5, 0 },
    6, 12, MST_DRYAD, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SNAPLASHER_VINE, 'w', LIGHTGREEN, "snaplasher vine",
    M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO,
    MR_RES_POISON,
    1300, 10, MONS_PLANT, MONS_SNAPLASHER_VINE, MH_PLANT, 40,
    { {AT_CONSTRICT, AF_CRUSH, 0}, {AT_HIT, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK },
    { 12, 1, 2, 0 },
    4, 7, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_SNAPLASHER_VINE_SEGMENT, '*', LIGHTGREEN, "snaplasher vine segment",
    M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO,
    MR_RES_POISON,
    1300, 10, MONS_PLANT, MONS_SNAPLASHER_VINE, MH_PLANT, 40,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 2, 2, 0 },
    6, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_GRAND_AVATAR, 'R', MAGENTA, "grand avatar",
    M_SEE_INVIS | M_GLOWS_LIGHT | M_INSUBSTANTIAL | M_NO_REGEN
        | M_NO_POLY_TO | M_NO_EXP_GAIN | M_AVATAR,
    mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC | MR_RES_FIRE, 2)
        | MR_RES_ACID | MR_RES_STICKY_FLAME,
    0, 10, MONS_GRAND_AVATAR, MONS_GRAND_AVATAR, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_REACH, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 0, 0, 50},
    20, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// snakes ('S')
DUMMY(MONS_SNAKE, 'S', LIGHTGREEN, "snake")

{
    MONS_BALL_PYTHON, 'S', GREEN, "ball python",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    100, 13, MONS_SNAKE, MONS_BALL_PYTHON, MH_NATURAL, 0,
    { {AT_BITE, AF_PLAIN, 2}, {AT_CONSTRICT, AF_CRUSH, 1},
       AT_NO_ATK, AT_NO_ATK },
    { 1, 2, 3, 0 },
    0, 11, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 12, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

{
    MONS_ADDER, 'S', LIGHTGREEN, "adder",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    200, 10, MONS_SNAKE, MONS_ADDER, MH_NATURAL, 10,
    { {AT_BITE, AF_POISON, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    1, 15, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 13, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_SNAKE
},

{
    MONS_WATER_MOCCASIN, 'S', BROWN, "water moccasin",
    M_COLD_BLOOD,
    MR_RES_POISON,
    300, 11, MONS_SNAKE, MONS_WATER_MOCCASIN, MH_NATURAL, 20,
    { {AT_BITE, AF_POISON, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    2, 15, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 14, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_SNAKE
},

{
    MONS_BLACK_MAMBA, 'S', BLUE, "black mamba",
    M_COLD_BLOOD,
    MR_RES_POISON,
    500, 12, MONS_SNAKE, MONS_BLACK_MAMBA, MH_NATURAL, 20,
    { {AT_BITE, AF_POISON, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    4, 15, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_REPTILE, HT_LAND, FL_NONE, 18, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

{
    MONS_SEA_SNAKE, 'S', LIGHTCYAN, "sea snake",
    M_COLD_BLOOD | M_SUBMERGES,
    MR_NO_FLAGS,
    400, 10, MONS_SNAKE, MONS_SEA_SNAKE, MH_NATURAL, 40,
    { {AT_BITE, AF_POISON_STRONG, 24}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 4, 5, 0 },
    2, 15, MST_NO_SPELLS, CE_POISONOUS, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 12, SWIM_ENERGY(4),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

{
    MONS_ANACONDA, 'S', LIGHTGREY, "anaconda",
    M_COLD_BLOOD,
    MR_NO_FLAGS,
    750, 10, MONS_SNAKE, MONS_ANACONDA, MH_NATURAL, 40,
    { {AT_CONSTRICT, AF_CRUSH, 6}, {AT_BITE, AF_PLAIN, 20},
       AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    4, 16, MST_NO_SPELLS, CE_CLEAN, S_HISS,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 18, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_GUARDIAN_SERPENT, 'S', WHITE, "guardian serpent",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS,
    MR_RES_POISON,
    800, 10, MONS_GUARDIAN_SERPENT, MONS_GUARDIAN_SERPENT, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    6, 14, MST_GUARDIAN_SERPENT, CE_MUTAGEN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_SHOCK_SERPENT, 'S', LIGHTBLUE, "shock serpent",
    M_COLD_BLOOD,
    mrd(MR_RES_ELEC, 3),
    400, 12, MONS_SNAKE, MONS_SHOCK_SERPENT, MH_NATURAL, 40,
    { {AT_BITE, AF_ELEC, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    2, 15, MST_SHOCK_SERPENT, CE_CLEAN, S_HISS,
    I_REPTILE, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

{
    MONS_MANA_VIPER, 'S', MAGENTA, "mana viper",
    M_COLD_BLOOD | M_SEE_INVIS | M_UNBLINDABLE,
    MR_RES_POISON,
    550, 10, MONS_SNAKE, MONS_MANA_VIPER, MH_NATURAL, MAG_IMMUNE,
    { {AT_BITE, AF_ANTIMAGIC, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    3, 14, MST_NO_SPELLS, CE_NOCORPSE, S_HISS,
    I_REPTILE, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_SNAKE
},

// trolls ('T')
{
    MONS_TROLL, 'T', BROWN, "troll",
    M_WARM_BLOOD | M_SPEAKS | M_FAST_REGEN,
    MR_NO_FLAGS,
    1500, 10, MONS_TROLL, MONS_TROLL, MH_NATURAL, 20,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 15},
      {AT_CLAW, AF_PLAIN, 15}, AT_NO_ATK },
    { 7, 3, 5, 0 },
    3, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_IRON_TROLL, 'T', CYAN, "iron troll",
    M_WARM_BLOOD | M_SPEAKS | M_FAST_REGEN,
    MR_RES_FIRE | MR_RES_COLD,
    1800, 10, MONS_TROLL, MONS_IRON_TROLL, MH_NATURAL, 100,
    { {AT_BITE, AF_PLAIN, 35}, {AT_CLAW, AF_PLAIN, 25},
      {AT_CLAW, AF_PLAIN, 25}, AT_NO_ATK },
    { 16, 3, 5, 0 },
    20, 4, MST_NO_SPELLS, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_TROLL, 'T', YELLOW, "deep troll",
    M_WARM_BLOOD | M_SEE_INVIS | M_UNBLINDABLE | M_SPEAKS | M_FAST_REGEN,
    MR_NO_FLAGS,
    1500, 9, MONS_TROLL, MONS_DEEP_TROLL, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 27}, {AT_CLAW, AF_PLAIN, 20},
      {AT_CLAW, AF_PLAIN, 20}, AT_NO_ATK },
    { 10, 3, 5, 0 },
    6, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_TROLL_EARTH_MAGE, 'T', MAGENTA, "deep troll earth mage",
    M_WARM_BLOOD | M_SEE_INVIS | M_UNBLINDABLE | M_SPEAKS,
    MR_NO_FLAGS,
    1500, 12, MONS_TROLL, MONS_DEEP_TROLL, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 27}, {AT_CLAW, AF_PLAIN, 20},
      {AT_CLAW, AF_PLAIN, 20}, AT_NO_ATK },
    { 12, 2, 4, 0 },
    // the extra AC is essentially a perma-stoneskin
    12, 10, MST_DEEP_TROLL_EARTH_MAGE, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_DEEP_TROLL_SHAMAN, 'T', WHITE, "deep troll shaman",
    M_WARM_BLOOD | M_SEE_INVIS | M_UNBLINDABLE | M_SPEAKS,
    MR_NO_FLAGS,
    1500, 12, MONS_TROLL, MONS_DEEP_TROLL, MH_NATURAL, 40,
    { {AT_BITE, AF_PLAIN, 27}, {AT_CLAW, AF_PLAIN, 20},
      {AT_CLAW, AF_PLAIN, 20}, AT_NO_ATK },
    { 12, 2, 4, 0 },
    6, 10, MST_DEEP_TROLL_SHAMAN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// vampires ('V')
{
    MONS_VAMPIRE, 'V', RED, "vampire",
    M_SEE_INVIS | M_SPEAKS | M_BLOOD_SCENT | M_WARM_BLOOD,
    MR_RES_COLD,
    550, 11, MONS_VAMPIRE, MONS_VAMPIRE, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 15}, {AT_BITE, AF_VAMPIRIC, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 6, 3, 5, 0 },
    10, 10, MST_VAMPIRE, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_VAMPIRE_KNIGHT, 'V', CYAN, "vampire knight",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS
        | M_BLOOD_SCENT | M_WARM_BLOOD,
    MR_RES_COLD,
    550, 16, MONS_VAMPIRE, MONS_VAMPIRE, MH_UNDEAD, 80,
    { {AT_HIT, AF_PLAIN, 33}, {AT_BITE, AF_VAMPIRIC, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 11, 3, 7, 0 },
    10, 10, MST_VAMPIRE_KNIGHT, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_VAMPIRE_MAGE, 'V', MAGENTA, "vampire mage",
    M_SEE_INVIS | M_SPEAKS | M_BLOOD_SCENT | M_WARM_BLOOD,
    MR_RES_COLD,
    550, 15, MONS_VAMPIRE, MONS_VAMPIRE, MH_UNDEAD, 80,
    { {AT_HIT, AF_PLAIN, 15}, {AT_BITE, AF_VAMPIRIC, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 10, 3, 5, 0 },
    10, 10, MST_VAMPIRE_MAGE, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_JIANGSHI, 'V', YELLOW, "jiangshi",
    M_SEE_INVIS | M_BLOOD_SCENT | M_FIGHTER,
    MR_RES_COLD,
    550, 10, MONS_JIANGSHI, MONS_VAMPIRE, MH_UNDEAD, 80,
    { {AT_CLAW, AF_VAMPIRIC, 27}, {AT_CLAW, AF_VAMPIRIC, 27}, AT_NO_ATK,
       AT_NO_ATK },
    { 10, 4, 5, 0 },
    10, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 18, MOVE_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// incorporeal undead ('W')
{ // dummy genus monster, but also used for removed monsters
    MONS_GHOST, 'W', WHITE, "ghost",
    M_INSUBSTANTIAL | M_NO_POLY_TO,
    MR_NO_FLAGS,
    0, 0, MONS_GHOST, MONS_GHOST, MH_UNDEAD, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// XP modifier is 5 for these, because they really aren't all that
// dangerous, but still come out at 200+ XP
{
    MONS_PHANTOM, 'W', BLUE, "phantom",
    M_INSUBSTANTIAL,
    mrd(MR_RES_COLD, 2),
    0, 5, MONS_GHOST, MONS_PHANTOM, MH_UNDEAD, 40,
    { {AT_HIT, AF_BLINK, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    3, 13, MST_BLINKER, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_HUNGRY_GHOST, 'W', GREEN, "hungry ghost",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_SPEAKS,
    mrd(MR_RES_COLD, 2),
    0, 8, MONS_GHOST, MONS_HUNGRY_GHOST, MH_UNDEAD, 40,
    { {AT_HIT, AF_HUNGER, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    0, 17, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_CORPSES, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FLAYED_GHOST, 'W', RED, "flayed ghost",
    M_INSUBSTANTIAL | M_SPEAKS,
    MR_NO_FLAGS,
    0, 10, MONS_GHOST, MONS_FLAYED_GHOST, MH_UNDEAD, 60,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    0, 14, MST_FLAYED_GHOST, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// player ghost - stats are stored in ghost struct
{
    MONS_PLAYER_GHOST, 'W', WHITE, "player ghost",
    M_FIGHTER | M_SPEAKS | M_INSUBSTANTIAL | M_NO_POLY_TO,
    MR_NO_FLAGS,
    0, 15, MONS_GHOST, MONS_PLAYER_GHOST, MH_UNDEAD, -5,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 2, 3, 0 },
    1, 2, MST_GHOST, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},


{
    MONS_SHADOW, 'W', MAGENTA, "shadow",
    M_SHADOW | M_SEE_INVIS | M_INSUBSTANTIAL,
    mrd(MR_RES_COLD, 3),
    0, 18, MONS_WRAITH, MONS_SHADOW, MH_UNDEAD, 40,
    { {AT_HIT, AF_SHADOWSTAB, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 2, 4, 0 },
    7, 10, MST_SHADOW, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_SILENT_SPECTRE, 'W', CYAN, "silent spectre",
    M_SPEAKS /* uh... */ | M_SEE_INVIS | M_INSUBSTANTIAL,
    mrd(MR_RES_COLD, 3),
    0, 10, MONS_WRAITH, MONS_SILENT_SPECTRE, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    5, 15, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DROWNED_SOUL, 'W', LIGHTCYAN, "drowned soul",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_SPEAKS,
    mrd(MR_RES_COLD, 3),
    0, 8, MONS_GHOST, MONS_DROWNED_SOUL, MH_UNDEAD, 60,
    { {AT_TOUCH, AF_DROWN, 0}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 1, 1, 0 },
    0, 13, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 12, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_WRAITH, 'W', LIGHTGREY, "wraith",
    M_SEE_INVIS | M_INSUBSTANTIAL,
    MR_RES_COLD,
    0, 14, MONS_WRAITH, MONS_WRAITH, MH_UNDEAD, 60,
    { {AT_HIT, AF_DRAIN_SPEED, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    10, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SHADOW_WRAITH, 'W', LIGHTMAGENTA, "shadow wraith",
    M_SEE_INVIS | M_INVIS | M_INSUBSTANTIAL,
    MR_NO_FLAGS,
    0, 15, MONS_WRAITH, MONS_SHADOW_WRAITH, MH_UNDEAD, 100,
    { {AT_HIT, AF_DRAIN_SPEED, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    7, 7, MST_NO_SPELLS, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FREEZING_WRAITH, 'W', LIGHTBLUE, "freezing wraith",
    M_SEE_INVIS | M_INSUBSTANTIAL,
    MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    0, 10, MONS_WRAITH, MONS_FREEZING_WRAITH, MH_UNDEAD, 40,
    { {AT_HIT, AF_COLD, 16}, {AT_HIT, AF_DRAIN_SPEED, 13}, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    12, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_EIDOLON, 'W', BROWN, "eidolon",
    M_SEE_INVIS | M_GLOWS_LIGHT | M_INSUBSTANTIAL,
    MR_RES_COLD,
    0, 14, MONS_WRAITH, MONS_EIDOLON, MH_UNDEAD, 140,
    { {AT_HIT, AF_DRAIN_SPEED, 27}, {AT_HIT, AF_DRAIN_STAT, 17}, AT_NO_ATK,
       AT_NO_ATK },
    { 13, 3, 5, 0 },
    12, 10, MST_EIDOLON, CE_NOCORPSE, S_MOAN,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_PHANTASMAL_WARRIOR, 'W', LIGHTGREEN, "phantasmal warrior",
    M_FIGHTER | M_SEE_INVIS | M_SHADOW | M_INSUBSTANTIAL,
    MR_RES_COLD,
    0, 13, MONS_WRAITH, MONS_PHANTASMAL_WARRIOR, MH_UNDEAD, 80,
    { {AT_HIT, AF_VULN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    12, 10, MST_PHANTASMAL_WARRIOR, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// large abominations ('X')
{
    // See comment under MONS_ABOMINATION_SMALL regarding holiness.
    MONS_ABOMINATION_LARGE, 'X', LIGHTRED, "large abomination",
    M_NO_REGEN,
    MR_NO_FLAGS,
    1500, 10, MONS_ABOMINATION_SMALL, MONS_ABOMINATION_LARGE, MH_UNDEAD, 100,
    { {AT_HIT, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 2, 5, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_THRASHING_HORROR, 'X', YELLOW, "thrashing horror",
    M_BATTY | M_NO_POLY_TO,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    1500, 10, MONS_THRASHING_HORROR, MONS_THRASHING_HORROR, MH_NONLIVING, 60,
    { {AT_TRAMPLE, AF_PLAIN, 17}, {AT_TRAMPLE, AF_PLAIN, 9},
       AT_NO_ATK, AT_NO_ATK },
    { 9, 3, 5, 0 },
    5, 10, MST_THRASHING_HORROR, CE_NOCORPSE, S_ROAR,
    I_ANIMAL, HT_LAND, FL_NONE, 25, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

{
    MONS_TENTACLED_MONSTROSITY, 'X', GREEN, "tentacled monstrosity",
    M_SEE_INVIS,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    1800, 10, MONS_TENTACLED_MONSTROSITY, MONS_TENTACLED_MONSTROSITY,
        MH_DEMONIC, 160,
    { {AT_TENTACLE_SLAP, AF_PLAIN, 22}, {AT_TENTACLE_SLAP, AF_PLAIN, 17},
      {AT_TENTACLE_SLAP, AF_PLAIN, 13}, {AT_CONSTRICT, AF_CRUSH, 9} },
    { 23, 3, 5, 0 },
    5, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

{
    MONS_ORB_GUARDIAN, 'X', MAGENTA, "Orb Guardian",
    M_FIGHTER | M_SEE_INVIS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    1800, 20, MONS_ORB_GUARDIAN, MONS_ORB_GUARDIAN, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 3, 5, 0 },
    13, 13, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

{
    MONS_TENTACLED_STARSPAWN, 'X', LIGHTCYAN, "tentacled starspawn",
    M_SEE_INVIS,
    mrd(MR_RES_POISON, 3),
    1800, 20, MONS_TENTACLED_STARSPAWN, MONS_TENTACLED_STARSPAWN, MH_NONLIVING, 120,
    { {AT_BITE, AF_PLAIN, 40}, {AT_ENGULF, AF_PLAIN, 25}, AT_NO_ATK,
       AT_NO_ATK },
    { 16, 3, 5, 0 },
    5, 5, MST_TENTACLED_STARSPAWN, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

{
    MONS_STARSPAWN_TENTACLE, 'w', LIGHTCYAN, "starspawn tentacle",
    M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO,
    mrd(MR_RES_POISON, 3),
    1300, 10, MONS_TENTACLED_STARSPAWN, MONS_STARSPAWN_TENTACLE,
        MH_NONLIVING, MAG_IMMUNE,
    { {AT_CONSTRICT, AF_CRUSH, 3}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 22, 0, 1, 10 },
    8, 2, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_STARSPAWN_TENTACLE_SEGMENT, '*', LIGHTCYAN, "starspawn tentacle segment",
    M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO,
    mrd(MR_RES_POISON, 3),
    1300, 10, MONS_TENTACLED_STARSPAWN, MONS_STARSPAWN_TENTACLE_SEGMENT,
        MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 22, 0, 1, 10 },
    8, 2, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_STARCURSED_MASS, 'X', BLUE, "starcursed mass",
    M_SEE_INVIS | M_FAST_REGEN,
    mrd(MR_RES_POISON, 3),
    1800, 12, MONS_STARCURSED_MASS, MONS_STARCURSED_MASS, MH_NONLIVING, 100,
    { {AT_ENGULF, AF_PLAIN, 16}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 9, 7, 0 },
    10, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

// yaks, sheep, elephants, and other beasts ('Y')
{
    MONS_SHEEP, 'Y', LIGHTGREY, "sheep",
    M_WARM_BLOOD | M_HERD,
    MR_NO_FLAGS,
    900, 10, MONS_SHEEP, MONS_SHEEP, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    2, 7, MST_NO_SPELLS, CE_CLEAN, S_BELLOW,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

{
    MONS_YAK, 'Y', BROWN, "yak",
    M_WARM_BLOOD | M_HERD,
    MR_NO_FLAGS,
    1200, 9, MONS_YAK, MONS_YAK, MH_NATURAL, 20,
    { {AT_GORE, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    4, 7, MST_NO_SPELLS, CE_CLEAN, S_BELLOW,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_DEATH_YAK, 'Y', YELLOW, "death yak",
    M_WARM_BLOOD | M_HERD,
    MR_NO_FLAGS,
    1500, 8, MONS_YAK, MONS_DEATH_YAK, MH_NATURAL, 100,
    { {AT_GORE, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    9, 5, MST_NO_SPELLS, CE_POISONOUS, S_BELLOW,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED
},

{
    MONS_CATOBLEPAS, 'Y', MAGENTA, "catoblepas",
    M_WARM_BLOOD,
    MR_RES_PETRIFY,
    1400, 8, MONS_YAK, MONS_CATOBLEPAS, MH_NATURAL, 100,
    { {AT_GORE, AF_PLAIN, 36}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    10, 2, MST_CATOBLEPAS, CE_CLEAN, S_BELLOW,
    I_ANIMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED
},

{
    MONS_ELEPHANT, 'Y', GREEN, "elephant",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    1600, 9, MONS_ELEPHANT, MONS_ELEPHANT, MH_NATURAL, 60,
    { {AT_TRAMPLE, AF_PLAIN, 20}, {AT_TRUNK_SLAP, AF_PLAIN, 5}, AT_NO_ATK,
       AT_NO_ATK },
    { 9, 5, 5, 0 },
    8, 2, MST_NO_SPELLS, CE_CLEAN, S_TRUMPET,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

{
    MONS_DIRE_ELEPHANT, 'Y', BLUE, "dire elephant",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    2200, 8, MONS_ELEPHANT, MONS_DIRE_ELEPHANT, MH_NATURAL, 100,
    { {AT_TRAMPLE, AF_PLAIN, 40}, {AT_TRUNK_SLAP, AF_PLAIN, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 15, 5, 5, 0 },
    13, 2, MST_NO_SPELLS, CE_CLEAN, S_TRUMPET,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

{
    MONS_HELLEPHANT, 'Y', LIGHTRED, "hellephant",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    2800, 10, MONS_ELEPHANT, MONS_HELLEPHANT, MH_DEMONIC, 140,
    { {AT_TRAMPLE, AF_PLAIN, 45}, {AT_BITE, AF_PLAIN, 20},
      {AT_GORE, AF_PLAIN, 15 }, AT_NO_ATK },
    { 20, 5, 7, 0 },
    13, 10, MST_HELLEPHANT, CE_CLEAN, S_TRUMPET,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

{
    MONS_APIS, 'Y', WHITE, "apis",
    M_WARM_BLOOD,
    MR_NO_FLAGS,
    1800, 8, MONS_APIS, MONS_APIS, MH_HOLY, 100,
    { {AT_GORE, AF_HOLY, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 5, 5, 0 },
    9, 5, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_QUADRUPED
},

{
    MONS_ICE_BEAST, 'Y', LIGHTBLUE, "ice beast",
    M_NO_FLAGS,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    1300, 13, MONS_ICE_BEAST, MONS_ICE_BEAST, MH_NATURAL, 20,
    { {AT_HIT, AF_COLD, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    5, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_SKY_BEAST, 'Y', LIGHTCYAN, "sky beast",
    M_NO_SKELETON,
    MR_RES_ASPHYX | mrd(MR_RES_ELEC, 3),
    480, 13, MONS_SKY_BEAST, MONS_SKY_BEAST, MH_NATURAL, 20,
    { {AT_HIT, AF_ELEC, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    3, 13, MST_NO_SPELLS, CE_MUTAGEN, S_SILENT,
    I_ANIMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED_WINGED
},

// zombies and derived undead ('Z')
// zombie, skeleton and simulacra species depend on corpse species,
// or else are chosen randomly
{
    MONS_ZOMBIE, 'Z', BROWN, "zombie",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    425, 9, MONS_ZOMBIE, MONS_ZOMBIE, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 5, 5, 0 },
    0, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_SKELETON, 'Z', LIGHTGREY, "skeleton",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    0, 9, MONS_SKELETON, MONS_SKELETON, MH_UNDEAD, -1,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 0, 0, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_SIMULACRUM, 'Z', LIGHTBLUE, "simulacrum",
    M_NO_REGEN,
    MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    0, 9, MONS_SIMULACRUM, MONS_SIMULACRUM, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    10, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_ZOMBIE_SMALL, 'Z', BROWN, "small zombie",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    0, 9, MONS_ZOMBIE, MONS_ZOMBIE, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 5, 5, 0 },
    0, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_SKELETON_SMALL, 'Z', LIGHTGREY, "small skeleton",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    0, 9, MONS_SKELETON, MONS_SKELETON, MH_UNDEAD, -1,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 0, 0, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_SIMULACRUM_SMALL, 'Z', LIGHTBLUE, "small simulacrum",
    M_NO_REGEN,
    MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    0, 9, MONS_SIMULACRUM, MONS_SIMULACRUM, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    10, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

{
    MONS_ZOMBIE_LARGE, 'Z', YELLOW, "large zombie",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    0, 9, MONS_ZOMBIE, MONS_ZOMBIE, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 23}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    8, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_SKELETON_LARGE, 'Z', WHITE, "large skeleton",
    M_NO_REGEN,
    mrd(MR_RES_COLD, 2),
    0, 9, MONS_SKELETON, MONS_SKELETON, MH_UNDEAD, -1,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 0, 0, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_SIMULACRUM_LARGE, 'Z', LIGHTCYAN, "large simulacrum",
    M_NO_REGEN,
    MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    0, 9, MONS_SIMULACRUM, MONS_SIMULACRUM, MH_UNDEAD, -1,
    { {AT_HIT, AF_PLAIN, 14}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 5, 0 },
    10, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

// spectral thing - similar to zombies/skeletons
{
    MONS_SPECTRAL_THING, 'Z', GREEN, "spectral thing",
    M_SEE_INVIS | M_GLOWS_LIGHT | M_INSUBSTANTIAL,
    MR_RES_COLD,
    0, 11, MONS_WRAITH, MONS_SPECTRAL_THING, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    8, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 7, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

// water monsters
{
    MONS_ELECTRIC_EEL, ';', LIGHTBLUE, "electric eel",
    M_NO_GEN_DERIVED | M_COLD_BLOOD | M_SUBMERGES,
    mrd(MR_RES_ELEC, 2),
    300, 19, MONS_ELECTRIC_EEL, MONS_ELECTRIC_EEL, MH_NATURAL, 10,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    1, 15, MST_ELECTRIC_EEL, CE_CLEAN, S_SILENT,
    I_REPTILE, HT_WATER, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_SNAKE
},

// A kraken and its tentacles get a random colour from ETC_KRAKEN.
{
    MONS_KRAKEN, 'X', COLOUR_UNDEF, "kraken",
    M_NO_SKELETON | M_COLD_BLOOD | M_FLEES,
    MR_NO_FLAGS,
    3000, 6, MONS_KRAKEN, MONS_KRAKEN, MH_NATURAL, 60,
    { {AT_BITE, AF_PLAIN, 50}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 10, 6, 0 },
    20, 0, MST_KRAKEN, CE_POISONOUS, S_SILENT,
    I_ANIMAL, HT_WATER, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

{
    MONS_KRAKEN_TENTACLE, 'w', COLOUR_UNDEF, "tentacle",
    M_COLD_BLOOD | M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO,
    MR_NO_FLAGS,
    1300, 10, MONS_KRAKEN, MONS_KRAKEN_TENTACLE, MH_NATURAL, MAG_IMMUNE,
    { {AT_TENTACLE_SLAP, AF_PLAIN, 29}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 2, 0 },
    5, 7, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_LEVITATE, 17, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

{
    MONS_KRAKEN_TENTACLE_SEGMENT, '*', COLOUR_UNDEF, "tentacle segment",
    M_COLD_BLOOD | M_NO_EXP_GAIN | M_STATIONARY | M_SUBMERGES | M_NO_POLY_TO,
    MR_NO_FLAGS,
    1300, 10, MONS_KRAKEN, MONS_KRAKEN_TENTACLE_SEGMENT, MH_NATURAL, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 2, 0 },
    5, 7, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_LEVITATE, 18, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

// Octopode race, not a 'normal' octopus.
// Dummy monster for recolouring.
{
    MONS_OCTOPODE, 'x', LIGHTCYAN, "octopode",
    M_NO_SKELETON | M_SPEAKS,
    MR_NO_FLAGS,
    700, 10, MONS_OCTOPODE, MONS_OCTOPODE, MH_NATURAL, 10,
    { {AT_TENTACLE_SLAP, AF_PLAIN, 24}, {AT_CONSTRICT, AF_CRUSH, 5},
       AT_NO_ATK, AT_NO_ATK },
    { 8, 4, 6, 0 },
    1, 5, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_OCTOPODE_CRUSHER, 'x', YELLOW, "octopode crusher",
    M_NO_SKELETON | M_FIGHTER | M_SPEAKS,
    MR_NO_FLAGS,
    700, 13, MONS_OCTOPODE, MONS_OCTOPODE, MH_NATURAL, 60,
    { {AT_TENTACLE_SLAP, AF_PLAIN, 35}, {AT_CONSTRICT, AF_CRUSH, 10},
       AT_NO_ATK, AT_NO_ATK },
    { 16, 5, 4, 0 },
    1, 18, MST_OCTOPODE_CRUSHER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

// lava monsters
{
    MONS_LAVA_SNAKE, 'S', LIGHTRED, "lava snake",
    M_WARM_BLOOD | M_SUBMERGES,
    mrd(MR_RES_FIRE, 3) | MR_VUL_COLD,
    425, 17, MONS_SNAKE, MONS_LAVA_SNAKE, MH_NATURAL, 10,
    { {AT_BITE, AF_FIRE, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    2, 17, MST_LAVA_SNAKE, CE_CLEAN, S_HISS,
    I_REPTILE, HT_LAVA, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_SNAKE
},

{
    MONS_SALAMANDER, 'N', LIGHTRED, "salamander",
    M_FIGHTER | M_WARM_BLOOD | M_SUBMERGES,
    mrd(MR_RES_FIRE, 3) | MR_VUL_COLD,
    1000, 10, MONS_NAGA, MONS_SALAMANDER, MH_NATURAL, 40,
    { {AT_HIT, AF_FIRE, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 5, 6, 0 },
    5, 7, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    // energy here is MOVE_ENERGY, but excludes swimming
    I_HIGH, HT_AMPHIBIOUS_LAVA, FL_NONE, 10, {14, 10, 10, 10, 10, 10, 10, 100},
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_NAGA
},

{
    MONS_SALAMANDER_FIREBRAND, 'N', WHITE, "salamander firebrand",
    M_FIGHTER | M_WARM_BLOOD | M_SUBMERGES,
    mrd(MR_RES_FIRE, 3) | MR_VUL_COLD,
    1000, 10, MONS_NAGA, MONS_SALAMANDER, MH_NATURAL, 80,
    { {AT_HIT, AF_FIREBRAND, 28}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 4, 5, 0 },
    5, 15, MST_NO_SPELLS, CE_CLEAN, S_SILENT,
    I_HIGH, HT_AMPHIBIOUS_LAVA, FL_NONE, 10, {8, 7, 10, 10, 10, 10, 10, 100},
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_NAGA
},

{
    MONS_SALAMANDER_MYSTIC, 'N', YELLOW, "salamander mystic",
    M_WARM_BLOOD | M_SUBMERGES,
    mrd(MR_RES_FIRE, 3) | MR_VUL_COLD,
    1000, 10, MONS_NAGA, MONS_SALAMANDER, MH_NATURAL, 60,
    { {AT_HIT, AF_FIRE, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 4, 5, 0 },
    5, 7, MST_SALAMANDER_MYSTIC, CE_CLEAN, S_SILENT,
    // energy here is MOVE_ENERGY, but excludes swimming
    I_HIGH, HT_AMPHIBIOUS_LAVA, FL_NONE, 10, {14, 10, 10, 10, 10, 10, 10, 100},
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_NAGA
},

// Semi-unique humans ('@')
{
    MONS_HELLBINDER, '@', ETC_FIRE, "Hellbinder",
    M_SPEAKS | M_WARM_BLOOD | M_MALE,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 0, 0, 150 },
    0, 13, MST_HELLBINDER, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_CLOUD_MAGE, '@', ETC_SILVER, "Cloud Mage",
    M_SPEAKS | M_WARM_BLOOD | M_MALE,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 0, 0, 150 },
    0, 13, MST_CLOUD_MAGE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// 'dummy' a fairly accurate description
DUMMY(MONS_PLAYER, '@', LIGHTGREY, "player")

// player illusion (Mara) - stats are stored in ghost struct. Undead/demonic
// flags are set based on the current player's species!
{
    MONS_PLAYER_ILLUSION, '@', WHITE, "player illusion",
    M_FIGHTER | M_SPEAKS | M_INSUBSTANTIAL | M_NO_POLY_TO,
    MR_RES_POISON,
    0, 15, MONS_PLAYER_ILLUSION, MONS_PLAYER_ILLUSION, MH_NATURAL, -5,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 2, 3, 0 },
    1, 2, MST_GHOST, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_PLAYER_SHADOW, 'W', MAGENTA, "shadow",
    M_CANT_SPAWN | M_NO_EXP_GAIN | M_INSUBSTANTIAL,
    MR_NO_FLAGS,
    0, 10, MONS_HUMAN, MONS_HUMAN, MH_NONLIVING, -3,
    { {AT_HIT, AF_PLAIN, 0}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 0, 0, 0, 1 },
    3, 13, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},
// dancing weapon
// These are named more explicitly when they attack, also when you use 'x'
// to examine them.
{
    MONS_DANCING_WEAPON, '(', COLOUR_UNDEF, "dancing weapon",
    M_FIGHTER,
    MR_RES_POISON | mrd(MR_RES_FIRE | MR_RES_COLD, 2) | mrd(MR_RES_ELEC, 3),
    425, 10, MONS_DANCING_WEAPON, MONS_DANCING_WEAPON, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 15 },
    10, 20, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

// spectral weapon, for skalds!
{
    MONS_SPECTRAL_WEAPON, '(', GREEN, "spectral weapon",
    M_SHADOW | M_INSUBSTANTIAL | M_NO_REGEN | M_AVATAR,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD |  MR_RES_ELEC,
    0, 11, MONS_WRAITH, MONS_SPECTRAL_WEAPON, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 3, 0, 0},
    5, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

// Demonic tentacle things.
{
    MONS_ELDRITCH_TENTACLE, 'w', COLOUR_UNDEF, "eldritch tentacle",
    M_NO_POLY_TO | M_STATIONARY | M_SEE_INVIS,
    mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC | MR_RES_ACID, 3)
        | MR_RES_HELLFIRE | MR_RES_STICKY_FLAME,
    1800, 10, MONS_ELDRITCH_TENTACLE, MONS_ELDRITCH_TENTACLE,
        MH_NONLIVING, MAG_IMMUNE,
    { {AT_TENTACLE_SLAP, AF_CHAOS, 30}, {AT_CLAW, AF_CHAOS, 40}, AT_NO_ATK,
       AT_NO_ATK },
    { 16, 5, 5, 0 },
    13, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_LEVITATE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_SNAKE
},

{
    MONS_ELDRITCH_TENTACLE_SEGMENT, '*', COLOUR_UNDEF, "eldritch tentacle segment",
    M_NO_EXP_GAIN | M_STATIONARY | M_NO_POLY_TO | M_SEE_INVIS,
    mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC | MR_RES_ACID, 3)
        | MR_RES_HELLFIRE | MR_RES_STICKY_FLAME,
    1800, 10, MONS_ELDRITCH_TENTACLE, MONS_ELDRITCH_TENTACLE_SEGMENT,
        MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 5, 5, 0 },
    13, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_ANIMAL, HT_AMPHIBIOUS, FL_LEVITATE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_MISC
},

// demonspawn ('6')
{
    MONS_DEMONSPAWN, '6', BROWN, "demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    3, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MONSTROUS_DEMONSPAWN, '6', LIGHTGRAY, "monstrous demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_MONSTROUS_DEMONSPAWN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 15},
      {AT_GORE, AF_PLAIN, 15}, AT_NO_ATK },
    { 10, 9, 2, 0 },
    3, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GELID_DEMONSPAWN, '6', BLUE, "gelid demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    mrd(MR_RES_COLD, 2),
    550, 10, MONS_DEMONSPAWN, MONS_GELID_DEMONSPAWN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    6, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_INFERNAL_DEMONSPAWN, '6', RED, "infernal demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    mrd(MR_RES_FIRE, 2),
    550, 10, MONS_DEMONSPAWN, MONS_INFERNAL_DEMONSPAWN, MH_NATURAL, 40,
    { {AT_HIT, AF_FIRE, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    6, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_PUTRID_DEMONSPAWN, '6', GREEN, "putrid demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_ALWAYS_CORPSE | M_NO_POLY_TO,
    MR_RES_POISON,
    550, 10, MONS_DEMONSPAWN, MONS_PUTRID_DEMONSPAWN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 4, 0 },
    3, 13, MST_NO_SPELLS, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_TORTUROUS_DEMONSPAWN, '6', MAGENTA, "torturous demonspawn",
    M_WARM_BLOOD | M_SPEAKS | M_NO_POLY_TO,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_TORTUROUS_DEMONSPAWN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 4, 0 },
    3, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// The nonbase demonspawn's hit dice HP values, AC, and EV are added to
// those of the relevant base demonspawn type.
{
    MONS_BLOOD_SAINT, '6', LIGHTBLUE, "blood saint",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 0 },
    0, 0, MST_DEMONSPAWN_BLOOD_SAINT, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_CHAOS_CHAMPION, '6', ETC_RANDOM, "chaos champion",
    M_WARM_BLOOD | M_SPEAKS | M_FIGHTER,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 1, 0 },
    0, 0, MST_DEMONSPAWN_CHAOS_CHAMPION, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_WARMONGER, '6', LIGHTCYAN, "warmonger",
    M_WARM_BLOOD | M_SPEAKS | M_FIGHTER,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 1, 2, 0 },
    0, 2, MST_DEMONSPAWN_WARMONGER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_CORRUPTER, '6', LIGHTGREEN, "corrupter",
    M_WARM_BLOOD | M_SPEAKS,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 1, 0 },
    0, 0, MST_DEMONSPAWN_CORRUPTER, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_BLACK_SUN, '6', LIGHTMAGENTA, "black sun",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS,
    MR_NO_FLAGS,
    550, 10, MONS_DEMONSPAWN, MONS_DEMONSPAWN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 0 },
    3, 13, MST_DEMONSPAWN_BLACK_SUN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// minor demons: imps, etc. ('5')
{
    MONS_CRIMSON_IMP, '5', RED, "crimson imp",
    M_SPEAKS | M_FAST_REGEN,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD,
    300, 13, MONS_CRIMSON_IMP, MONS_CRIMSON_IMP, MH_DEMONIC, 40,
    { {AT_HIT, AF_PLAIN, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 3, 0 },
    3, 14, MST_BLINKER, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE,
         MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_QUASIT, '5', LIGHTGREY, "quasit",
    M_NO_FLAGS,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD,
    425, 13, MONS_QUASIT, MONS_QUASIT, MH_DEMONIC, 10,
    { {AT_BITE, AF_DRAIN_DEX, 3}, {AT_CLAW, AF_DRAIN_DEX, 2},
      {AT_CLAW, AF_DRAIN_DEX, 2}, AT_NO_ATK },
    { 3, 2, 6, 0 },
    5, 17, MST_NO_SPELLS, CE_NOCORPSE, S_MOAN,
    I_NORMAL, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_WHITE_IMP, '5', WHITE, "white imp",
    M_SPEAKS,
    MR_RES_POISON | mrd(MR_RES_COLD, 2) | MR_VUL_FIRE,
    300, 10, MONS_WHITE_IMP, MONS_WHITE_IMP, MH_DEMONIC, 10,
    { {AT_HIT, AF_COLD, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    4, 10, MST_WHITE_IMP, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LITTLE,
         MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_UFETUBUS, '5', LIGHTCYAN, "ufetubus",
    M_NO_FLAGS,
    MR_VUL_FIRE | MR_RES_COLD,
    550, 28, MONS_UFETUBUS, MONS_UFETUBUS, MH_DEMONIC, 10,
    { {AT_HIT, AF_PLAIN, 5}, {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK },
    { 1, 4, 6, 0 },
    2, 15, MST_NO_SPELLS, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_IRON_IMP, '5', CYAN, "iron imp",
    M_SPEAKS,
    MR_RES_POISON | mrd(MR_RES_ELEC, 2) | MR_RES_HELLFIRE | MR_RES_COLD,
    875, 14, MONS_IRON_IMP, MONS_IRON_IMP, MH_DEMONIC, 10,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 3, 5, 0 },
    6, 8, MST_NO_SPELLS, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_SHADOW_IMP, '5', MAGENTA, "shadow imp",
    M_SEE_INVIS | M_SPEAKS,
    MR_RES_POISON | mrd(MR_RES_COLD, 2),
    100, 11, MONS_SHADOW_IMP, MONS_SHADOW_IMP, MH_DEMONIC, 10,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 3, 5, 0 },
    3, 11, MST_SHADOW_IMP, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

// devils etc. ('4')
{
    MONS_BLUE_DEVIL, '4', BLUE, "blue devil",
    M_NO_FLAGS,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    550, 14, MONS_BLUE_DEVIL, MONS_BLUE_DEVIL, MH_DEMONIC, 40,
    { {AT_SWOOP, AF_PLAIN, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    14, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_RUST_DEVIL, '4', BROWN, "rust devil",
    M_NO_FLAGS,
    MR_RES_POISON | mrd(MR_RES_ELEC, 2) | MR_RES_HELLFIRE | MR_RES_COLD,
    1100, 15, MONS_RUST_DEVIL, MONS_RUST_DEVIL, MH_DEMONIC, 60,
    { {AT_HIT, AF_CORRODE, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 3, 5, 0 },
    10, 8, MST_NO_SPELLS, CE_NOCORPSE, S_SCREECH,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ORANGE_DEMON, '4', LIGHTRED, "orange demon",
    M_NO_FLAGS,
    MR_NO_FLAGS,
    550, 12, MONS_ORANGE_DEMON, MONS_ORANGE_DEMON, MH_DEMONIC, 60,
    { {AT_REACH_STING, AF_WEAKNESS_POISON, 10}, {AT_HIT, AF_PLAIN, 8}, AT_NO_ATK,
       AT_NO_ATK },
    { 8, 4, 5, 0 },
    3, 7, MST_NO_SPELLS, CE_NOCORPSE, S_SCREECH,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID_TAILED
},

{
    MONS_RED_DEVIL, '4', RED, "red devil",
    M_FIGHTER,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD,
    550, 13, MONS_RED_DEVIL, MONS_RED_DEVIL, MH_DEMONIC, 60,
    { {AT_KITE, AF_PLAIN, 19}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 3, 0 },
    7, 13, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_SIXFIRHY, '4', LIGHTBLUE, "sixfirhy",
    M_NO_FLAGS,
    MR_NO_FLAGS, // Can't have RES_ELEC since most sources of damage do nothing
                 // in that case.  We want to "suffer" the damage to get healed.
    300, 6, MONS_SIXFIRHY, MONS_SIXFIRHY, MH_DEMONIC, 60,
    { {AT_HIT, AF_ELEC, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    2, 20, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 40, MOVE_ENERGY(6), // speed is cut to 1/3 later
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    MONS_HELLWING, '4', LIGHTGREY, "hellwing",
    M_FAST_REGEN,
    MR_RES_POISON,
    1300, 12, MONS_HELLWING, MONS_HELLWING, MH_DEMONIC, 60,
    { {AT_HIT, AF_PLAIN, 17}, {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK },
    { 7, 4, 5, 0 },
    8, 10, MST_HELLWING, CE_NOCORPSE, S_MOAN,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID_WINGED
},

// middle demons ('3')
{
    MONS_SUN_DEMON, '3', YELLOW, "sun demon",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_ELEC | MR_RES_POISON | MR_VUL_COLD | MR_RES_HELLFIRE | MR_VUL_WATER,
    550, 14, MONS_SUN_DEMON, MONS_SUN_DEMON, MH_DEMONIC, 80,
    { {AT_HIT, AF_FIRE, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    10, 12, MST_NO_SPELLS, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 12, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SOUL_EATER, '3', MAGENTA, "soul eater",
    M_SEE_INVIS,
    MR_RES_POISON | MR_RES_COLD,
    550, 13, MONS_SOUL_EATER, MONS_SOUL_EATER, MH_DEMONIC, 140,
    { {AT_HIT, AF_DRAIN_XP, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    18, 10, MST_SOUL_EATER, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ICE_DEVIL, '3', WHITE, "ice devil",
    M_NO_FLAGS,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    550, 11, MONS_ICE_DEVIL, MONS_ICE_DEVIL, MH_DEMONIC, 80,
    { {AT_HIT, AF_COLD, 16}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 3, 5, 0 },
    12, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SMOKE_DEMON, '3', LIGHTGREY, "smoke demon",
    M_INSUBSTANTIAL | M_UNBREATHING,
    MR_RES_POISON | mrd(MR_RES_FIRE, 2),
    0, 15, MONS_SMOKE_DEMON, MONS_SMOKE_DEMON, MH_DEMONIC, 60,
    { {AT_HIT, AF_PLAIN, 8}, {AT_HIT, AF_PLAIN, 5}, {AT_HIT, AF_PLAIN, 5},
       AT_NO_ATK },
    { 7, 3, 5, 0 },
    5, 9, MST_SMOKE_DEMON, CE_NOCORPSE, S_ROAR,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_NEQOXEC, '3', LIGHTMAGENTA, "neqoxec",
    M_NO_FLAGS,
    MR_RES_POISON,
    550, 12, MONS_NEQOXEC, MONS_NEQOXEC, MH_DEMONIC, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    4, 12, MST_NEQOXEC, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_YNOXINUL, '3', LIGHTCYAN, "ynoxinul",
    M_SEE_INVIS,
    mrd(MR_RES_ELEC, 2) | MR_RES_POISON | MR_RES_COLD,
    550, 13, MONS_YNOXINUL, MONS_YNOXINUL, MH_DEMONIC, 40,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    3, 10, MST_YNOXINUL, CE_NOCORPSE, S_BELLOW,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_CHAOS_SPAWN, '3', ETC_RANDOM, "chaos spawn",
    M_SEE_INVIS,
    MR_NO_FLAGS,
    1500, 12, MONS_CHAOS_SPAWN, MONS_CHAOS_SPAWN, MH_DEMONIC, 60,
    { {AT_RANDOM, AF_CHAOS, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 5, 0 },
    7, 12, MST_NO_SPELLS, CE_NOCORPSE, S_RANDOM,
    I_ANIMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_MISC
},

// greater demons ('2')

{
    MONS_SHADOW_DEMON, '2', MAGENTA, "shadow demon",
    M_SEE_INVIS,
    MR_RES_POISON | mrd(MR_RES_COLD, 2),
    141, 13, MONS_SHADOW_DEMON, MONS_SHADOW_DEMON, MH_DEMONIC, 100,
    { {AT_HIT, AF_PLAIN, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 3, 5, 0 },
    7, 12, MST_SHADOW_DEMON, CE_NOCORPSE, S_CROAK,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_GREEN_DEATH, '2', GREEN, "green death",
    M_SEE_INVIS,
    MR_RES_POISON,
    1300, 14, MONS_GREEN_DEATH, MONS_GREEN_DEATH, MH_DEMONIC, 160,
    { {AT_HIT, AF_PLAIN, 32}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 3, 5, 0 },
    5, 7, MST_GREEN_DEATH, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_BLIZZARD_DEMON, '2', LIGHTBLUE, "blizzard demon",
    M_SEE_INVIS,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD | MR_RES_ELEC, 2),
    1300, 16, MONS_BLIZZARD_DEMON, MONS_BLIZZARD_DEMON, MH_DEMONIC, 140,
    { {AT_HIT, AF_PLAIN, 20}, {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK },
    { 12, 3, 5, 0 },
    10, 10, MST_BLIZZARD_DEMON, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_BALRUG, '2', RED, "balrug",
    M_FIGHTER | M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD | MR_VUL_WATER,
    1300, 12, MONS_BALRUG, MONS_BALRUG, MH_DEMONIC, 160,
    { {AT_HIT, AF_FIRE, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    5, 12, MST_BALRUG, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_WINGED, 12, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_CACODEMON, '2', YELLOW, "cacodemon",
    M_SEE_INVIS | M_FAST_REGEN,
    MR_RES_POISON | MR_RES_ELEC,
    1300, 17, MONS_CACODEMON, MONS_CACODEMON, MH_DEMONIC, 160,
    { {AT_HIT, AF_PLAIN, 22}, {AT_HIT, AF_PLAIN, 22}, AT_NO_ATK, AT_NO_ATK },
    { 13, 5, 5, 0 },
    11, 10, MST_CACODEMON, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{   // with randomised stats
    MONS_HELL_BEAST, '2', BROWN, "hell beast",
    M_FIGHTER,
    MR_NO_FLAGS,
    1300, 17, MONS_HELL_BEAST, MONS_HELL_BEAST, MH_DEMONIC, 20,
    { {AT_BITE, AF_PLAIN, 28}, {AT_TRAMPLE, AF_PLAIN, 20}, AT_NO_ATK,
       AT_NO_ATK },
    { 7, 9, 6, 0 },
    5, 14, MST_NO_SPELLS, CE_NOCORPSE, S_RANDOM,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_QUADRUPED
},

{
    MONS_HELLION, '2', LIGHTRED, "hellion",
    M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD | MR_VUL_WATER,
    550, 12, MONS_HELLION, MONS_HELLION, MH_DEMONIC, 60,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    5, 10, MST_HELLION, CE_NOCORPSE, S_SCREAM,
    I_HIGH, HT_LAND, FL_NONE, 12, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_REAPER, '2', LIGHTGREY, "reaper",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS,
    MR_RES_POISON | MR_RES_COLD,
    550, 14, MONS_REAPER, MONS_REAPER, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    15, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_LOROCYPROCA, '2', BLUE, "lorocyproca",
    M_SEE_INVIS | M_INVIS,
    MR_RES_POISON | MR_RES_COLD | MR_RES_FIRE | MR_RES_ELEC,
    550, 14, MONS_LOROCYPROCA, MONS_LOROCYPROCA, MH_DEMONIC, 140,
    { {AT_HIT, AF_ANTIMAGIC, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 3, 5, 0 },
    10, 12, MST_NO_SPELLS, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_TORMENTOR, '2', LIGHTMAGENTA, "tormentor",
    M_SPEAKS,
    MR_RES_POISON | MR_RES_FIRE,
    550, 10, MONS_TORMENTOR, MONS_TORMENTOR, MH_DEMONIC, 60,
    { {AT_HIT, AF_PAIN, 8}, {AT_HIT, AF_PAIN, 8}, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 5, 0 },
    12, 12, MST_TORMENTOR, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// fiends, etc. ('1')
{
    MONS_BRIMSTONE_FIEND, '1', LIGHTRED, "Brimstone Fiend",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD | MR_VUL_WATER,
    1300, 17, MONS_BRIMSTONE_FIEND, MONS_BRIMSTONE_FIEND, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 25}, {AT_HIT, AF_PLAIN, 15}, {AT_HIT, AF_PLAIN, 15},
       AT_NO_ATK },
    { 18, 3, 5, 0 },
    15, 6, MST_FIEND, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_ICE_FIEND, '1', WHITE, "Ice Fiend",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 3),
    1300, 17, MONS_ICE_FIEND, MONS_ICE_FIEND, MH_DEMONIC, MAG_IMMUNE,
    { {AT_CLAW, AF_COLD, 25}, {AT_CLAW, AF_COLD, 25}, AT_NO_ATK, AT_NO_ATK },
    { 18, 3, 5, 0 },
    15, 6, MST_ICE_FIEND, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_SHADOW_FIEND, '1', MAGENTA, "Shadow Fiend",
    M_SEE_INVIS | M_SHADOW,
    MR_RES_POISON | mrd(MR_RES_COLD, 2) | MR_RES_ELEC,
    433, 14, MONS_SHADOW_FIEND, MONS_SHADOW_FIEND, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PAIN, 25}, {AT_HIT, AF_DRAIN_XP, 15},
      {AT_HIT, AF_DRAIN_XP, 15}, AT_NO_ATK },
    { 18, 3, 5, 0 },
    15, 6, MST_SHADOW_FIEND, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_HELL_SENTINEL, '1', BROWN, "Hell Sentinel",
    M_SEE_INVIS | M_GLOWS_LIGHT,
    MR_RES_HELLFIRE | mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC, 3),
    1300, 10, MONS_HELL_SENTINEL, MONS_HELL_SENTINEL, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 40}, {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK },
    { 19, 5, 5, 0 },
    25, 3, MST_HELL_SENTINEL, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_EXECUTIONER, '1', LIGHTGREY, "Executioner",
    M_FIGHTER | M_SEE_INVIS,
    MR_RES_ELEC | MR_RES_FIRE | MR_RES_COLD | MR_RES_POISON,
    1300, 18, MONS_EXECUTIONER, MONS_EXECUTIONER, MH_DEMONIC, 140,
    { {AT_HIT, AF_PLAIN, 30}, {AT_HIT, AF_PLAIN, 10}, {AT_HIT, AF_PLAIN, 10},
       AT_NO_ATK },
    { 12, 3, 5, 0 },
    10, 15, MST_EXECUTIONER, CE_NOCORPSE, S_SCREAM,
    I_HIGH, HT_LAND, FL_NONE, 20, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// trees and related creatures ('7')
DUMMY(MONS_ANIMATED_TREE, '7', ETC_TREE, "animated tree")

// non-living creatures
// golems ('8')
DUMMY(MONS_GOLEM, '8', LIGHTGREY, "golem")

{
    MONS_IRON_GOLEM, '8', CYAN, "iron golem",
    M_ARTIFICIAL,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 3),
    3900, 10, MONS_GOLEM, MONS_IRON_GOLEM, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 7, 4, 0 },
    15, 3, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 7, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_CRYSTAL_GUARDIAN, '8', GREEN, "crystal guardian",
    M_SEE_INVIS | M_SPEAKS | M_ARTIFICIAL,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 3),
    2600, 10, MONS_GOLEM, MONS_CRYSTAL_GUARDIAN, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 3, 2, 0 },
    22, 0, MST_CRYSTAL_GUARDIAN, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_TOENAIL_GOLEM, '8', RED, "toenail golem",
    M_ARTIFICIAL,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    1300, 10, MONS_GOLEM, MONS_TOENAIL_GOLEM, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 13}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 5, 3, 0 },
    8, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_ELECTRIC_GOLEM, '8', LIGHTCYAN, "electric golem",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_SPEAKS
        | M_ARTIFICIAL,
    mrd(MR_RES_ELEC | MR_RES_POISON, 3) | MR_RES_FIRE | MR_RES_COLD,
    0, 12, MONS_GOLEM, MONS_ELECTRIC_GOLEM, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_ELEC, 15}, {AT_HIT, AF_ELEC, 15}, {AT_HIT, AF_PLAIN, 15},
      {AT_HIT, AF_PLAIN, 15} },
    { 15, 7, 4, 0 },
    5, 20, MST_ELECTRIC_GOLEM, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 16, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_USHABTI, '8', BROWN, "ushabti",
    M_ARTIFICIAL | M_FIGHTER | M_SEE_INVIS,
    mrd(MR_RES_POISON, 3) | mrd(MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 1),
    300, 10, MONS_GOLEM, MONS_USHABTI, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 5, 3, 0 },
    9, 6, MST_USHABTI, CE_NOCORPSE, S_RUMBLE,
    I_NORMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_MISC
},

{
    MONS_GUARDIAN_GOLEM, '8', LIGHTGREEN, "guardian golem",
    M_ARTIFICIAL,
    mrd(MR_RES_POISON, 3) | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC,
    1300, 10, MONS_GOLEM, MONS_GUARDIAN_GOLEM, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 6, 4, 0 },
    10, 2, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
   MONS_SPELLFORGED_SERVITOR, '8', LIGHTMAGENTA, "spellforged servitor",
    M_ARTIFICIAL | M_NO_POLY_TO,
    mrd(MR_RES_POISON, 3) | mrd(MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2),
    550, 10, MONS_GOLEM, MONS_GOLEM, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 0, 0, 80 },
    10, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// statues and statue-like things (also '8')
{
    MONS_ICE_STATUE, '8', LIGHTBLUE, "ice statue",
    M_ARTIFICIAL | M_STATIONARY | M_SPEAKS,
    MR_RES_POISON | MR_VUL_FIRE | mrd(MR_RES_COLD, 3) | mrd(MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    1300, 10, MONS_STATUE, MONS_ICE_STATUE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 0, 0, 70 },
    12, 1, MST_ICE_STATUE, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 16, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_OBSIDIAN_STATUE, '8', MAGENTA, "obsidian statue",
    M_ARTIFICIAL | M_STATIONARY | M_SPEAKS,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    5200, 10, MONS_STATUE, MONS_OBSIDIAN_STATUE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 70 },
    12, 1, MST_OBSIDIAN_STATUE, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_ORANGE_STATUE, '8', LIGHTRED, "orange crystal statue",
    M_ARTIFICIAL | M_STATIONARY | M_SPEAKS,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    1300, 10, MONS_STATUE, MONS_ORANGE_STATUE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 70 },
    12, 1, MST_ORANGE_CRYSTAL_STATUE, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 6, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{ // always redefined
    MONS_STATUE, '8', LIGHTGREY, "statue",
    M_ARTIFICIAL | M_STATIONARY | M_SPEAKS | M_ARCHER | M_DONT_MELEE | M_NO_POLY_TO,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    5200, 10, MONS_STATUE, MONS_STATUE, MH_NONLIVING, MAG_IMMUNE,
    { {AT_WEAP_ONLY, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 0, 0, 70 },
    12, 1, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_TRAINING_DUMMY, '8', LIGHTGREY, "training dummy",
    M_STATIONARY,
    MR_RES_PETRIFY,
    1300, 10, MONS_TRAINING_DUMMY, MONS_TRAINING_DUMMY, MH_NONLIVING, MAG_IMMUNE,
    { {AT_WEAP_ONLY, AF_PLAIN, 1}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 6 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_PILLAR_OF_SALT, '8', WHITE, "pillar of salt",
    M_NO_EXP_GAIN | M_STATIONARY,
    MR_RES_POISON,
    550, 10, MONS_PILLAR_OF_SALT, MONS_PILLAR_OF_SALT, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 1 },
    1, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_BLOCK_OF_ICE, '8', ETC_ICE, "block of ice",
    M_NO_EXP_GAIN | M_STATIONARY,
    mrd(MR_RES_COLD, 3) | MR_VUL_FIRE | MR_RES_POISON,
    550, 10, MONS_BLOCK_OF_ICE, MONS_BLOCK_OF_ICE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 5, 20 },
    15, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_DIAMOND_OBELISK, '8', WHITE, "diamond obelisk",
    M_ARTIFICIAL | M_STATIONARY | M_NO_EXP_GAIN | M_NO_POLY_TO,
    MR_RES_PETRIFY,
    1300, 10, MONS_DIAMOND_OBELISK, MONS_DIAMOND_OBELISK, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 8, 0, 0, 10000 },
    12, 1, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

{
    MONS_LIGHTNING_SPIRE, '8', ETC_ELECTRICITY, "lightning spire",
    M_STATIONARY | M_NO_POLY_TO,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    425, 10, MONS_LIGHTNING_SPIRE, MONS_LIGHTNING_SPIRE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 20 },
    13, 3, MST_ELECTRIC_EEL, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_MISC
},

// gargoyles ('9')
{
    MONS_GARGOYLE, '9', LIGHTGREY, "gargoyle",
    M_ARTIFICIAL,
    mrd(MR_RES_POISON, 3) | MR_RES_ELEC,
    550, 26, MONS_GARGOYLE, MONS_GARGOYLE, MH_NONLIVING, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 3, 3, 0 },
    18, 6, MST_GARGOYLE, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED_TAILED
},

{
    MONS_WAR_GARGOYLE, '9', CYAN, "war gargoyle",
    M_ARTIFICIAL | M_SEE_INVIS | M_FIGHTER,
    MR_RES_ELEC | MR_RES_FIRE | MR_RES_COLD | mrd(MR_RES_POISON, 3),
    550, 18, MONS_GARGOYLE, MONS_WAR_GARGOYLE, MH_NONLIVING, 100,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 2, 4, 0 },
    22, 4, MST_IRON_DRAGON, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_WINGED, 14, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED_TAILED
},

{
    MONS_MOLTEN_GARGOYLE, '9', RED, "molten gargoyle",
    M_ARTIFICIAL,
    mrd(MR_RES_POISON, 3) | MR_RES_ELEC | mrd(MR_RES_FIRE, 3),
    550, 18, MONS_GARGOYLE, MONS_MOLTEN_GARGOYLE, MH_NONLIVING, 60,
    { {AT_HIT, AF_FIRE, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 7, 3, 3, 0 },
    14, 7, MST_MOLTEN_GARGOYLE, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED_TAILED
},

// major demons ('&')
// Random demon in pan - only one per level.  Stats are stored in ghost struct.
{
    MONS_PANDEMONIUM_LORD, '&', COLOUR_UNDEF, "pandemonium lord",
    // See invis is also set in ghost.cc
    M_FIGHTER | M_SPEAKS | M_HYBRID | M_SEE_INVIS,
    MR_RES_POISON,
    1300, 14, MONS_PANDEMONIUM_LORD, MONS_PANDEMONIUM_LORD, MH_DEMONIC, -5,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 19, 0, 8, 100 },
    1, 2, MST_GHOST, CE_NOCORPSE, S_DEMON_TAUNT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// Demon in hell.  Currently only used as genus/species for hell guardians.
{ // dummy genus monster -- used as a species so not a DUMMY
    MONS_HELL_LORD, '&', COLOUR_UNDEF, "hell lord",
    M_FIGHTER | M_SPEAKS | M_CANT_SPAWN,
    MR_RES_POISON,
    1300, 14, MONS_HELL_LORD, MONS_HELL_LORD, MH_DEMONIC, -5,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 19, 0, 8, 100 },
    1, 2, MST_GHOST, CE_NOCORPSE, S_DEMON_TAUNT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// explodey things / orb of fire ('*')
{
    MONS_BALL_LIGHTNING, '*', LIGHTCYAN, "ball lightning",
    M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_CONJURED,
    mrd(MR_RES_ELEC | MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD, 3)
        | MR_RES_WIND,
    0, 20, MONS_BALL_LIGHTNING, MONS_BALL_LIGHTNING, MH_NONLIVING, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 12, 0, 0, 1 },
    0, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 20, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{
    MONS_GIANT_SPORE, '*', GREEN, "giant spore",
    M_NO_FLAGS,
    MR_RES_POISON,
    425, 10, MONS_FUNGUS, MONS_GIANT_SPORE, MH_PLANT, 0,
    { {AT_HIT, AF_PLAIN, 1}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 1 },
    0, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ORB
},

{
    MONS_LOST_SOUL, '*', LIGHTGREEN, "lost soul",
    M_INSUBSTANTIAL | M_MAINTAIN_RANGE | M_SUBMERGES,
    mrd(MR_RES_POISON | MR_RES_ELEC, 1) | MR_RES_FIRE | MR_RES_COLD,
    0, 2, MONS_LOST_SOUL, MONS_LOST_SOUL, MH_UNDEAD, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 1, 3, 0 },
    0, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 13, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ORB
},

{
    MONS_LURKING_HORROR, '*', BLUE, "lurking horror",
    M_INSUBSTANTIAL, MR_NO_FLAGS,
    0, 10, MONS_LURKING_HORROR, MONS_LURKING_HORROR, MH_NONLIVING, 10,
    { {AT_HIT, AF_PLAIN, 1}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 1 },
    0, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 12, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_ORB
},

{
    MONS_ORB_OF_FIRE, '*', RED, "orb of fire",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_GLOWS_LIGHT
        | M_GLOWS_RADIATION,
    mrd(MR_RES_POISON | MR_RES_ELEC, 3) | MR_RES_HELLFIRE | MR_RES_COLD
        | MR_VUL_WATER,
    0, 13, MONS_ORB_OF_FIRE, MONS_ORB_OF_FIRE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 30, 0, 0, 150 },
    20, 20, MST_ORB_OF_FIRE, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{ // not an actual monster, used by a spell
    MONS_ORB_OF_DESTRUCTION, '*', WHITE, "orb of destruction",
    M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_NO_EXP_GAIN | M_NO_POLY_TO
        | M_PROJECTILE,
    mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC, 3) | MR_RES_HELLFIRE
        | mrd(MR_RES_ACID, 3) | MR_RES_STICKY_FLAME,
    0, 0, MONS_ORB_OF_DESTRUCTION, MONS_ORB_OF_DESTRUCTION,
        MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 0, 0, 1000 /* unkillable */ },
    0, 10, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{ // not an actual monster, used by a spell
    MONS_FULMINANT_PRISM, '*', ETC_MAGIC, "fulminant prism",
    M_GLOWS_LIGHT | M_NO_POLY_TO | M_STATIONARY | M_CONJURED,
    mrd(MR_RES_POISON, 3) | mrd(MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 1),
    300, 0, MONS_FULMINANT_PRISM, MONS_FULMINANT_PRISM,
        MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 2, 2, 0},
    3, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{ // not an actual monster, used by a spell
    MONS_SINGULARITY, '*', ETC_DISJUNCTION, "singularity",
    M_NO_POLY_TO | M_STATIONARY | M_CONJURED | M_NO_EXP_GAIN | M_NO_POLY_TO
        | M_PROJECTILE | M_INSUBSTANTIAL,
    mrd(MR_RES_POISON, 3) | mrd(MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 3),
    300, 0, MONS_SINGULARITY, MONS_SINGULARITY,
        MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 0, 0, 10000 }, // Invincible in monster::hurt
    3, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{
    MONS_WRETCHED_STAR, '*', MAGENTA, "wretched star",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_GLOWS_LIGHT
        | M_GLOWS_RADIATION,
    mrd(MR_RES_POISON | MR_RES_ELEC, 1) | MR_RES_FIRE | MR_RES_COLD,
    0, 13, MONS_WRETCHED_STAR, MONS_WRETCHED_STAR, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 70 },
    10, 10, MST_WRETCHED_STAR, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(8),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

{
    MONS_BATTLESPHERE, '*', ETC_MAGIC, "battlesphere",
    M_SEE_INVIS | M_INSUBSTANTIAL | M_GLOWS_LIGHT | M_NO_EXP_GAIN
        | M_NO_POLY_TO | M_MAINTAIN_RANGE | M_AVATAR,
    mrd(MR_RES_POISON | MR_RES_COLD | MR_RES_ELEC | MR_RES_FIRE, 2)
    | MR_RES_ACID | MR_RES_STICKY_FLAME,
    0, 0, MONS_BATTLESPHERE, MONS_BATTLESPHERE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 2, 2, 0},
    0, 5, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_LEVITATE, 30, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_ORB
},

// other symbols
{
    MONS_DEATH_COB, '%', YELLOW, "death cob",
    M_SPEAKS,
    MR_RES_COLD,
    150, 10, MONS_DEATH_COB, MONS_DEATH_COB, MH_UNDEAD, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 4, 5, 0 },
    10, 15, MST_NO_SPELLS, CE_NOCORPSE, S_MOAN,
    I_NORMAL, HT_LAND, FL_NONE, 25, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_HUMANOID
},

// non-human uniques
// "A"ngels.
{
    MONS_MENNAS, 'A', LIGHTCYAN, "Mennas",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_GLOWS_LIGHT | M_UNIQUE | M_MALE,
    MR_RES_POISON | mrd(MR_RES_ELEC, 2),
    550, 10, MONS_ANGEL, MONS_ANGEL, MH_HOLY, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 30}, {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK,
       AT_NO_ATK },
    { 19, 0, 0, 150 },
    15, 28, MST_MENNAS, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_WINGED, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED
},

// "c"entaurs.
{
    MONS_NESSOS, 'c', MAGENTA, "Nessos",
    M_UNIQUE | M_WARM_BLOOD | M_ARCHER | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    1500, 12, MONS_CENTAUR, MONS_CENTAUR, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 16}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 0, 0, 72 },
    4, 8, MST_NESSOS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_CENTAUR
},

// "C"yclopes and giants.
{
    MONS_CHUCK, 'C', WHITE, "Chuck",
    M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_NO_FLAGS,
    2300, 10, MONS_GIANT, MONS_STONE_GIANT, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 120 },
    14, 2, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_IRON_GIANT, 'C', CYAN, "the iron giant",
    M_WARM_BLOOD | M_SPEAKS | M_FIGHTER | M_UNIQUE,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD,
    3500, 10, MONS_GIANT, MONS_IRON_GIANT, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 60}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 22, 0, 0, 220 },
    18, 2, MST_IRON_GIANT, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_POLYPHEMUS, 'C', GREEN, "Polyphemus",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    2500, 10, MONS_GIANT, MONS_CYCLOPS, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 50}, {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 150 },
    10, 3, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_ANTAEUS, 'C', LIGHTCYAN, "Antaeus",
    M_UNIQUE | M_WARM_BLOOD | M_FIGHTER | M_SEE_INVIS | M_MALE
        | M_SPEAKS,
    mrd(MR_RES_ELEC | MR_RES_COLD, 2) | MR_VUL_FIRE,
    1800, 15, MONS_GIANT, MONS_TITAN, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_COLD, 75}, {AT_HIT, AF_COLD, 30}, AT_NO_ATK, AT_NO_ATK },
    { 22, 0, 0, 700 },
    28, 4, MST_ANTAEUS, CE_NOCORPSE, S_DEMON_TAUNT,
    I_HIGH, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

// "d"raconians.
{
    MONS_TIAMAT, 'd', COLOUR_UNDEF, "Tiamat",
    M_UNIQUE | M_SEE_INVIS | M_COLD_BLOOD | M_SPEAKS | M_FEMALE,
    MR_RES_POISON,
    900, 10, MONS_DRACONIAN, MONS_DRACONIAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 60}, {AT_TAIL_SLAP, AF_PLAIN, 45}, AT_NO_ATK,
       AT_NO_ATK },
    { 22, 0, 0, 380 },
    // Gets her breath in
    30, 10, MST_NO_SPELLS, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM,
        MON_SHAPE_HUMANOID_WINGED_TAILED
},

// "D"ragons and hydras.
{
    MONS_XTAHUA, 'D', RED, "Xtahua",
    M_UNIQUE | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_RES_POISON | mrd(MR_RES_FIRE, 2) | MR_VUL_COLD,
    2400, 18, MONS_DRAGON, MONS_FIRE_DRAGON, MH_NATURAL, 180,
    { {AT_BITE, AF_PLAIN, 35}, {AT_CLAW, AF_PLAIN, 17},
      {AT_TRAMPLE, AF_PLAIN, 20}, AT_NO_ATK },
    { 19, 0, 0, 133 },
    15, 7, MST_XTAHUA, CE_CLEAN, S_ROAR,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_DOORS, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_LERNAEAN_HYDRA, 'D', YELLOW, "the Lernaean hydra",
    M_UNIQUE | M_COLD_BLOOD | M_FAST_REGEN,
    MR_RES_POISON,
    2100, 11, MONS_HYDRA, MONS_HYDRA, MH_NATURAL, 120,
    { {AT_BITE, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 30, 0, 0, 150 },
    0, 5, MST_NO_SPELLS, CE_POISONOUS, S_ROAR,
    I_REPTILE, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

{
    MONS_SERPENT_OF_HELL, 'D', ETC_FIRE, "the Serpent of Hell",
    M_SEE_INVIS | M_UNIQUE,
    MR_RES_POISON | MR_RES_HELLFIRE,
    1800, 18, MONS_DRAGON, MONS_SERPENT_OF_HELL, MH_DEMONIC, 180,
    { {AT_BITE, AF_FIRE, 35}, {AT_CLAW, AF_PLAIN, 15},
      {AT_TRAMPLE, AF_PLAIN, 15}, AT_NO_ATK },
    { 20, 0, 0, 280 },
    16, 12, MST_SERPENT_OF_HELL, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_DOORS, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SERPENT_OF_HELL_COCYTUS, 'D', ETC_ICE, "the Serpent of Hell",
    M_SEE_INVIS | M_UNIQUE,
    MR_RES_POISON | mrd(MR_RES_COLD, 3),
    1800, 18, MONS_DRAGON, MONS_SERPENT_OF_HELL, MH_DEMONIC, 180,
    { {AT_BITE, AF_COLD, 35}, {AT_CLAW, AF_PLAIN, 15},
      {AT_TRAMPLE, AF_PLAIN, 15}, AT_NO_ATK },
    { 20, 0, 0, 280 },
    20, 12, MST_SERPENT_OF_HELL, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_DOORS, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SERPENT_OF_HELL_DIS, 'D', ETC_IRON, "the Serpent of Hell",
    M_SEE_INVIS | M_UNIQUE,
    MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD,
    1800, 18, MONS_DRAGON, MONS_SERPENT_OF_HELL, MH_DEMONIC, 180,
    { {AT_BITE, AF_REACH, 45}, {AT_CLAW, AF_PLAIN, 25},
      {AT_TRAMPLE, AF_PLAIN, 25}, {AT_TAIL_SLAP, AF_PLAIN, 15} },
    { 20, 0, 0, 320 },
    30, 8, MST_SERPENT_OF_HELL, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_DOORS, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

{
    MONS_SERPENT_OF_HELL_TARTARUS, 'D', ETC_DEATH, "the Serpent of Hell",
    M_SEE_INVIS | M_UNIQUE,
    MR_RES_POISON | mrd(MR_RES_COLD, 2),
    1800, 18, MONS_DRAGON, MONS_SERPENT_OF_HELL, MH_DEMONIC, 180,
    { {AT_BITE, AF_DRAIN_XP, 35}, {AT_CLAW, AF_PLAIN, 15},
      {AT_TRAMPLE, AF_PLAIN, 15}, AT_NO_ATK },
    { 20, 0, 0, 320 },
    16, 12, MST_SERPENT_OF_HELL, CE_NOCORPSE, S_ROAR,
    I_HIGH, HT_LAND, FL_WINGED, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_DOORS, SIZE_GIANT, MON_SHAPE_QUADRUPED_WINGED
},

// "e"lves
{
    MONS_DUVESSA, 'e', BLUE, "Duvessa",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 0, 0, 35 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DOWAN, 'e', RED, "Dowan",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    450, 10, MONS_ELF, MONS_ELF, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 25 },
    0, 13, MST_DOWAN, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FANNAR, 'e', LIGHTBLUE, "Fannar",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    450, 16, MONS_ELF, MONS_ELF, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 8}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 80 },
    0, 13, MST_FANNAR, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "F"rogs.
{
    MONS_PRINCE_RIBBIT, 'F', LIGHTCYAN, "Prince Ribbit",
    M_UNIQUE | M_COLD_BLOOD | M_SPEAKS | M_MALE
        | M_PHASE_SHIFT,
    MR_NO_FLAGS,
    450, 12, MONS_BLINK_FROG, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 0, 0, 40 },
    0, 16, MST_PRINCE_RIBBIT, CE_CLEAN, S_CROAK,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 14, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_QUADRUPED_TAILLESS
},

// "g"oblins, gnolls, and dwarves.
{
    MONS_IJYB, 'g', BLUE, "Ijyb",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    400, 5, MONS_GOBLIN, MONS_GOBLIN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 4}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 28 },
    2, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    MONS_ROBIN, 'g', LIGHTCYAN, "Robin",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    500, 10, MONS_GOBLIN, MONS_HOBGOBLIN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 0, 0, 16 },
    1, 8, MST_ROBIN, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_GRUM, 'g', LIGHTRED, "Grum",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    680, 19, MONS_GNOLL, MONS_GNOLL, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 0, 0, 40 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_GROWL,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_CRAZY_YIUF, 'g', COLOUR_UNDEF, "Crazy Yiuf",
    M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_NO_FLAGS,
    680, 10, MONS_GNOLL, MONS_GNOLL, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 20 },
    2, 9, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_WIGLAF, 'g', LIGHTGREEN, "Wiglaf",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 8, MONS_DWARF, MONS_DWARF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 17, 0, 0, 140 },
    1, 10, MST_BK_OKAWARU, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_JORGRUN, 'g', LIGHTMAGENTA, "Jorgrun",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_NO_REGEN | M_MALE,
    MR_NO_FLAGS,
    600, 8, MONS_DWARF, MONS_DEEP_DWARF, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 120 },
    2, 15, MST_JORGRUN, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "h" - carnivorous quadrupeds
{
    MONS_NATASHA, 'h', MAGENTA, "Natasha",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_FEMALE,
    MR_NO_FLAGS,
    200, 10, MONS_FELID, MONS_FELID, MH_NATURAL, 20,
    { {AT_CLAW, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 5, 0, 0 },
    2, 12, MST_NATASHA, CE_CLEAN, S_HISS,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(8),
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_QUADRUPED
},

// "H"ybrids.
{
    MONS_ARACHNE, 'H', LIGHTCYAN, "Arachne",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_WEB_SENSE | M_FEMALE,
    MR_NO_FLAGS, // no rPois- (breathes through the human half)
    900, 10, MONS_SPIDER, MONS_ARACHNE, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 17, 0, 0, 200 },
    3, 10, MST_ARACHNE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_ARACHNID
},

{
    MONS_SOJOBO, 'Q', LIGHTGREEN, "Sojobo",
    M_FIGHTER | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_NO_FLAGS,
    550, 18, MONS_TENGU, MONS_TENGU, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 28}, {AT_PECK, AF_PLAIN, 14}, {AT_CLAW, AF_PLAIN, 14},
       AT_NO_ATK },
    { 20, 0, 0, 150 },
    2, 22, MST_SOJOBO, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, MOVE_ENERGY(9),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ASTERION, 'H', LIGHTBLUE, "Asterion",
    M_HYBRID | M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_UNIQUE | M_MALE,
    MR_NO_FLAGS,
    900, 15, MONS_MINOTAUR, MONS_MINOTAUR, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 35}, {AT_GORE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 130 },
    4, 4, MST_ASTERION, CE_CLEAN, S_BELLOW,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// Spr"i"ggans.
{
    MONS_AGNES, 'i', LIGHTCYAN, "Agnes",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_FEMALE,
    MR_NO_FLAGS,
    200, 20, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 0, 0, 100 },
    0, 20, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 18, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    MONS_THE_ENCHANTRESS, 'i', LIGHTMAGENTA, "the Enchantress",
    M_WARM_BLOOD | M_SPEAKS | M_SEE_INVIS | M_UNIQUE | M_PHASE_SHIFT | M_FEMALE,
    MR_RES_FIRE | MR_RES_COLD,
    200, 35, MONS_SPRIGGAN, MONS_SPRIGGAN, MH_NATURAL, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 100 },
    1, 28, MST_THE_ENCHANTRESS, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, MOVE_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

// "J"ellies.
{
    MONS_ROYAL_JELLY, 'J', YELLOW, "the royal jelly",
    M_SEE_INVIS | M_UNBLINDABLE | M_ACID_SPLASH | M_UNIQUE,
    MR_RES_POISON | MR_RES_ASPHYX | mrd(MR_RES_ACID, 3),
    550, 20, MONS_JELLY, MONS_JELLY, MH_NATURAL, 180,
    { {AT_HIT, AF_ACID, 50}, {AT_HIT, AF_ACID, 30}, AT_NO_ATK, AT_NO_ATK },
    { 21, 0, 0, 230 },
    8, 4, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_MEDIUM, MON_SHAPE_BLOB
},

{
    MONS_DISSOLUTION, 'J', LIGHTMAGENTA, "Dissolution",
    M_UNIQUE | M_SEE_INVIS | M_UNBLINDABLE | M_ACID_SPLASH | M_BURROWS
        | M_SPEAKS | M_FAST_REGEN | M_MALE,
    MR_RES_POISON | MR_RES_ASPHYX | mrd(MR_RES_ACID, 3),
    550, 60, MONS_JELLY, MONS_JELLY, MH_NATURAL, 120,
    { {AT_HIT, AF_ACID, 50}, {AT_HIT, AF_ACID, 30}, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 180 },
    10, 1, MST_DISSOLUTION, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_ITEMS, SIZE_LARGE, MON_SHAPE_BLOB
},

// Snails and other gastropods.
{
    MONS_GASTRONOK, 'w', MAGENTA, "Gastronok",
    M_UNIQUE | M_SEE_INVIS | M_SPEAKS | M_NO_WAND | M_MALE,
    MR_NO_FLAGS,
    1800, 10, MONS_ELEPHANT_SLUG, MONS_ELEPHANT_SLUG, MH_NATURAL, 80,
    { {AT_BITE, AF_PLAIN, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 0, 0, 150 },
    2, 1, MST_GASTRONOK, CE_POISONOUS, S_GURGLE,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 5, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_BIG, MON_SHAPE_SNAIL
},

// "K"obolds.
{
    MONS_SONJA, 'K', RED, "Sonja",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    400, 12, MONS_KOBOLD, MONS_KOBOLD, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 9}, {AT_HIT, AF_PLAIN, 5}, {AT_HIT, AF_PLAIN, 5},
       AT_NO_ATK },
    { 6, 0, 0, 30 },
    2, 24, MST_SONJA, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_SMALL, MON_SHAPE_HUMANOID
},

{
    // XP modifier is very high to compensate for 4 created-friendly humans
    MONS_PIKEL, 'K', BLUE, "Pikel",
    M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_NO_FLAGS,
    500, 32, MONS_KOBOLD, MONS_BIG_KOBOLD, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 0, 0, 40 },
    4, 12, MST_NO_SPELLS, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "L"iches.
{
    // May be re-spawned after his death.
    MONS_BORIS, 'L', RED, "Boris",
    M_UNIQUE | M_SEE_INVIS | M_SPEAKS | M_MALE,
    mrd(MR_RES_COLD, 2) | MR_RES_ELEC,
    275, 15, MONS_LICH, MONS_LICH, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 25}, {AT_TOUCH, AF_DRAIN_XP, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 22, 0, 0, 154 },
    12, 10, MST_BORIS, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "M"ummies.
{
    MONS_MENKAURE, 'M', MAGENTA, "Menkaure",
    M_UNIQUE | M_SPEAKS | M_SEE_INVIS | M_MALE,
    MR_VUL_FIRE | MR_RES_COLD,
    550, 48, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, 20,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 24 },
    3, 6, MST_MENKAURE, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 8, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_KHUFU, 'M', LIGHTRED, "Khufu",
    M_SEE_INVIS | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_RES_COLD | MR_RES_ELEC,
    550, 20, MONS_MUMMY, MONS_MUMMY, MH_UNDEAD, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 240 },
    10, 6, MST_KHUFU, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "m"erfolk.
{
    MONS_ILSUIW, 'm', LIGHTGREEN, "Ilsuiw",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    500, 10, MONS_MERFOLK, MONS_MERFOLK, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 150 },
    5, 18, MST_ILSUIW, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_AMPHIBIOUS, FL_NONE, 10, SWIM_ENERGY(6),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "N"agas.
{
    MONS_VASHNIA, 'N', LIGHTCYAN, "Vashnia",
    M_SEE_INVIS | M_WARM_BLOOD | M_FEMALE
        | M_ARCHER | M_DONT_MELEE | M_PHASE_SHIFT | M_SPEAKS | M_UNIQUE,
    MR_RES_POISON,
    1000, 14, MONS_NAGA, MONS_NAGA, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 25}, {AT_CONSTRICT, AF_CRUSH, 7}, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 160 },
    6, 18, MST_VASHNIA, CE_POISONOUS, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, MOVE_ENERGY(14),
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_NAGA
},

// "O"gres.
{
    MONS_EROLCHA, 'O', LIGHTBLUE, "Erolcha",
    M_UNIQUE | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    1300, 26, MONS_OGRE, MONS_OGRE, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 0, 0, 54 },
    3, 7, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// "o"rcs.
{
    MONS_BLORK_THE_ORC, 'o', BROWN, "Blork the orc",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    600, 15, MONS_ORC, MONS_ORC, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 32 },
    0, 9, MST_ORC_WIZARD_III, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_URUG, 'o', RED, "Urug",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    600, 15, MONS_ORC, MONS_ORC, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 0, 0, 66 },
    2, 13, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_NERGALLE, 'o', WHITE, "Nergalle",
    M_UNIQUE | M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    600, 12, MONS_ORC, MONS_ORC, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 60 },
    9, 11, MST_NERGALLE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SAINT_ROKA, 'o', LIGHTBLUE, "Saint Roka",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    600, 15, MONS_ORC, MONS_ORC, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 200 },
    3, 10, MST_SAINT_ROKA, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// Dwarves

// Rakshasas and demons.
{
    MONS_AZRAEL, 'R', LIGHTRED, "Azrael",
    M_UNIQUE | M_GLOWS_LIGHT | M_SPEAKS | M_MALE,
    MR_RES_POISON | MR_RES_HELLFIRE | MR_VUL_COLD | MR_VUL_WATER,
    1300, 12, MONS_EFREET, MONS_EFREET, MH_DEMONIC, 40,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 11, 0, 0, 88 },
    10, 5, MST_AZRAEL, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_MARA, 'R', LIGHTMAGENTA, "Mara",
    M_SEE_INVIS | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_RES_POISON | mrd(MR_RES_FIRE, 2),
    550, 25, MONS_RAKSHASA, MONS_RAKSHASA, MH_DEMONIC, 140,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 140 },
    10, 14, MST_MARA, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// "S"nakes and guardian serpents.
{
    MONS_AIZUL, 'S', LIGHTMAGENTA, "Aizul",
    M_SEE_INVIS | M_WARM_BLOOD | M_SPEAKS | M_UNIQUE | M_MALE,
    MR_RES_POISON,
    800, 10, MONS_GUARDIAN_SERPENT, MONS_GUARDIAN_SERPENT, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 25}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 142 },
    8, 18, MST_AIZUL, CE_MUTAGEN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 15, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_SNAKE
},

// "T"rolls.
{
    // Snorg can go berserk.
    MONS_SNORG, 'T', LIGHTGREEN, "Snorg",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FAST_REGEN | M_MALE,
    MR_NO_FLAGS,
    1500, 25, MONS_TROLL, MONS_TROLL, MH_NATURAL, 60,
    { {AT_BITE, AF_PLAIN, 20}, {AT_CLAW, AF_PLAIN, 15},
      {AT_CLAW, AF_PLAIN, 15}, AT_NO_ATK },
    { 8, 0, 0, 96 },
    0, 10, MST_SNORG, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_PURGY, 'T', GREEN, "Purgy",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FAST_REGEN | M_MALE,
    MR_NO_FLAGS,
    1100, 10, MONS_TROLL, MONS_TROLL, MH_NATURAL, 10,
    { {AT_BITE, AF_PLAIN, 9}, {AT_CLAW, AF_PLAIN, 4},
      {AT_CLAW, AF_PLAIN, 4}, AT_NO_ATK },
    { 5, 0, 0, 35 },
    1, 12, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// "V"ampires.
{
    MONS_JORY, 'V', LIGHTRED, "Jory",
    M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE
        | M_BLOOD_SCENT | M_WARM_BLOOD | M_UNIQUE,
    MR_RES_COLD,
    550, 13, MONS_VAMPIRE, MONS_VAMPIRE, MH_UNDEAD, 160,
    { {AT_HIT, AF_PLAIN, 40}, {AT_BITE, AF_VAMPIRIC, 15}, AT_NO_ATK,
       AT_NO_ATK },
    { 18, 0, 0, 180 },
    10, 15, MST_JORY, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// Elephants.
{
    MONS_NELLIE, 'Y', LIGHTMAGENTA, "Nellie",
    M_WARM_BLOOD | M_UNIQUE | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    2300, 8, MONS_ELEPHANT, MONS_HELLEPHANT, MH_DEMONIC, 140,
    { {AT_TRAMPLE, AF_PLAIN, 45}, {AT_BITE, AF_PLAIN, 20},
      {AT_GORE, AF_PLAIN, 15 }, AT_NO_ATK },
   { 20, 0, 0, 240 },
    13, 10, MST_HELLEPHANT, CE_CLEAN, S_TRUMPET,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_QUADRUPED
},

// Skulls and "z" undead.
{
    MONS_MURRAY, 'z', LIGHTRED, "Murray",
    M_UNIQUE | M_SEE_INVIS | M_SPEAKS | M_MALE,
    mrd(MR_RES_FIRE | MR_RES_ELEC | MR_RES_COLD, 2),
    75, 15, MONS_CURSE_SKULL, MONS_CURSE_SKULL, MH_UNDEAD, MAG_IMMUNE,
    { {AT_BITE, AF_PLAIN, 20}, {AT_BITE, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 160 },
    30, 10, MST_MURRAY, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_TINY, MON_SHAPE_MISC
},

// numbers!
{
    MONS_IGNACIO, '1', LIGHTMAGENTA, "Ignacio",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_RES_ELEC | MR_RES_FIRE | MR_RES_COLD | MR_RES_POISON,
    1300, 14, MONS_EXECUTIONER, MONS_EXECUTIONER, MH_DEMONIC, 160,
    { {AT_HIT, AF_PLAIN, 20}, {AT_HIT, AF_PLAIN, 10}, {AT_HIT, AF_PLAIN, 10},
      {AT_HIT, AF_PLAIN, 5} },
    { 18, 0, 0, 250 },
    10, 15, MST_IGNACIO, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 20, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_GRINDER, '5', BLUE, "Grinder",
    M_UNIQUE | M_SEE_INVIS | M_SPEAKS | M_NO_HT_WAND | M_MALE,
    MR_RES_POISON | mrd(MR_RES_COLD, 2),
    100, 11, MONS_SHADOW_IMP, MONS_SHADOW_IMP, MH_DEMONIC, 20,
    { {AT_HIT, AF_PAIN, 11}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 0, 0, 40 },
    3, 11, MST_GRINDER, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LITTLE, MON_SHAPE_HUMANOID
},

{
    // Roxanne obviously can't use items, but we want to equip her with
    // a spellbook, so MONUSE_STARTING_EQUIPMENT is necessary.
    MONS_ROXANNE, '8', BLUE, "Roxanne",
    M_ARTIFICIAL | M_UNIQUE | M_STATIONARY | M_SPEAKS | M_NO_WAND | M_FEMALE,
    mrd(MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD | MR_RES_ELEC, 2)
        | MR_RES_PETRIFY,
    1300, 10, MONS_STATUE, MONS_STATUE, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 180 },
    20, 0, MST_ROXANNE, CE_NOCORPSE, S_SILENT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// human uniques
{
    MONS_TERENCE, '@', LIGHTCYAN, "Terence",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 2, 0, 0, 20 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_JESSICA, '@', LIGHTGREY, "Jessica",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 125, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 0, 0, 10 },
    0, 10, MST_JESSICA, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_SIGMUND, '@', YELLOW, "Sigmund",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 10,
    { {AT_HIT, AF_PLAIN, 5}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 3, 0, 0, 30 },
    0, 11, MST_ORC_WIZARD_II, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_EDMUND, '@', RED, "Edmund",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 0, 0, 44 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_PSYCHE, '@', LIGHTMAGENTA, "Psyche",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 7}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 0, 0, 39 },
    0, 12, MST_ORC_WIZARD_III, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_DONALD, '@', BLUE, "Donald",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 26}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 84 },
    3, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_AMPHIBIOUS, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_JOSEPH, '@', CYAN, "Joseph",
    M_UNIQUE | M_FIGHTER | M_ARCHER | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 40,
    { {AT_HIT, AF_PLAIN, 15}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 6, 0, 0, 66 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_ERICA, '@', MAGENTA, "Erica",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 10}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 0, 0, 64 },
    0, 11, MST_WIZARD_II, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_JOSEPHINE, '@', WHITE, "Josephine",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    mrd(MR_RES_NEG, 3),
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 11}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 10, 0, 0, 69 },
    0, 10, MST_JOSEPHINE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_HAROLD, '@', LIGHTGREEN, "Harold",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_MALE
        | M_SPEAKS,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 60,
    { {AT_HIT, AF_PLAIN, 12}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 9, 0, 0, 76 },
    0, 8, MST_HAROLD, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MAUD, '@', RED, "Maud",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 32}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 0, 0, 118 },
    0, 10, MST_NO_SPELLS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_LOUISE, '@', LIGHTMAGENTA, "Louise",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 80,
    { {AT_HIT, AF_PLAIN, 17}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 13, 0, 0, 106 },
    0, 10, MST_WIZARD_IV, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FRANCES, '@', YELLOW, "Frances",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_FEMALE
        | M_SPEAKS,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 29}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 14, 0, 0, 121 },
    0, 10, MST_FRANCES, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_RUPERT, '@', LIGHTRED, "Rupert",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 12, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 21}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 123 },
    0, 10, MST_RUPERT, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_KIRKE, '@', LIGHTBLUE, "Kirke",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 100,
    { {AT_HIT, AF_PLAIN, 18}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 110 },
    0, 10, MST_KIRKE, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_NORRIS, '@', BROWN, "Norris",
    M_UNIQUE | M_FIGHTER | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 36}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 20, 0, 0, 214 },
    1, 9, MST_NORRIS, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_FREDERICK, '@', GREEN, "Frederick",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 12, MONS_DEMIGOD, MONS_DEMIGOD, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 27}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 21, 0, 0, 159 },
    0, 10, MST_FREDERICK, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MARGERY, '@', LIGHTRED, "Margery",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_SPEAKS | M_FEMALE,
    MR_NO_FLAGS,
    550, 15, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 140,
    { {AT_HIT, AF_PLAIN, 30}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 22, 0, 0, 164 },
    0, 10, MST_MARGERY, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_EUSTACHIO, '@', BLUE, "Eustachio",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 20, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_PLAIN, 6}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 4, 0, 0, 40 },
    0, 13, MST_EUSTACHIO, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_MAURICE, '@', GREEN, "Maurice",
    M_UNIQUE | M_WARM_BLOOD | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    550, 24, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 20,
    { {AT_HIT, AF_STEAL, 9}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 5, 0, 0, 60 },
    1, 13, MST_MAURICE, CE_CLEAN, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

{
    MONS_NIKOLA, '@', LIGHTCYAN, "Nikola",
    M_UNIQUE | M_WARM_BLOOD | M_SEE_INVIS | M_MALE
        | M_SPEAKS,
    MR_NO_FLAGS, // Xom would hate MR_RES_ELEC here.
    550, 10, MONS_HUMAN, MONS_HUMAN, MH_NATURAL, 120,
    { {AT_HIT, AF_PLAIN, 20}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 190 },
    1, 9, MST_NIKOLA, CE_CLEAN, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_HUMANOID
},

// unique major demons ('&')
{
    MONS_MNOLEG, '&', LIGHTGREEN, "Mnoleg",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    mrd(MR_RES_ELEC, 2) | MR_RES_POISON | MR_RES_FIRE,
    1300, 15, MONS_PANDEMONIUM_LORD, MONS_PANDEMONIUM_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_CLAW, AF_KLOWN, 40}, {AT_TENTACLE_SLAP, AF_MUTATE, 35},
      {AT_GORE, AF_VULN, 30}, {AT_KICK, AF_BLINK, 23} },
    { 17, 0, 0, 350 },
    10, 25, MST_MNOLEG, CE_NOCORPSE, S_BUZZ,
    I_HIGH, HT_LAND, FL_NONE, 13, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_LOM_LOBON, '&', LIGHTBLUE, "Lom Lobon",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_RES_POISON | MR_RES_FIRE | mrd(MR_RES_COLD | MR_RES_ELEC, 3),
    1300, 15, MONS_PANDEMONIUM_LORD, MONS_PANDEMONIUM_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_ANTIMAGIC, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 19, 0, 0, 360 },
    10, 20, MST_LOM_LOBON, CE_NOCORPSE, S_SCREAM,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_CEREBOV, '&', RED, "Cerebov",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_RES_POISON | MR_RES_HELLFIRE,
    1800, 15, MONS_PANDEMONIUM_LORD, MONS_PANDEMONIUM_LORD, MH_DEMONIC, 160,
    { {AT_HIT, AF_PLAIN, 60}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 21, 0, 0, 650 },
    30, 8, MST_CEREBOV, CE_NOCORPSE, S_SHOUT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_GIANT, MON_SHAPE_HUMANOID
},

{
    MONS_GLOORX_VLOQ, '&', LIGHTGREY, "Gloorx Vloq",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_RES_POISON | MR_RES_COLD | mrd(MR_RES_ELEC, 2),
    1300, 15, MONS_PANDEMONIUM_LORD, MONS_PANDEMONIUM_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 45}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 350 },
    10, 10, MST_GLOORX_VLOQ, CE_NOCORPSE, S_MOAN,
    I_HIGH, HT_LAND, FL_LEVITATE, 20, DEFAULT_ENERGY,
    MONUSE_OPEN_DOORS, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_GERYON, '&', GREEN, "Geryon",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_NO_FLAGS,
    1800, 15, MONS_HELL_LORD, MONS_HELL_LORD, MH_DEMONIC, 120,
    { {AT_TAIL_SLAP, AF_REACH, 35}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 15, 0, 0, 300 },
    15, 6, MST_GERYON, CE_NOCORPSE, S_ROAR,
    I_NORMAL, HT_LAND, FL_WINGED, 10, DEFAULT_ENERGY,
    MONUSE_STARTING_EQUIPMENT, MONEAT_NOTHING, SIZE_GIANT,
        MON_SHAPE_HUMANOID_WINGED
},

{
    MONS_DISPATER, '&', MAGENTA, "Dispater",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    mrd(MR_RES_ELEC, 3) | MR_RES_POISON | MR_RES_HELLFIRE | MR_RES_COLD,
    1300, 15, MONS_HELL_LORD, MONS_HELL_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 50}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 16, 0, 0, 450 },
    40, 3, MST_DISPATER, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_ASMODEUS, '&', LIGHTRED, "Asmodeus",
    M_UNIQUE | M_FIGHTER | M_SEE_INVIS | M_SPEAKS | M_MALE,
    MR_RES_ELEC | MR_RES_POISON | MR_RES_HELLFIRE,
    1300, 25, MONS_HELL_LORD, MONS_HELL_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_PLAIN, 50}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 17, 0, 0, 450 },
    30, 7, MST_ASMODEUS, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_LEVITATE, 10, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

{
    MONS_ERESHKIGAL, '&', WHITE, "Ereshkigal",
    M_UNIQUE | M_SEE_INVIS | M_SPEAKS | M_FEMALE,
    mrd(MR_RES_ELEC, 2) | MR_RES_POISON | MR_RES_COLD,
    1300, 15, MONS_HELL_LORD, MONS_HELL_LORD, MH_DEMONIC, MAG_IMMUNE,
    { {AT_HIT, AF_DRAIN_XP, 40}, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 18, 0, 0, 350 },
    10, 30, MST_ERESHKIGAL, CE_NOCORPSE, S_SHOUT,
    I_HIGH, HT_LAND, FL_NONE, 14, DEFAULT_ENERGY,
    MONUSE_WEAPONS_ARMOUR, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_HUMANOID
},

// Impossible to hit, impossible to damage, immune to everything,
// unkillable, just sits there doing nothing but casting Shadow Creatures
// over and over.
{
    MONS_TEST_SPAWNER, 'X', WHITE, "test spawner",
    M_STATIONARY | M_INSUBSTANTIAL | M_NO_POLY_TO
        | M_FAST_REGEN,
    mrd(MR_RES_ELEC | MR_RES_POISON | MR_RES_FIRE | MR_RES_COLD
        | MR_RES_ROTTING | MR_RES_ACID, 4) | MR_RES_STICKY_FLAME,
    0, 15, MONS_TEST_SPAWNER, MONS_TEST_SPAWNER, MH_NONLIVING, MAG_IMMUNE,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1000, 1000, 0, 0 },
    127, 127, MST_TEST_SPAWNER, CE_NOCORPSE, S_SILENT,
    I_PLANT, HT_AMPHIBIOUS, FL_NONE, 14, SWIM_ENERGY(6),
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_LARGE, MON_SHAPE_MISC
},

// an unspecified monster
{
    MONS_SENSED, '{', RED, "sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_SENSED_FRIENDLY, '{', GREEN, "friendly sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_SENSED_TRIVIAL, '{', BLUE, "trivial sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_SENSED_EASY, '{', LIGHTGRAY, "easy sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_SENSED_TOUGH, '{', YELLOW, "tough sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

{
    MONS_SENSED_NASTY, '{', LIGHTRED, "nasty sensed monster",
    M_CANT_SPAWN,
    MR_NO_FLAGS,
    0, 0, MONS_SENSED, MONS_SENSED, MH_NONLIVING, 0,
    { AT_NO_ATK, AT_NO_ATK, AT_NO_ATK, AT_NO_ATK },
    { 1, 1, 0, 0 },
    0, 0, MST_NO_SPELLS, CE_NOCORPSE, S_SILENT,
    I_NORMAL, HT_LAND, FL_NONE, 10, DEFAULT_ENERGY,
    MONUSE_NOTHING, MONEAT_NOTHING, SIZE_MEDIUM, MON_SHAPE_MISC
},

/*
  For simplicity, here again the explanation:
    - row 1: monster id, display character, display colour, name
    - row 2: monster flags
    - row 3: monster resistance flags
    - row 4: mass, experience modifier, genus, species, holiness, resist magic
    - row 5: damage for each of four attacks
    - row 6: hit dice, described by four parameters
    - row 7: AC, evasion, sec(spell), corpse_thingy, zombie size, shouts
    - row 8: intel, habitat, flight class, speed, energy_usage
    - row 9: gmon_use class, gmon_eat class, body size, body shape
*/

};
