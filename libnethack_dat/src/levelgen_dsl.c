/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-11-18 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "levelgen_dsl.h"
#include "pm.h"

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
lg_fill_map(struct level *lev, char c) {
    int val = lg_what_map_char(c);

    if (val == INVALID_TYPE) {
        impossible("invalid fill character '%c' in fill", c);
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
lg_new_map(struct coord size, const char *text, struct maparea **chain) {

    int len = strlen(text);
    if (len != size.x * size.y)
        /* This is not a recoverable error. The only sane thing would be to
         * return NULL, which will give a crash. Better exit gracefully. */
        panic("map diagram is %d characters, expected %dx%d", len,
              size.x, size.y);

    char *buf = malloc(len + 1);
    memset(buf, ' ', len);
    buf[len] = '\0';
    memcpy(buf, text, len);

    char *c;
    for (c = buf; c < buf + len; ++c) {
        char typ = lg_what_map_char(*c);
        if (typ == INVALID_TYPE) {
            impossible("invalid fill character '%c' in map", *c);
            *c = ' ';
        } else
            *c = typ;
    }

    struct maparea *map = malloc(sizeof(struct maparea));
    map->locs = buf;
    map->area.lx = map->area.ly = 0;
    map->area.hx = size.x - 1;
    map->area.hy = size.y - 1;
    map->nextmap = *chain;
    *chain = map;
    return map;
}

void
lg_place_map(struct level *lev, struct maparea *map, struct coord loc) {
    /* Reset the area position so that it can be used as a baseline */
    map->area.hx -= map->area.lx;
    map->area.hy -= map->area.ly;
    map->area.lx = loc.x;
    map->area.ly = loc.y;
    map->area.hx += loc.x;
    map->area.hy += loc.y;

    if (map->area.lx < 0 || map->area.ly < 0 ||
        map->area.hx >= COLNO || map->area.hy >= ROWNO)
        panic("map area does not fit on map!");

    int x, y;
    for (x = map->area.lx; x <= map->area.hx; ++x) {
        for (y = map->area.ly; y <= map->area.hy; ++y) {
            int ix = (y - map->area.ly) * (map->area.hx - map->area.lx + 1) +
                     (x - map->area.lx);
            lev->locations[x][y].typ = map->locs[ix];
            lev->locations[x][y].flags = 0;
            lev->locations[x][y].horizontal = 0;
        }
    }
}

/* This attempts to generate a monster of the specified id and place it
 * near the location specified. It will do its best to meet the requirements,
 * but will fall back to a random monster if a genocided monster type was
 * chosen.
 */
struct monst *lg_gen_monster(struct level *lev, short id, struct zone zn) {
    return NULL;
#if 0
    if (id < LOW_PM || id >= NUMMONS || is_placeholder(&mons[id])) {
        /* NON_PM means it was requested hy the level to generate no monster. */
        if (id != NON_PM)
            impossible("invalid monster id in monster generation: %d", id);
        return NULL;
    }

    const struct permonst *pm = &mons[id];
    uchar mvf = mvitals[id].mvflags;
    /* If we wanted a unique, and it's already been generated, ignore it. */
    if ((pm->geno & G_UNIQ) && (mvf & G_EXTINCT))
        return NULL;
    /* We will not respect exinction, but we will respect genocide. Extinction
     * is not respected because either the id came about as a request for a
     * specific class in the levelgen function, in which case we should honour
     * it if possible, or it came from the level manager, which knows better
     * than we whether to respect extinction. If the monster is genocided, we
     * will instead attempt to (once) generate a monster under the same rules,
     * and abort if we fail. */
    if (mvf & G_GENO) {
        id = lev->mgr->pick_monster(lev, 0, TRUE);
        if (id < LOW_PM || id >= NUMMONS || is_placeholder(&mons[id])) {
            if (id != NON_PM)
                impossible("level manager gave invalid monster id: %d", id);
            return NULL;
        }

        pm = &mons[id];
        mvf = mvitals[id].mvflags;
        if (((pm->geno & G_UNIQ) && (mvf & G_EXTINCT)) || (mvf & G_GENO))
            return NULL;
    }

    /* FIXME: a) the hardcoded role boundaries are a hack.
     *        b) this check ought to be elsewhere. Not sure where, but Not Here.
     */
    struct monst *mtmp = NULL;
    if (id >= PM_ARCHEOLOGIST && id <= PM_WIZARD)
        mtmp = mk_mplayer(pm, lev, loc.x, loc.y, FALSE);
    else
        mtmp = makemon(pm, lev, loc.x, loc.y, MM_ADJACENTOK);

    return mtmp;
#endif
}

/* Place a restricted-teleport region on the map. There are a bunch of things
 * wrong with this and it needs to be rethought, but this will work in the
 * interim.
 * FIXME: Fix all the things wrong with this.
 */
void
lg_tele_region(struct level *lev, char dir, struct zone zn) {
#if 0
    if (dir != LR_UPTELE && dir != LR_DOWNTELE && dir != LR_TELE) {
        impossible("invalid direction for teleport region!");
        return;
    }

    if (dir == LR_UPTELE || dir == LR_TELE) {
        lev->updest.lx = reg.lx;
        lev->updest.ly = reg.ly;
        lev->updest.hx = reg.hx;
        lev->updest.hy = reg.hy;

        lev->updest.nlx = lev->updest.nhx = COLNO;
        lev->updest.nly = lev->updest.nhy = ROWNO;
    }

    if (dir == LR_DOWNTELE || dir == LR_TELE) {
        lev->dndest.lx = reg.lx;
        lev->dndest.ly = reg.ly;
        lev->dndest.hx = reg.hx;
        lev->dndest.hy = reg.hy;

        lev->dndest.nlx = lev->dndest.nhx = COLNO;
        lev->dndest.nly = lev->dndest.nhy = ROWNO;
    }
#endif
}

/* Determine if we can place a portal here, even if there's a trap present. This
 * is just occupied()/bad_location(), but with a weaker trap check on the second
 * pass.
 */
static boolean
portal_ok(struct coord c, void *arg) {
    struct level *lev = arg;
    struct trap *ttmp = NULL;
    if ((ttmp = t_at(lev, c.x, c.y)) && (ttmp->ttyp == MAGIC_PORTAL ||
                                         ttmp->ttyp != VIBRATING_SQUARE))
        return FALSE;

    schar typ = lev->locations[c.x][c.y].typ;
    /* FIXME: The reliance on maze here means that we must go after the maze
     * flag, and that's ugly. */
    if ((typ == CORR && lev->flags.is_maze_lev) || typ == ROOM || typ == AIR ||
        typ == ICE)
        return TRUE;

    return FALSE;
}

/* FIXME: put this somewhere waaaaay better */
static boolean
has_trap(struct coord c, void *lev) {
    return !!t_at(lev, c.x, c.y);
}

/* Place a portal somewhere in the given region. We will try really hard to do
 * so, first by looking for a place we can safely put it, and if we can't find
 * one, then we'll replace a trap to make it work. We could theoretically be
 * even *more* agressive and start changing location types, but that would
 * require knowledge about the level (do we use AIR? ROOM? CORR?).
 */
void 
lg_place_portal(struct level *lev, const char *dest, struct zone zn) {
    s_level *sp = find_level(dest);
    if (!sp) {
        impossible("unable to find destination level of portal: %s", dest);
    }

    struct zone znp = zn_intersect(zn, zn_pred(portal_ok, lev));
    if (zn_is_empty(znp)) {
        impossible("unable to find location to place portal");
        return;
    }

    struct zone znt = zn_pred(has_trap, lev);

    zn = zn_intersect(znp, znt);
    
    if (zn_is_empty(zn))
        zn = znp;

    struct coord c = zn_rand(zn);

    if (!mkportal(lev, c.x, c.y, sp->lev))
        impossible("unable to place portal");
}

/* levelgen_dsl.c */
