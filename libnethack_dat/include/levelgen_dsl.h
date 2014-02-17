/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-02-17 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef LEVGEN_DSL_H
#define LEVGEN_DSL_H

#include "hack.h"

struct coord {
    int x, y;
};

struct area {
    int tlx, tly, brx, bry;
};

struct maparea {
    struct area area;

    const char * map;
};

enum justification {
    j_start,
    j_end,
    j_half_start,
    j_center,
};

enum cardinal_dir {
    cd_north,
    cd_south,
    cd_west,
    cd_east,
};

/* Note that some of these macros are declarations!
 *
 * It is important that declaration macros NOT use the do-while idiom so that
 * the declaration remains visible outside the block. Using these inside a naked
 * "if" or similar statement is likely an error anyway, so the usual reason for
 * the idiom does not apply.
 */

/* =============
 * Helper macros
 */

/* ==============================
 * Declaractions and flow control
 */
#define INIT_MAZE(lev) \
    struct level *lev_ = lev; \
    int i_; \
    lev->flags.is_maze_lev = TRUE \

#define MAP(map) \
    SUBMAP(map_, C(COLNO - 1, ROWNO - 1), map); \
    PLACE_AT(map_, C(0, 0))
    
#define SUBMAP(name, size, map) \
    struct maparea * name = new_map(size, map)

#define REGION(name, reg) \
    struct area name = (reg)

#define REGION_ARRAY(name, ...) \
    struct area name[] = { __VA_ARGS__ }; \
    lg_shuffle_array(name, sizeof name / sizeof *name, sizeof *name)
#define COORD_ARRAY(name, ...) \
    struct coord name[] = { __VA_ARGS__ }; \
    lg_shuffle_array(name, sizeof name / sizeof *name, sizeof *name)
#define CHAR_ARRAY(name, ...) \
    char name[] = { __ VA_ARGS__ }; \
    lg_shuffle_array(name, sizeof name / sizeof *name, sizeof *name)

#define RETURN_LEV(...) TODO
#define REPEAT(n) \
    for (i_ = 0; i_ < n; ++i_)

/* ====================================================
 * Statements and functions with an effect on the level
 */

#define FILL_MAP(c) \
    do { lg_fill_map(lev_, c, __LINE__, __FILE__); } while (0)

#define MAKE_MAP(...) TODO

#define NOTELEPORT   do { lev_->flags.noteleport   = TRUE; } while (0)
#define HARDFLOOR    do { lev_->flags.hardfloor    = TRUE; } while (0)
#define NOMMAP       do { lev_->flags.nommap       = TRUE; } while (0)
#define SHORTSIGHTED do { lev_->flags.shortsighted = TRUE; } while (0)
#define ARBOREAL     do { lev_->flags.arboreal     = TRUE; } while (0)

#define MAZEWALK(...) TODO
#define PLACE_JUSTIFIED(...) TODO
#define PLACE_AT(...) TODO

#define TRAP(...) TODO
#define OBJ(...) TODO
#define OBJ_EX(...) TODO
#define MON(...) TODO
#define MON_EX(...) TODO

#define FILL_IRREGULAR(...) TODO
#define LIGHT_REGION(...) TODO
#define DARKEN_REGION(...) TODO
#define NON_DIGGABLE(...) TODO

#define TELEPORT_REGION(...) TODO
#define STAIR_UP(...) TODO
#define STAIR_DOWN(...) TODO
#define IF_BRANCH_UP(...) TODO
#define BRANCH_UP(...) TODO

#define FOR_EACH_COORD(...) TODO
#define FOR_EACH_REGION(...) TODO

#define PLACE_DOOR(...) TODO
#define PLACE_FOUNTAIN(...) TODO
#define PLACE_ALTAR(...) TODO

#define MAKE_ROOM(...) TODO
#define MAKE_TEMPLE(...) TODO
#define MAKE_MORGUE(...) TODO
#define MAKE_ABANDONED_SHOP(...) TODO
#define MAKE_SWAMP(...) TODO

/* ============================================
 * Functions to shorthand coordinates & areas
 */

#define C(x, y) ((struct coord){(x), (y)})
#define R(tlx, tly, brx, bry) ((struct area){(tlx), (tly), (brx), (bry)})
#define REL(rel, coord) \
    (C((coord).x + (rel).tlx, \
       (coord).y + (rel).tly))
