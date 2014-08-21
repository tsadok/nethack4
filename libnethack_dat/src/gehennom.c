/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-08-25 */
/* nh4-scripts: LEVGEN */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "levelgen_dsl.h"
#include "gehennom_gen.h"

void
gen_valley(struct level *lev) {
    INIT_MAZE(lev);
    FILL_MAP(' ');
    
    NOTELEPORT;
    HARDFLOOR;
    NOMMAP;

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

    NON_DIGGABLE(MR(WHOLEMAP));

    { /* features */
        REGION(temple, R(3, 7, 7, 15));
        LIGHT_REGION(temple);
        PLACE_ALTAR(UNALIGNED, C(6, 11));
        MAKE_TEMPLE(temple);

        PLACE_DOOR(LOCKED, C(6, 2));
        PLACE_DOOR(LOCKED, C(10, 5));
        PLACE_DOOR(LOCKED, C(8, 7));

        REGION_ARRAY(morgues, R(21, 2, 26, 9), R(11, 15, 18, 19),
                              R(39, 10, 45, 15));

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
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_ARCHEOLOGIST));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_ARCHEOLOGIST));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_BARBARIAN));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_BARBARIAN));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_CAVEMAN));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_CAVEWOMAN));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_HEALER));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_HEALER));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_KNIGHT));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_KNIGHT));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_RANGER));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_RANGER));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_ROGUE));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_ROGUE));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_SAMURAI));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_SAMURAI));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_TOURIST));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_TOURIST));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_VALKYRIE));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_VALKYRIE));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_WIZARD));
            OBJ_EX(CORPSE, RANDOM_LOC, CORPSENM(PM_WIZARD));
        }

        OBJ(RUBY, RANDOM_LOC);
        REPEAT(4) {
            OBJ(RANDOM_OBJ_OF('[') RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF(')'), RANDOM_LOC);
        }

        REPEAT(3) {
            OBJ(RANDOM_OBJ_OF('!'), RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF('?'), RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF('('), RANDOM_LOC);
        }

        REPEAT(2) {
            OBJ(RANDOM_OBJ_OF('*'), RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF('/'), RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF('='), RANDOM_LOC);
            OBJ(RANDOM_OBJ_OF('+'), RANDOM_LOC);
        }
    }

    { /* traps */
        TRAP(SPIKED_PIT, C(7, 3));
        TRAP(SPIKED_PIT, C(16, 6));
        TRAP(SLP_GAS_TRAP, C(16, 6));
        TRAP(SQKY_BOARD, C(23, 13));
        TRAP(SQKY_BOARD, RANDOM_LOC);
        TRAP(DART_TRAP, C(62, 2));
        TRAP(DART_TRAP, C(28, 18));
        TRAP(ANTI_MAGIC, RANDOM_LOC);
        TRAP(ANTI_MAGIC, RANDOM_LOC);
        TRAP(MAGIC_TRAP, RANDOM_LOC);
        TRAP(MAGIC_TRAP, RANDOM_LOC);
    }

    { /* monsters */
        REPEAT(6)
            MON(PM_GHOST, RANDOM_LOC);

        REPEAT(4) {
            MON(RANDOM_MON_OF('Z'), RANDOM_LOC);
            MON(RANDOM_MON_OF('M'), RANDOM_LOC);
        }

        REPEAT(3) {
            MON(PM_VAMPIRE_BAT, RANDOM_LOC);
            MON(RANDOM_MON_OF('V'), RANDOM_LOC);
        }

        MON(RANDOM_MON_OF('L'), RANDOM_LOC);
    }

    RETURN_LEV;
}

