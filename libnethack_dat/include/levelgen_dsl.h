/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-05 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef LEVGEN_DSL_H
#define LEVGEN_DSL_H

#include "coord.h"
#include "hack.h"
#include "rm.h"
#include "rect.h"
#include "zone.h"

struct maparea {
    struct rect area;
    char * locs;
    struct maparea *nextmap;
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
#define INIT_LEVGEN(lev) \
    struct level *lev_ = (lev); \
    { \
        if (!lev_->mgr) \
            impossible("generated level must have a manager"); \
        int x_, y_; \
        for (x_ = 0; x_ < ROWNO; ++x_) { \
            for (y_ = 0; y_ < COLNO; ++y_) { \
                lev->locations[x_][y_].typ = INVALID_TYPE; \
                lev->locations[x_][y_].lit = FALSE; \
                lev->locations[x_][y_].flags = 0; \
                lev->locations[x_][y_].horizontal = 0; \
                lev->locations[x_][y_].waslit = 0; \
                lev->locations[x_][y_].roomno = 0; \
                lev->locations[x_][y_].edge = 0; \
            } \
        } \
        lev_->flags.noteleport   = FALSE; \
        lev_->flags.hardfloor    = FALSE; \
        lev_->flags.nommap       = FALSE; \
        lev_->flags.shortsighted = FALSE; \
        lev_->flags.arboreal     = FALSE; \
        lev_->flags.is_maze_lev  = FALSE; \
        lev_->flags.hero_memory  = TRUE; \
    } \
    struct maparea *mapchain_ = NULL; \
    int i_

#define MAP(map) \
    SUBMAP(wholemap_, C(COLNO, ROWNO), (map)); \
    PLACE_AT(wholemap_, C(0, 0))
    
#define SUBMAP(name, size, map) \
    struct maparea *name = lg_new_map((size), (map), &mapchain_)

#define ZONE(name, zn) \
    struct zone name = (zn)
#define RECT(name, r) \
    struct rect name = (r)

#define ZONE_ARRAY(name, ...) \
    struct zone name[] = { __VA_ARGS__ }; \
    lg_shuffle_array(name, sizeof (name) / sizeof (*(name)), sizeof (*(name)))
#define COORD_ARRAY(name, ...) \
    struct coord name[] = { __VA_ARGS__ }; \
    lg_shuffle_array((name), sizeof (name) / sizeof (*(name)), sizeof (*(name)))
#define CHAR_ARRAY(name, ...) \
    char name[] = { __VA_ARGS__ }; \
    lg_shuffle_array(name, sizeof (name) / sizeof (*(name)), sizeof (*(name)))

#define FINISH_LEV \
    do { \
        while (mapchain_) { \
            struct maparea *map_ = mapchain_; \
            mapchain_ = map_->nextmap; \
            free(map_->locs); \
            free(map_); \
        } \
        int x_, y_; \
        for (x_ = 0; x_ < ROWNO; ++x_) { \
            for (y_ = 0; y_ < COLNO; ++y_) { \
                if (lev_->locations[x_][y_].typ == CROSSWALL) \
                    lev_->locations[x_][y_].typ = ROOM; \
                if (lev_->locations[x_][y_].typ == LAVAPOOL) \
                    lev_->locations[x_][y_].lit = 1; \
                if (lev_->locations[x_][y_].typ == HWALL || \
                    lev_->locations[x_][y_].typ == IRONBARS) \
                    lev_->locations[x_][y_].horizontal = 1; \
            } \
        } \
    } while (0)

#define REPEAT(n) \
    for (i_ = 0; i_ < (n); ++i_)

#define FOR_EACH_COORD(...) /* TODO */
#define FOR_EACH_ZONE(...) /* TODO */

#define IF_BRANCH_UP /* TODO */
#define IF_BRANCH_DOWN /* TODO */

/* ====================================================
 * Statements and functions with an effect on the level
 */

#define FILL_MAP(c) \
    do { lg_fill_map(lev_, (c)); } while (0)

#define MAKE_MAP(...) /* TODO */

#define MAZE         do { lev_->flags.is_maze_lev  = TRUE;  } while (0)
#define NOTELEPORT   do { lev_->flags.noteleport   = TRUE;  } while (0)
#define HARDFLOOR    do { lev_->flags.hardfloor    = TRUE;  } while (0)
#define NOMMAP       do { lev_->flags.nommap       = TRUE;  } while (0)
#define SHORTSIGHTED do { lev_->flags.shortsighted = TRUE;  } while (0)
#define ARBOREAL     do { lev_->flags.arboreal     = TRUE;  } while (0)
#define FORGETFUL    do { lev_->flags.hero_memory  = FALSE; } while (0)

#define MAZEWALK(...) /* TODO */
#define PLACE_JUSTIFIED(...) /* TODO */
#define PLACE_AT(map, loc) do { lg_place_map(lev_, (map), (loc)); } while (0)

#define TRAP(...) /* TODO */
#define OBJ(...) /* TODO */
#define OBJ_EX(...) /* TODO */
#define MON(id, loc, ...) do { \
        struct monst *mon_ = lg_gen_monster(lev_, (id), (loc)); \
        __VA_ARGS__; \
    } while (0)

