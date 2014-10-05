/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-05 */
/* nh4-scripts: LEVGEN */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "levelgen_dsl.h"
#include "gehennom_gen.h"

#pragma clang diagnostic ignored "-Wunused"

void
gen_valley(struct level *lev) {
    INIT_LEVGEN(lev);
    MAZE;
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

    NON_DIGGABLE(EVERYWHERE);

    { /* features */
        ZONE(temple, ZR(3, 7, 7, 15));
        LIGHT_ZONE(temple);
        PLACE_ALTAR(UNALIGNED, ZC(6, 11));
        MAKE_TEMPLE(temple);

        PLACE_SDOOR(LOCKED, ZC(6, 2));
        PLACE_SDOOR(LOCKED, ZC(10, 5));
        PLACE_SDOOR(LOCKED, ZC(8, 7));

        ZONE_ARRAY(morgues, ZR(21, 2, 26, 9), ZR(11, 15, 18, 19),
                            ZR(39, 10, 45, 15));

        FOR_EACH_ZONE(morgue, morgues) {
            LIGHT_ZONE(morgue);
            FILL_IRREGULAR(morgue);
            MAKE_MORGUE(morgue);
        }
    }

    { /* stairs & teleports */
        STAIR_DOWN(ZC(3, 2));
        BRANCH_UP(ZC(68, 18));

        TELEPORT_REGION(DOWN, ZR(60, 10, 74, 19));
    }

    { /* objects */
        { /* adventurer corpses */
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_ARCHEOLOGIST));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_ARCHEOLOGIST));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_BARBARIAN));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_BARBARIAN));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_CAVEMAN));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_CAVEWOMAN));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_HEALER));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_HEALER));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_KNIGHT));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_KNIGHT));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_RANGER));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_RANGER));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_ROGUE));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_ROGUE));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_SAMURAI));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_SAMURAI));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_TOURIST));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_TOURIST));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_VALKYRIE));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_VALKYRIE));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_WIZARD));
            OBJ(CORPSE, EVERYWHERE, CORPSENM(PM_WIZARD));
        }

        OBJ(RUBY, EVERYWHERE);
        REPEAT(4) {
            OBJ(RANDOM_OBJ_OF('[') EVERYWHERE);
            OBJ(RANDOM_OBJ_OF(')'), EVERYWHERE);
        }

        REPEAT(3) {
            OBJ(RANDOM_OBJ_OF('!'), EVERYWHERE);
            OBJ(RANDOM_OBJ_OF('?'), EVERYWHERE);
            OBJ(RANDOM_OBJ_OF('('), EVERYWHERE);
        }

        REPEAT(2) {
            OBJ(RANDOM_OBJ_OF('*'), EVERYWHERE);
            OBJ(RANDOM_OBJ_OF('/'), EVERYWHERE);
            OBJ(RANDOM_OBJ_OF('='), EVERYWHERE);
            OBJ(RANDOM_OBJ_OF('+'), EVERYWHERE);
        }
    }

    { /* traps */
        TRAP(SPIKED_PIT, ZC(7, 3));
        TRAP(SPIKED_PIT, ZC(16, 6));
        TRAP(SLP_GAS_TRAP, ZC(16, 6));
        TRAP(SQKY_BOARD, ZC(23, 13));
        TRAP(SQKY_BOARD, EVERYWHERE);
        TRAP(DART_TRAP, ZC(62, 2));
        TRAP(DART_TRAP, ZC(28, 18));
        TRAP(ANTI_MAGIC, EVERYWHERE);
        TRAP(ANTI_MAGIC, EVERYWHERE);
        TRAP(MAGIC_TRAP, EVERYWHERE);
        TRAP(MAGIC_TRAP, EVERYWHERE);
    }

    { /* monsters */
        REPEAT(6)
            MON(PM_GHOST, EVERYWHERE);

        REPEAT(4) {
            MON(RANDOM_MON_OF('Z'), EVERYWHERE);
            MON(RANDOM_MON_OF('M'), EVERYWHERE);
        }

        REPEAT(3) {
            MON(PM_VAMPIRE_BAT, EVERYWHERE);
            MON(RANDOM_MON_OF('V'), EVERYWHERE);
        }

        MON(RANDOM_MON_OF('L'), EVERYWHERE);
    }

    FINISH_LEV;
}