void
gen_juiblex(struct level *lev) {
    INIT_MAZE(lev);

    MAKE_MAP('.', '}', SMOOTHED, JOINED, UNLIT, UNWALLED);

    NOTELEPORT;
    SHORTSIGHTED;

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
    MAKE_SWAMP(MR(mainmap));

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

    { /* stairs / teleports */
        REGION(downstair, R(0, 0, 10, 20));
        REGION(upstair, R(68, 0, 78, 20));

        TELEPORT_REGION(UP, downstair);
        TELEPORT_REGION(DOWN, upstair);
        STAIR_DOWN(IN(downstair));
        STAIR_UP(IN(upstair));

        IF_BRANCH_UP
            BRANCH_UP(IN(downstair))
    }

    { /* fountains / mimics */
        COORD_ARRAY(places,
                    REL(MR(mainmap), C(4, 2)), REL(MR(mainmap), C(46, 2)),
                    REL(MR(mainmap), C(4, 15)), REL(MR(mainmap), C(46, 15)));

        PLACE_FOUNTAIN(places[0]);

        MON_EX(PM_GIANT_MIMIC, places[1], MIMIC("fountain"));
        MON_EX(PM_GIANT_MIMIC, places[2], MIMIC("fountain"));
        MON_EX(PM_GIANT_MIMIC, places[3], MIMIC("fountain"));
    }


    { /* monsters */
        MON(PM_JUIBLEX, REL(MR(mainmap), C(25, 8)));

        MON(PM_LEMURE, REL(MR(mainmap), C(43, 8)));
        MON(PM_LEMURE, REL(MR(mainmap), C(44, 8)));
        MON(PM_LEMURE, REL(MR(mainmap), C(45, 8)));

        CHAR_ARRAY(rndmons, 'j', 'b', 'P', 'F');
        MON(RANDOM_MON_OF(rndmons[0]), REL(MR(mainmap), C(25, 6)));
        MON(RANDOM_MON_OF(rndmons[1]), REL(MR(mainmap), C(24, 7)));
        MON(RANDOM_MON_OF(rndmons[2]), REL(MR(mainmap), C(26, 7)));
        MON(RANDOM_MON_OF(rndmons[3]), REL(MR(mainmap), C(23, 8)));
        MON(RANDOM_MON_OF(rndmons[3]), REL(MR(mainmap), C(27, 8)));
        MON(RANDOM_MON_OF(rndmons[2]), REL(MR(mainmap), C(24, 9)));
        MON(RANDOM_MON_OF(rndmons[1]), REL(MR(mainmap), C(26, 9)));
        MON(RANDOM_MON_OF(rndmons[0]), REL(MR(mainmap), C(25, 10)));

        REPEAT(4) {
            MON(RANDOM_MON_OF('j'), IN(MR(mainmap)));
            MON(RANDOM_MON_OF('P'), IN(MR(mainmap)));
        }

        REPEAT(3) {
            MON(RANDOM_MON_OF('b'), IN(MR(mainmap)));
            MON(RANDOM_MON_OF('F'), IN(MR(mainmap)));
        }

        REPEAT(2) {
            MON(RANDOM_MON_OF('m'), IN(MR(mainmap)));
            MON(PM_JELLYFISH, IN(MR(mainmap)));
        }
    }

    { /* objects */
        OBJ(BOULDER, IN(MR(mainmap)));
        OBJ(BOULDER, IN(MR(upisland)));
        OBJ(BOULDER, IN(MR(downisland)));

        REPEAT(3) {
            OBJ(RANDOM_OBJ_OF('!'), IN(MR(mainmap)));
            OBJ(RANDOM_OBJ_OF('%'), IN(MR(mainmap)));
        }
    }

    { /* traps */
        REPEAT(2) {
            TRAP(SLEEP_GAS, IN(MR(mainmap)));
            TRAP(ANTI_MAGIC, IN(MR(mainmap)));
            TRAP(MAGIC, IN(MR(mainmap)));
        }
    }

  RETURN_LEV;
}