#define FILL_IRREGULAR(...) /* TODO */
#define LIGHT_ZONE(...) /* TODO */
#define DARKEN_ZONE(...) /* TODO */
#define NON_DIGGABLE(...) /* TODO */

#define TELEPORT_REGION(dir, reg) \
    do { lg_tele_region(lev_, (dir), (reg)); } while (0)

/* These all take regions, not positions, because of difficulty guaranteeing
 * that a single position is good. You can use a region containing a single
 * coordinate to specify an exact position */
#define STAIR_UP(...) /* TODO */
#define STAIR_DOWN(...) /* TODO */
#define BRANCH_UP(...) /* TODO */
#define BRANCH_DOWN(...) /* TODO */
#define PORTAL(dest, reg) \
    do { lg_place_portal(lev_, (dest), (reg)); } while (0)

#define PLACE_DOOR(mask, loc) /* TODO */
#define PLACE_SDOOR(mask, loc) /* TODO */
#define PLACE_FOUNTAIN(loc) /* TODO */
#define PLACE_ALTAR(align, loc) /* TODO */

#define MAKE_ROOM(...) /* TODO */
#define MAKE_TEMPLE(...) /* TODO */
#define MAKE_MORGUE(...) /* TODO */
#define MAKE_ABANDONED_SHOP(...) /* TODO */
#define MAKE_SWAMP(...) /* TODO */

/* ============================================
 * Functions to shorthand coordinates & areas
 */

#define C(x, y) ((struct coord){(x), (y)})
#define ZC(x, y) (zn_loc((struct coord){(x), (y)}))
#define R(lx, ly, hx, hy) ((struct rect){(lx), (ly), (hx), (hy)})
#define Z(rect) (zn_rect((rect)))
#define ZR(lx, ly, hx, hy) (Z(R((lx), (ly), (hx), (hy))))

#define MR(map) ((map)->area)
#define MZ(map) (Z(MR(map)))

#define REL(rect, zn) \
    (zn_shift((zn), (struct coord){(rect).lx, (rect).ly}))

/* ================================
 * Functions for use in expressions
 */

#define RANDOM_MON_OF(c) \
    (lev_->mgr->pick_monster(lev_, def_char_to_monclass((c)), TRUE))
#define RANDOM_OBJ_OF(c) /* TODO */

#define MIMIC /* TODO */
#define HOSTILE (mon_ ? (mon_->mpeaceful = 0, set_malign(mon_)) : (void) 0)
#define CORPSENM /* TODO */

/* ==========================================
 * Various constants and constant-like macros
 */

#define WHOLEMAP (wholemap_)
#define EVERYWHERE (zn_whole())

#define UNALIGNED (AM_NONE)
#define LAWFUL (AM_LAWFUL)
#define NEUTRAL (AM_NEUTRAL)
#define CHAOTIC (AM_CHAOTIC)

#define GONE (D_NODOOR)
#define BROKEN (D_BROKEN)
#define OPEN (D_ISOPEN)
#define CLOSED (D_CLOSED)
#define LOCKED (D_LOCKED | D_CLOSED)
#define TRAPPED (D_TRAPPED | D_CLOSED)
#define LOCKED_TRAPPED (D_TRAPPED | D_LOCKED | D_CLOSED)

#define RANDOM_OBJ /* TODO */
#define RANDOM_MON (lev_->mgr->pick_monster(lev_, 0, TRUE))

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

char lg_what_map_char(char c);
char lg_what_mon_char(char c);
void lg_fill_map(struct level *lev, char c);
void lg_shuffle_array(void *ptr, size_t num, size_t size);
struct maparea *lg_new_map(struct coord size, const char *text,
                           struct maparea **chain);
void lg_place_map(struct level *lev, struct maparea *map, struct coord loc);
struct monst *lg_gen_monster(struct level *lev, short id, struct zone zn);
void lg_tele_region(struct level *lev, char dir, struct zone zn);
void lg_place_portal(struct level *lev, const char *dest, struct zone zn);

#endif