void
gen_juiblex(struct level *lev) {
    INIT_LEVGEN(lev);
    MAZE;

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
    MAKE_SWAMP(MZ(mainmap));

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
        ZONE(downstair, ZR(0, 0, 10, 20));
        ZONE(upstair, ZR(68, 0, 78, 20));

        TELEPORT_REGION(UP, downstair);
        TELEPORT_REGION(DOWN, upstair);
        STAIR_DOWN(downstair);
        STAIR_UP(upstair);

        IF_BRANCH_UP
            BRANCH_UP(downstair)
    }

    { /* fountains / mimics */
        ZONE_ARRAY(places,
                   REL(MR(mainmap), ZC(4, 2)), REL(MR(mainmap), ZC(46, 2)),
                   REL(MR(mainmap), ZC(4, 15)), REL(MR(mainmap), ZC(46, 15)));

        PLACE_FOUNTAIN(places[0]);

        MON(PM_GIANT_MIMIC, places[1], MIMIC("fountain"));
        MON(PM_GIANT_MIMIC, places[2], MIMIC("fountain"));
        MON(PM_GIANT_MIMIC, places[3], MIMIC("fountain"));
    }


    { /* monsters */
        MON(PM_JUIBLEX, REL(MR(mainmap), ZC(25, 8)));

        MON(PM_LEMURE, REL(MR(mainmap), ZC(43, 8)));
        MON(PM_LEMURE, REL(MR(mainmap), ZC(44, 8)));
        MON(PM_LEMURE, REL(MR(mainmap), ZC(45, 8)));

        CHAR_ARRAY(rndmons, 'j', 'b', 'P', 'F');
        MON(RANDOM_MON_OF(rndmons[0]), REL(MR(mainmap), ZC(25, 6)));
        MON(RANDOM_MON_OF(rndmons[1]), REL(MR(mainmap), ZC(24, 7)));
        MON(RANDOM_MON_OF(rndmons[2]), REL(MR(mainmap), ZC(26, 7)));
        MON(RANDOM_MON_OF(rndmons[3]), REL(MR(mainmap), ZC(23, 8)));
        MON(RANDOM_MON_OF(rndmons[3]), REL(MR(mainmap), ZC(27, 8)));
        MON(RANDOM_MON_OF(rndmons[2]), REL(MR(mainmap), ZC(24, 9)));
        MON(RANDOM_MON_OF(rndmons[1]), REL(MR(mainmap), ZC(26, 9)));
        MON(RANDOM_MON_OF(rndmons[0]), REL(MR(mainmap), ZC(25, 10)));

        REPEAT(4) {
            MON(RANDOM_MON_OF('j'), MZ(mainmap));
            MON(RANDOM_MON_OF('P'), MZ(mainmap));
        }

        REPEAT(3) {
            MON(RANDOM_MON_OF('b'), MZ(mainmap));
            MON(RANDOM_MON_OF('F'), MZ(mainmap));
        }

        REPEAT(2) {
            MON(RANDOM_MON_OF('m'), MZ(mainmap));
            MON(PM_JELLYFISH, MZ(mainmap));
        }
    }

    { /* objects */
        OBJ(BOULDER, MZ(mainmap));
        OBJ(BOULDER, IN(MR(upisland)));
        OBJ(BOULDER, IN(MR(downisland)));

        REPEAT(3) {
            OBJ(RANDOM_OBJ_OF('!'), MZ(mainmap));
            OBJ(RANDOM_OBJ_OF('%'), MZ(mainmap));
        }
    }

    { /* traps */
        REPEAT(2) {
            TRAP(SLEEP_GAS, MZ(mainmap));
            TRAP(ANTI_MAGIC, MZ(mainmap));
            TRAP(MAGIC, MZ(mainmap));
        }
    }

  FINISH_LEV;
}