#define REL_REG(rel, reg) \
    (R((reg).tlx + (rel).tlx, \
       (reg).tly + (rel).tly, \
       (reg).brx + (rel).tlx, \
       (reg).bry + (rel).tly))
#define IN(reg) \
    C((reg).tlx + rn2((reg).brx - (reg).tlx), \
      (reg).tly + rn2((reg).bry - (reg).tly))
#define MR(map) ((map).area)

/* ================================
 * Functions for use in expressions
 */

#define RANDOM_MON_OF TODO
#define RANDOM_OBJ_OF TODO

#define MIMIC TODO
#define CORPSENM TODO

/* ==========================================
 * Various constants and constant-like macros
 */

#define WHOLEMAP (map_)

#define UNALIGNED (AM_NONE)
#define LAWFUL (AM_LAWFUL)
#define NEUTRAL (AM_NEUTRAL)
#define CHAOTIC (AM_CHAOTIC)

#define GONE (D_NODOOR)
#define BROKEN (D_BROKEN)
#define OPEN (D_ISOPEN)
#define CLOSED (D_CLOSED)
#define LOCKED (D_LOCKED)
#define TRAPPED (D_TRAPPED)

#define RANDOM_LOC (C(rn2(COLNO), rn2(ROWNO)))
#define RANDOM_OBJ TODO
#define RANDOM_MON TODO

#define SMOOTHED (TRUE)
#define UNSMOOTHED (FALSE)
#define JOINED (TRUE)
#define UNJOINED (FALSE)
#define LIT (TRUE)
#define UNLIT (FALSE)
#define WALLED (TRUE)
#define UNWALLED (FALSE)

#define UP (LR_UPTELE)
#define DOWN (LR_DOWNTELE)
#define BOTH (LR_TELE)

#define LEFT (j_start)
#define RIGHT (j_end)
#define TOP (j_start)
#define BOTTOM (j_end)
#define CENTER (j_center)
#define HALF_LEFT (j_half_left)

#define NORTH (cd_north)
#define SOUTH (cd_south)
#define EAST (cd_east)
#define WEST (cd_west)

/* ==========================================================
 * Real functions that haven't been moved to source files yet
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
char
lg_what_map_char(char c) {
    switch (c) {
    case ' ':
        return STONE;
    case '#':
        return CORR;
    case '.':
        return ROOM;
    case '-':
        return HWALL;
    case '|':
        return VWALL;
    case '+':
        return DOOR;
    case 'A':
        return AIR;
    case 'B':
        return CROSSWALL;       /* hack: boundary location */
    case 'C':
        return CLOUD;
    case 'S':
        return SDOOR;
    case 'H':
        return SCORR;
    case '{':
        return FOUNTAIN;
    case '\\':
        return THRONE;
    case 'K':
        return SINK;
    case '}':
        return MOAT;
    case 'P':
        return POOL;
    case 'L':
        return LAVAPOOL;
    case 'I':
        return ICE;
    case 'W':
        return WATER;
    case 'T':
        return TREE;
    case 'F':
        return IRONBARS;        /* Fe = iron */
    }
    return INVALID_TYPE;
}

void
lg_fill_map(struct level *lev, char c, int line, const char * file) {
    int val = lg_what_map_char(c);

    if (val == INVALID_TYPE) {
        impossible("Invalid fill character at line %d of %s", line, file);
        val = STONE;
    }

    int x, y;
    for (x = 0; x < COLNO; ++x)
        for (y = 0; y < ROWNO; ++y)
            lev->locations[x][y].typ = val;
}

void
lg_shuffle_array(void *ptr, size_t num, size_t size) {
    char tmp[size];
    int i, j;

    for (i = 0; i < num - 1; ++i) {
        j = rn2(num - i) + i;

        if (j == i)
            continue;

        memcpy(tmp, (char*)ptr + j * size, size);
        memcpy((char*)ptr + j * size, (char*)ptr + i * size, size);
        memcpy((char*)ptr + i * size, tmp, size);
    }
}

struct maparea *
new_map (struct coord size, const char *text) {
    return 0;
}
#pragma clang diagnostic pop

#endif
