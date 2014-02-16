/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-02-15 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include  "rm.h"
#include "levelgen_dsl.h"

void
gen_valley(struct level *lev) {
  INIT_MAZE(lev, "valley", ' ');
  
  NON_DIGGABLE(C(0, 0), C(75, 19));
  NOTELEPORT;
  HARDFLOOR;
  NOMMAP;

  { /* map */
    MAP("                                                                              "
        "  ----------------------------------------------------------------------------"
        "  |...S.|..|.....|  |.....-|      |................|   |...............| |...|"
        "  |---|.|.--.---.|  |......--- ----..........-----.-----....---........---.-.|"
        "  |   |.|.|..| |.| --........| |.............|   |.......---| |-...........--|"
        "  |   |...S..| |.| |.......-----.......------|   |--------..---......------- |"
        "  |----------- |.| |-......| |....|...-- |...-----................----       |"
        "  |.....S....---.| |.......| |....|...|  |..............-----------          |"
        "  |.....|.|......| |.....--- |......---  |....---.......|                    |"
        "  |.....|.|------| |....--   --....-- |-------- ----....---------------      |"
        "  |.....|--......---BBB-|     |...--  |.......|    |..................|      |"
        "  |..........||........-|    --...|   |.......|    |...||.............|      |"
        "  |.....|...-||-........------....|   |.......---- |...||.............--     |"
        "  |.....|--......---...........--------..........| |.......---------...--    |"
        "  |.....| |------| |--.......--|   |..B......----- -----....| |.|  |....---  |"
        "  |.....| |......--| ------..| |----..B......|       |.--------.-- |-.....---|"
        "  |------ |........|  |.|....| |.....----BBBB---------...........---.........|"
        "  |       |........|  |...|..| |.....|  |-.............--------...........---|"
        "  |       --.....-----------.| |....-----.....----------     |.........----  |"
        "  |        |..|..B...........| |.|..........|.|              |.|........|    |"
        "  ----------------------------------------------------------------------------");
  }

  { /* features */
    REGION(temple, C(3, 7), C(7, 15));
    LIGHT_REGION(temple);
    ALTAR(UNALIGNED, C(5, 11));
    MAKE_TEMPLE(temple);

    DOOR(LOCKED, C(4, 1));
    DOOR(LOCKED, C(8, 4));
    DOOR(LOCKED, C(6, 6));

    REGION_ARRAY(morgues, (R(21, 2, 26, 9), R(11, 15, 18, 19),
                           R(39, 10, 45, 15)));

    FOR_EACH_REGION(morgue, morgues) {
      LIGHT_REGION(morgue);
      FILL_IRREGULAR(morgue);
      MAKE_MORGUE(morgue);
    }
  }

  { /* stairs & teleports */
    STAIR_DOWN(C(3, 2));
    BRANCH_UP(C(68, 18));

    TELEPORT_REGION(DOWN, R(60, 10, 74, 19));
  }

  { /* objects */
    { /* adventurer corpses */
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("archeologist"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("archeologist"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("barbarian"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("barbarian"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("caveman"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("cavewoman"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("healer"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("healer"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("knight"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("knight"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("ranger"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("ranger"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("rogue"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("rogue"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("samurai"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("samurai"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("tourist"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("tourist"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("valkyrie"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("valkyrie"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("wizard"));
      OBJ_EX('%', "corpse", RANDOM_LOC, CORPSE("wizard"));
    }

    OBJ('*', "ruby", RANDOM_LOC);
    REPEAT(4) {
      OBJ('[', RANDOM_TYPE, RANDOM_LOC);
      OBJ(')', RANDOM_TYPE, RANDOM_LOC);
    }

    REPEAT(3) {
      OBJ('!', RANDOM_TYPE, RANDOM_LOC);
      OBJ('?', RANDOM_TYPE, RANDOM_LOC);
      OBJ('(', RANDOM_TYPE, RANDOM_LOC);
    }

    REPEAT(2) {
      OBJ('*', RANDOM_TYPE, RANDOM_LOC);
      OBJ('/', RANDOM_TYPE, RANDOM_LOC);
      OBJ('=', RANDOM_TYPE, RANDOM_LOC);
      OBJ('+', RANDOM_TYPE, RANDOM_LOC);
    }
  }

  { /* traps */
    TRAP(SPIKED_PIT, C(7, 3));
    TRAP(SPIKED_PIT, C(16, 6));
    TRAP(SLEEP_GAS, C(16, 6);
    TRAP(BOARD, C(23, 13));
    TRAP(BOARD, RANDOM_LOC);
    TRAP(DART, C(62, 2));
    TRAP(DART, C(28, 18));
    TRAP(ANTI_MAGIC, RANDOM_LOC);
    TRAP(ANTI_MAGIC, RANDOM_LOC);
    TRAP(MAGIC, RANDOM_LOC);
    TRAP(MAGIC, RANDOM_LOC);
  }

  { /* monsters */
    REPEAT(6)
      MON('W', "ghost", RANDOM_LOC);

    REPEAT(4) {
      MON('Z', RANDOM_TYPE, RANDOM_LOC);
      MON('M', RANDOM_TYPE, RANDOM_LOC);
    }

    REPEAT(3) {
      MON('B', "vampire bat", RANDOM_LOC);
      MON('V', RANDOM_TYPE, RANDOM_LOC);
    }

    MON('L', RANDOM_TYPE, RANDOM_LOC);
  }

  RETURN_LEV;
}

void
gen_juiblex(struct level *lev) {
  INIT_MAZE(lev, "juiblex", ' ');

  MAKE_MAP('.', '}', SMOOTHED, JOINED, UNLIT, OPEN);

  NOTELEPORT;
  SHORTSIGHTED;

  { /* maps */
    SUBMAP(mainmap, C(51, 18),
      "..}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.."
      ".}}}.}}}}}..}}}..}}}}}..}}}..}}}}}..}}}..}}}}}.}}}."
      "}}}...}}..}}.}.}}.}}.}}}...}}}.}}}..}}}..}}}}...}}}"
      ".}}}.}}.}}}.}}.}}.}}...}}.}}.....}}.....}....}.}}}."
      "..}}}..}}}.}}.}}.}}..}}.....}}.}}}.}}.}}}}}}}}}}}.."
      ".}}}..}}}}}.}}.}}.}}...}}}}}.....}}.}}}}}}.....}}}."
      "}}}..}}...}}..}}.}}}.}}}...}}}.}}}.}.}}}}..P.P..}}}"
      "}}.}}}}...}}}}}.}...}}}..P..}}}.}.}}}.}}}}.....}}}}"
      "}.}}}}.}}.}..}.}}}}}}}..P.P..}}}.}}}.}}..}}...}}}}."
      ".}}}}.}}}}....}}}}}.}}}..P..}}}.}}}}.}}..}}...}}}.}"
      "}}}}..}}.}}..}}}}...}}}}...}}}.}}}}}.}}}}.}}}}}}.}}"
      "}}}...}}...}}}..}}}}}}}}}}}}.....}}}}.}}...}..}.}}}"
      ".}}}..}}.}}}}....}}..}}}..}}.....}}}}.}}}.}....}}}."
      "..}}}.}}}}..}}..}}..}}..}}..}}.}}}..}.}..}}}..}}}.."
      ".}}}.}}}}....}}}}..}}....}}}}}}}...}}}....}}}}.}}}."
      "}}}...}}}....}}}..}}}....}}}..}}...}}}....}}}...}}}"
      ".}}}.}}}}}..}}}..}}}}}..}}}..}}}}}..}}}..}}}}}.}}}."
      "..}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}.}}}}}..");
    PLACE_JUSTIFIED(CENTER, CENTER, mainmap);
    MAKE_SWAMP(mainmap);

    SUBMAP(downisland, C(8, 5),
      "}}}}}}}}"
      "}}...}}}"
      "}}}...}}"
      "}}}}.}}}"
      "}}}}}}}}");
    PLACE_JUSTIFIED(LEFT, BOTTOM, downisland);

    SUBMAP(upisland, C(8, 5),
      "}}}}}}}}"
      "}}}}.}}}"
      "}}}...}}"
      "}}...}}}"
      "}}}}}}}}");
    PLACE_JUSTIFIED(RIGHT, TOP, upisland);
  }

  { /* stairs / teleports */
    REGION(downstair, R(1, 0, 11, 20));
    REGION(upstair, R(69, 0, 79, 20));

    TELEPORT_REGION(UP, downstair);
    TELEPORT_REGION(DOWN, upstair);
    STAIR_DOWN(IN(downstair));
    STAIR_UP(IN(upstair));

    IF_BRANCH_UP
      BRANCH_UP(IN(downstair))
  }

  { /* fountains / mimics */
    COORD_ARRAY(places, (REL(mainmap, C(4, 2)), REL(mainmap, C(46, 2)),
                         REL(mainmap, C(4, 15)), REL(mainmap, C(46, 15))));

    FOUNTAIN(AT(places[0]));

    MON_EX('m', "giant mimic", places[1], MIMIC("fountain"));
    MON_EX('m', "giant mimic", places[2], MIMIC("fountain"));
    MON_EX('m', "giant mimic", places[3], MIMIC("fountain"));
  }

  CHAR_ARRAY(rndmons, ('j', 'b', 'P', 'F'));

  { /* monsters */
    MON('&', "Juiblex", REL(mainmap, C(25, 8)));

    MON('i', "lemure", REL(mainmap, (C43, 8)));
    MON('i', "lemure", REL(mainmap, (C44, 8)));
    MON('i', "lemure", REL(mainmap, (C45, 8)));

    MON(rndmons[0], RANDOM_TYPE, REL(mainmap, C(25, 6)));
    MON(rndmons[1], RANDOM_TYPE, REL(mainmap, C(24, 7)));
    MON(rndmons[2], RANDOM_TYPE, REL(mainmap, C(26, 7)));
    MON(rndmons[3], RANDOM_TYPE, REL(mainmap, C(23, 8)));
    MON(rndmons[3], RANDOM_TYPE, REL(mainmap, C(27, 8)));
    MON(rndmons[2], RANDOM_TYPE, REL(mainmap, C(24, 9)));
    MON(rndmons[1], RANDOM_TYPE, REL(mainmap, C(26, 9)));
    MON(rndmons[0], RANDOM_TYPE, REL(mainmap, C(25, 10)));

    REPEAT(4) {
      MON('j', RANDOM_TYPE, IN(mainmap));
      MON('P', RANDOM_TYPE, IN(mainmap));
    }

    REPEAT(3) {
      MON('b', RANDOM_TYPE, IN(mainmap));
      MON('F', RANDOM_TYPE, IN(mainmap));
    }

    REPEAT(2) {
      MON('m', RANDOM_TYPE, IN(mainmap));
      MON(';', "jellyfish", IN(mainmap));
    }
  }

  { /* objects */
    OBJ('`', "boulder", IN(mainmap));
    OBJ('`', "boulder", IN(upisland));
    OBJ('`', "boulder", IN(downisland));

    REPEAT(3) {
      OBJ('!', RANDOM_TYPE, IN(mainmap));
      OBJ('%', RANDOM_TYPE, IN(mainmap));
    }
  }

  { /* traps */
    REPEAT(2) {
      TRAP(SLEEP_GAS, IN(mainmap));
      TRAP(ANTI_MAGIC, IN(mainmap));
      TRAP(MAGIC, IN(mainmap));
    }
  }

  RETURN_LEV;
}

void
gen_orcus(struct level *lev) {
  INIT_MAZE(lev, "orcus", ' ');

  NOTELEPORT;
  SHORTSIGHTED;

  { /* map */
    SUBMAP(intown, C(45, 17)
    ".|....|....|....|..............|....|........"
    ".|....|....|....|..............|....|........"
    ".|....|....|....|--...-+-------|............."
    ".|....|....|....|..............+............."
    ".|.........|....|..............|....|........"
    ".--+-...-+----+--....-------...--------.-+---"
    ".....................|.....|................."
    ".....................|.....|................."
    ".--+----....-+---....|.....|...----------+---"
    ".|....|....|....|....---+---...|......|......"
    ".|.........|....|..............|......|......"
    ".----...---------.....-----....+......|......"
    ".|........................|....|......|......"
    ".----------+-...--+--|....|....----------+---"
    ".|....|..............|....+....|............."
    ".|....+.......|......|....|....|............."
    ".|....|.......|......|....|....|.............");
    PLACE_JUSTIFIED(RIGHT, CENTER, intown);

    MAZEWALK(REL(intown, C(0, 6)), WEST);
    DARKEN_REGION(intown);
    MAKE_ROOM(intown);
  }

  { /* stairs / teleports */
    REGION(leftside, R(1, 0, 12, 20));

    TELEPORT_REGION(BOTH, leftside);

    STAIR_UP(IN(leftside));
    STAIR_DOWN(REL(intown, C(33, 15)));

    IF_BRANCH_UP
      BRANCH_UP(IN(leftside));
  }

  { /* boulders for destroyed walls */
    COORD_ARRAY(boulders, (C(19, 2), C(20, 2), C(21, 2), C(36, 2), C(36, 3),
                           C(6, 4), C(5, 5), C(6, 5), C(7, 5), C(39, 5),
                           C(8, 8), C(9, 8), C(10, 8), C(11, 8), C(6, 10),
                           C(11, 9), C(6, 10), C(5, 11), C(6, 11), C(7, 11),
                           C(21, 11), C(21, 12), C(13, 13), C(14, 13), C(15, 13),
                           C(14, 14)));

    FOR_EACH_COORD(boulder, boulders) {
      OBJ('`', "boulder", REL(intown, boulder));
    }
  }

  { /* features */
    DOOR(CLOSED, REL(intown, C(23, 2)));
    DOOR(OPEN, REL(intown, C(31, 3)));
    DOOR(GONE, REL(intown, C(3, 5)));
    DOOR(CLOSED, REL(intown, C(9, 5)));
    DOOR(CLOSED, REL(intown, C(14, 5)));
    DOOR(CLOSED, REL(intown, C(41, 5)));
    DOOR(OPEN, REL(intown, C(3, 8)));
    DOOR(GONE, REL(intown, C(13, 8)));
    DOOR(OPEN, REL(intown, C(41, 8)));
    DOOR(CLOSED, REL(intown, C(24, 9)));
    DOOR(CLOSED, REL(intown, C(31, 11)));
    DOOR(OPEN, REL(intown, C(11, 13)));
    DOOR(CLOSED, REL(intown, C(18, 13)));
    DOOR(CLOSED, REL(intown, C(41, 13)));
    DOOR(OPEN, REL(intown, C(26, 14)));
    DOOR(CLOSED, REL(intown, C(6, 15)));

    ALTAR(UNALIGNED, REL(intown, C(24, 7)));

    REGION_ARRAY(shops, (REL_REGION(intown, R(32, 9, 37, 12)),
                         REL_REGION(intown, R(12, 0, 15, 4))));
    FOR_EACH_REGION(shop, shops) {
      LIGHT_REGION(shop);
      MAKE_ABANDONED_SHOP(shop);
    }

    MAKE_MORGUE(REL_REG(intown, R(22, 12, 25, 16)));
  }

  { /* traps */
    TRAP(SPIKED_PIT, IN(intown));
    TRAP(SLEEP_GAS, IN(intown));
    TRAP(ANTI_MAGIC, IN(intown));
    REPEAT(3)
      TRAP(FIRE, IN(intown));
    REPEAT(2)
      TRAP(MAGIC, IN(intown));
  }

  REPEAT(10)
    OBJ(RANDOM_CLASS, RANDOM_TYPE, RANDOM_LOC);

  { /* monsters */
    MON('&', "Orcus", REL(intown, C(33, 15)));
    MON('Z', "human zombie", REL(intown, C(32, 15)));
    MON('W', "shade", REL(intown, C(32, 14)));
    MON('W', "shade", REL(intown, C(32, 16)));
    MON('V', "vampire", REL(intown, C(35, 16)));
    MON('V', "vampire", REL(intown, C(35, 14)));
    MON('V', "vampire lord", REL(intown, C(36, 14)));
    MON('V', "vampire lord", REL(intown, C(36, 15)));

    REPEAT(5) {
      MON('Z', "skeleton", IN(intown));
      MON(RANDOM_MONLET, RANDOM_TYPE, RANDOM_LOC);
    }

    REPEAT(4)
      MON('W', "shade", IN(intown));

    REPEAT(3) {
      MON('Z', "giant zombie", IN(intown));
      MON('Z', "ettin zombie", IN(intown));
      MON('Z', "human zombie", IN(intown));
      MON('V', "vampire", IN(intown));
    }

    REPEAT(2)
      MON('V', "vampire lord", IN(intown));
  }

  RETURN_LEV;
}