void
gen_orcus(struct level *lev) {
    INIT_LEVGEN(lev);
    MAZE;
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

    MAZEWALK(REL(MR(intown), ZC(0, 6)), WEST);
    DARKEN_ZONE(MR(intown));
    MAKE_ROOM(MR(intown));

    { /* stairs / teleports */
        RECT(leftside, R(0, 0, 11, 20));

        TELEPORT_REGION(BOTH, Z(leftside));

        STAIR_UP(Z(leftside));
        STAIR_DOWN(REL(MR(intown), ZC(33, 15)));

        IF_BRANCH_UP
            BRANCH_UP(Z(leftside));
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
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(23, 2)));
        PLACE_DOOR(OPEN, REL(MR(intown), ZC(31, 3)));
        PLACE_DOOR(GONE, REL(MR(intown), ZC(3, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(9, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(14, 5)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(41, 5)));
        PLACE_DOOR(OPEN, REL(MR(intown), ZC(3, 8)));
        PLACE_DOOR(GONE, REL(MR(intown), ZC(13, 8)));
        PLACE_DOOR(OPEN, REL(MR(intown), ZC(41, 8)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(24, 9)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(31, 11)));
        PLACE_DOOR(OPEN, REL(MR(intown), ZC(11, 13)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(18, 13)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(41, 13)));
        PLACE_DOOR(OPEN, REL(MR(intown), ZC(26, 14)));
        PLACE_DOOR(CLOSED, REL(MR(intown), ZC(6, 15)));

        PLACE_ALTAR(UNALIGNED, REL(MR(intown), ZC(24, 7)));

        ZONE_ARRAY(shops, REL(MR(intown), ZR(32, 9, 37, 12)),
                          REL(MR(intown), ZR(12, 0, 15, 4)));
        FOR_EACH_ZONE(shop, shops) {
            LIGHT_ZONE(shop);
            MAKE_ABANDONED_SHOP(shop);
        }

        MAKE_MORGUE(REL(MR(intown), ZR(22, 12, 25, 16)));
    }

    { /* traps */
        TRAP(SPIKED_PIT, MZ(intown));
        TRAP(SLEEP_GAS, MZ(intown));
        TRAP(ANTI_MAGIC, MZ(intown));
        REPEAT(3)
            TRAP(FIRE, MZ(intown));
        REPEAT(2)
            TRAP(MAGIC, MZ(intown));
    }

    REPEAT(10)
        OBJ(RANDOM_OBJ, EVERYWHERE);

    { /* monsters */
        MON(PM_ORCUS, REL(MR(intown), ZC(33, 15)));
        MON(PM_HUMAN_ZOMBIE, REL(MR(intown), ZC(32, 15)));
        MON(PM_SHADE, REL(MR(intown), ZC(32, 14)));
        MON(PM_SHADE, REL(MR(intown), ZC(32, 16)));
        MON(PM_VAMPIRE, REL(MR(intown), ZC(35, 16)));
        MON(PM_VAMPIRE, REL(MR(intown), ZC(35, 14)));
        MON(PM_VAMPIRE_LORD, REL(MR(intown), ZC(36, 14)));
        MON(PM_VAMPIRE_LORD, REL(MR(intown), ZC(36, 15)));

        REPEAT(5) {
            MON(PM_SKELETON, MZ(intown));
            MON(RANDOM_MON, EVERYWHERE);
        }

        REPEAT(4)
            MON(PM_SHADE, MZ(intown));

        REPEAT(3) {
            MON(PM_GNOME_ZOMBIE, MZ(intown));
            MON(PM_ETTIN_ZOMBIE, MZ(intown));
            MON(PM_HUMAN_ZOMBIE, MZ(intown));
            MON(PM_VAMPIRE, MZ(intown));
        }

        REPEAT(2)
            MON(PM_VAMPIRE_LORD, MZ(intown));
    }

  FINISH_LEV;
}