void
gen_orcus(struct level *lev) {
    INIT_MAZE(lev);
    FILL_MAP(' ');

    NOTELEPORT;
    SHORTSIGHTED;

    SUBMAP(intown, C(45, 17),
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

    MAZEWALK(REL(MR(intown), C(0, 6)), WEST);
    DARKEN_REGION(MR(intown));
    MAKE_ROOM(MR(intown));

    { /* stairs / teleports */
        REGION(leftside, R(0, 0, 11, 20));

        TELEPORT_REGION(BOTH, leftside);

        STAIR_UP(IN(leftside));
        STAIR_DOWN(REL(MR(intown), C(33, 15)));

        IF_BRANCH_UP
            BRANCH_UP(IN(leftside));
    }

    { /* boulders for destroyed walls */
        COORD_ARRAY(boulders, C(19, 2), C(20, 2), C(21, 2), C(36, 2), C(36, 3),
                              C(6, 4), C(5, 5), C(6, 5), C(7, 5), C(39, 5),
                              C(8, 8), C(9, 8), C(10, 8), C(11, 8), C(6, 10),
                              C(11, 9), C(6, 10), C(5, 11), C(6, 11), C(7, 11),
                              C(21, 11), C(21, 12), C(13, 13), C(14, 13),
                              C(15, 13), C(14, 14));

        FOR_EACH_COORD(boulder, boulders) {
            OBJ('`', "boulder", REL(MR(intown), boulder));
        }
    }

    { /* features */
        PLACE_DOOR(CLOSED, REL(MR(intown), C(23, 2)));
        PLACE_DOOR(OPEN, REL(MR(intown), C(31, 3)));
        PLACE_DOOR(GONE, REL(MR(intown), C(3, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(9, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(14, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(41, 5)));
        PLACE_DOOR(OPEN, REL(MR(intown), C(3, 8)));
        PLACE_DOOR(GONE, REL(MR(intown), C(13, 8)));
        PLACE_DOOR(OPEN, REL(MR(intown), C(41, 8)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(24, 9)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(31, 11)));
        PLACE_DOOR(OPEN, REL(MR(intown), C(11, 13)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(18, 13)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(41, 13)));
        PLACE_DOOR(OPEN, REL(MR(intown), C(26, 14)));
        PLACE_DOOR(CLOSED, REL(MR(intown), C(6, 15)));

        PLACE_ALTAR(UNALIGNED, REL(MR(intown), C(24, 7)));

        REGION_ARRAY(shops, REL_REGION(MR(intown), R(32, 9, 37, 12)),
                            REL_REGION(MR(intown), R(12, 0, 15, 4)));
        FOR_EACH_REGION(shop, shops) {
            LIGHT_REGION(shop);
            MAKE_ABANDONED_SHOP(shop);
        }

        MAKE_MORGUE(REL_REG(MR(intown), R(22, 12, 25, 16)));
    }

    { /* traps */
        TRAP(SPIKED_PIT, IN(MR(intown)));
        TRAP(SLEEP_GAS, IN(MR(intown)));
        TRAP(ANTI_MAGIC, IN(MR(intown)));
        REPEAT(3)
            TRAP(FIRE, IN(MR(intown)));
        REPEAT(2)
            TRAP(MAGIC, IN(MR(intown)));
    }

    REPEAT(10)
        OBJ(RANDOM_OBJ, RANDOM_LOC);

    { /* monsters */
        MON(PM_ORCUS, REL(MR(intown), C(33, 15)));
        MON(PM_HUMAN_ZOMBIE, REL(MR(intown), C(32, 15)));
        MON(PM_SHADE, REL(MR(intown), C(32, 14)));
        MON(PM_SHADE, REL(MR(intown), C(32, 16)));
        MON(PM_VAMPIRE, REL(MR(intown), C(35, 16)));
        MON(PM_VAMPIRE, REL(MR(intown), C(35, 14)));
        MON(PM_VAMPIRE_LORD, REL(MR(intown), C(36, 14)));
        MON(PM_VAMPIRE_LORD, REL(MR(intown), C(36, 15)));

        REPEAT(5) {
            MON(PM_SKELETON, IN(MR(intown)));
            MON(RANDOM_MON, RANDOM_LOC);
        }

        REPEAT(4)
            MON(PM_SHADE, IN(MR(intown)));

        REPEAT(3) {
            MON(PM_GNOME_ZOMBIE, IN(MR(intown)));
            MON(PM_ETTIN_ZOMBIE, IN(MR(intown)));
            MON(PM_HUMAN_ZOMBIE, IN(MR(intown)));
            MON(PM_VAMPIRE, IN(MR(intown)));
        }

        REPEAT(2)
            MON(PM_VAMPIRE_LORD, IN(MR(intown)));
    }

  RETURN_LEV;
}
