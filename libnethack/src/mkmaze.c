/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-11-18 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sp_lev.h"
#include "lev.h"        /* save & restore info */

#include "gehennom_gen.h" /* FIXME: temporary! */
#include "endgame_gen.h"

static boolean iswall(struct level *lev, int x, int y);
static boolean iswall_or_stone(struct level *lev, int x, int y);
static boolean is_solid(struct level *lev, int x, int y);
static int extend_spine(int[3][3], int, int, int);
static boolean okay(struct level *lev, int x, int y, int dir);
static void maze0xy(coord *);
static boolean put_lregion_here(struct level *lev, xchar, xchar, xchar, xchar,
                                xchar, xchar, xchar, boolean,
                                struct level *dest);
static void move(int *, int *, int);
static boolean bad_location(struct level *lev, xchar x, xchar y, xchar lx,
                            xchar ly, xchar hx, xchar hy);
static const char * waterbody_impl(xchar x, xchar y, boolean article);


static boolean
iswall(struct level *lev, int x, int y)
{
    int type;

    if (!isok(x, y))
        return FALSE;
    type = lev->locations[x][y].typ;
    return (IS_WALL(type) || IS_DOOR(type) || type == SDOOR ||
            type == IRONBARS);
}

static boolean
iswall_or_stone(struct level *lev, int x, int y)
{
    int type;

    /* out of bounds = stone */
    if (!isok(x, y))
        return TRUE;

    type = lev->locations[x][y].typ;
    return (type == STONE || IS_WALL(type) || IS_DOOR(type) || type == SDOOR ||
            type == IRONBARS);
}

/* return TRUE if out of bounds, wall or rock */
static boolean
is_solid(struct level *lev, int x, int y)
{
    return !isok(x, y) || IS_STWALL(lev->locations[x][y].typ);
}


/*
 * Return 1 (not TRUE - we're doing bit vectors here) if we want to extend
 * a wall spine in the (dx,dy) direction.  Return 0 otherwise.
 *
 * To extend a wall spine in that direction, first there must be a wall there.
 * Then, extend a spine unless the current position is surrounded by walls
 * in the direction given by (dx,dy).  E.g. if 'x' is our location, 'W'
 * a wall, '.' a room, 'a' anything (we don't care), and our direction is
 * (0,1) - South or down - then:
 *
 *      a a a
 *      W x W           This would not extend a spine from x down
 *      W W W           (a corridor of walls is formed).
 *
 *      a a a
 *      W x W           This would extend a spine from x down.
 *      . W W
 */
static int
extend_spine(int locale[3][3], int wall_there, int dx, int dy)
{
    int spine, nx, ny;

    nx = 1 + dx;
    ny = 1 + dy;

    if (wall_there) {   /* wall in that direction */
        if (dx) {
            if (locale[1][0] && locale[1][2] && /* EW are wall/stone */
                locale[nx][0] && locale[nx][2]) {      /* diag are wall/stone */
                spine = 0;
            } else {
                spine = 1;
            }
        } else {        /* dy */
            if (locale[0][1] && locale[2][1] && /* NS are wall/stone */
                locale[0][ny] && locale[2][ny]) {      /* diag are wall/stone */
                spine = 0;
            } else {
                spine = 1;
            }
        }
    } else {
        spine = 0;
    }

    return spine;
}


/*
 * Wall cleanup.  This function has two purposes: (1) remove walls that
 * are totally surrounded by stone - they are redundant.  (2) correct
 * the types so that they extend and connect to each other.
 */
void
wallification(struct level *lev, int x1, int y1, int x2, int y2)
{
    uchar type;
    int x, y;
    struct rm *loc;
    int bits;
    int locale[3][3];   /* rock or wall status surrounding positions */

    /*
     * Value 0 represents a free-standing wall.  It could be anything,
     * so even though this table says VWALL, we actually leave whatever
     * typ was there alone.
     */
    static const xchar spine_array[16] = {
        VWALL, HWALL, HWALL, HWALL,
        VWALL, TRCORNER, TLCORNER, TDWALL,
        VWALL, BRCORNER, BLCORNER, TUWALL,
        VWALL, TLWALL, TRWALL, CROSSWALL
    };

    /* sanity check on incoming variables */
    if (x1 < 0 || x2 >= COLNO || x1 > x2 || y1 < 0 || y2 >= ROWNO || y1 > y2)
        panic("wallification: bad bounds (%d,%d) to (%d,%d)", x1, y1, x2, y2);

    /* Step 1: change walls surrounded by rock to rock. */
    for (x = x1; x <= x2; x++)
        for (y = y1; y <= y2; y++) {
            loc = &lev->locations[x][y];
            type = loc->typ;
            if (IS_WALL(type) && type != DBWALL) {
                if (is_solid(lev, x - 1, y - 1) && is_solid(lev, x - 1, y) &&
                    is_solid(lev, x - 1, y + 1) && is_solid(lev, x, y - 1) &&
                    is_solid(lev, x, y + 1) && is_solid(lev, x + 1, y - 1) &&
                    is_solid(lev, x + 1, y) && is_solid(lev, x + 1, y + 1))
                    loc->typ = STONE;
            }
        }

    /*
     * Step 2: set the correct wall type.  We can't combine steps
     * 1 and 2 into a single sweep because we depend on knowing if
     * the surrounding positions are stone.
     */
    for (x = x1; x <= x2; x++)
        for (y = y1; y <= y2; y++) {
            loc = &lev->locations[x][y];
            type = loc->typ;
            if (!(IS_WALL(type) && type != DBWALL))
                continue;

            /* set the locations TRUE if rock or wall or out of bounds */
            locale[0][0] = iswall_or_stone(lev, x - 1, y - 1);
            locale[1][0] = iswall_or_stone(lev, x, y - 1);
            locale[2][0] = iswall_or_stone(lev, x + 1, y - 1);

            locale[0][1] = iswall_or_stone(lev, x - 1, y);
            locale[2][1] = iswall_or_stone(lev, x + 1, y);

            locale[0][2] = iswall_or_stone(lev, x - 1, y + 1);
            locale[1][2] = iswall_or_stone(lev, x, y + 1);
            locale[2][2] = iswall_or_stone(lev, x + 1, y + 1);

            /* determine if wall should extend to each direction NSEW */
            bits = (extend_spine(locale, iswall(lev, x, y - 1), 0, -1) << 3)
                | (extend_spine(locale, iswall(lev, x, y + 1), 0, 1) << 2)
                | (extend_spine(locale, iswall(lev, x + 1, y), 1, 0) << 1)
                | extend_spine(locale, iswall(lev, x - 1, y), -1, 0);

            /* don't change typ if wall is free-standing */
            if (bits)
                loc->typ = spine_array[bits];
        }
}

static boolean
okay(struct level *lev, int x, int y, int dir)
{
    move(&x, &y, dir);
    move(&x, &y, dir);
    if (x < 3 || y < 3 || x > x_maze_max || y > y_maze_max ||
        lev->locations[x][y].typ != 0)
        return FALSE;
    return TRUE;
}

/* find random starting point for maze generation */
static void
maze0xy(coord * cc)
{
    cc->x = 3 + 2 * rn2((x_maze_max >> 1) - 1);
    cc->y = 3 + 2 * rn2((y_maze_max >> 1) - 1);
    return;
}

/*
 * Bad if:
 *      pos is occupied OR
 *      pos is inside restricted region (lx,ly,hx,hy) OR
 *      NOT (pos is corridor and a maze level OR pos is a room OR pos is air)
 */
boolean
bad_location(struct level * lev, xchar x, xchar y, xchar lx, xchar ly, xchar hx,
             xchar hy)
{
    return ((boolean)
            (occupied(lev, x, y) || within_bounded_area(x, y, lx, ly, hx, hy) ||
             !((lev->locations[x][y].typ == CORR && lev->flags.is_maze_lev) ||
               lev->locations[x][y].typ == ROOM ||
               lev->locations[x][y].typ == AIR)));
}

/* pick a location in area (lx, ly, hx, hy) but not in (nlx, nly, nhx, nhy) */
/* and place something (based on rtype) in that region */
void
place_lregion(struct level *lev, xchar lx, xchar ly, xchar hx, xchar hy,
              xchar nlx, xchar nly, xchar nhx, xchar nhy, xchar rtype,
              struct level *dest)
{
    int trycnt;
    boolean oneshot;
    xchar x, y;

    if (lx == COLNO) {  /* default to whole level */
        /*
         * if there are rooms and this a branch, let place_branch choose
         * the branch location (to avoid putting branches in corridors).
         */
        if (rtype == LR_BRANCH && lev->nroom) {
            place_branch(lev, Is_branchlev(lev), COLNO, ROWNO);
            return;
        }

        lx = 1;
        hx = COLNO - 1;
        ly = 1;
        hy = ROWNO - 1;
    }

    /* first a probabilistic approach */
    oneshot = (lx == hx && ly == hy);
    for (trycnt = 0; trycnt < 200; trycnt++) {
        x = rn1((hx - lx) + 1, lx);
        y = rn1((hy - ly) + 1, ly);
        if (put_lregion_here
            (lev, x, y, nlx, nly, nhx, nhy, rtype, oneshot, dest))
            return;
    }

    /* then a deterministic one */
    oneshot = TRUE;
    for (x = lx; x <= hx; x++)
        for (y = ly; y <= hy; y++)
            if (put_lregion_here
                (lev, x, y, nlx, nly, nhx, nhy, rtype, oneshot, dest))
                return;

    impossible("Couldn't place lregion type %d!", rtype);
}

static boolean
put_lregion_here(struct level *lev, xchar x, xchar y, xchar nlx, xchar nly,
                 xchar nhx, xchar nhy, xchar rtype, boolean oneshot,
                 struct level * dest)
{
    if (bad_location(lev, x, y, nlx, nly, nhx, nhy)) {
        if (!oneshot) {
            return FALSE;       /* caller should try again */
        } else {
            /* Must make do with the only location possible; avoid failure due
               to a misplaced trap. It might still fail if there's a dungeon
               feature here. */
            struct trap *t = t_at(lev, x, y);

            if (t && t->ttyp != MAGIC_PORTAL && t->ttyp != VIBRATING_SQUARE)
                deltrap(lev, t);
            if (bad_location(lev, x, y, nlx, nly, nhx, nhy))
                return FALSE;
        }
    }
    switch (rtype) {
    case LR_TELE:
    case LR_UPTELE:
    case LR_DOWNTELE:
        /* "something" means the player in this case */
        if (MON_AT(lev, x, y)) {
            /* move the monster if no choice, or just try again */
            if (oneshot)
                rloc(m_at(lev, x, y), FALSE);
            else
                return FALSE;
        }
        u_on_newpos(x, y);
        break;
    case LR_PORTAL:
        mkportal(lev, x, y, dest);
        break;
    case LR_DOWNSTAIR:
    case LR_UPSTAIR:
        mkstairs(lev, x, y, (char)rtype, NULL);
        break;
    case LR_BRANCH:
        place_branch(lev, Is_branchlev(lev), x, y);
        break;
    }
    return TRUE;
}

void
makemaz(struct level *lev, const char *s)
{
    int x, y;
    char protofile[20];
    s_level *sp = Is_special(lev);
    coord mm;

    if (*s) {
        if (sp && sp->rndlevs)
            snprintf(protofile, SIZE(protofile), "%s-%d", s, rnd((int)sp->rndlevs));
        else
            strcpy(protofile, s);
    } else if (*(dungeons[lev->z.dnum].proto)) {
        if (dunlevs_in_dungeon(&lev->z) > 1) {
            if (sp && sp->rndlevs)
                snprintf(protofile, SIZE(protofile), "%s%d-%d", dungeons[lev->z.dnum].proto,
                        dunlev(lev), rnd((int)sp->rndlevs));
            else
                snprintf(protofile, SIZE(protofile), "%s%d", dungeons[lev->z.dnum].proto,
                        dunlev(lev));
        } else if (sp && sp->rndlevs) {
            snprintf(protofile, SIZE(protofile), "%s-%d", dungeons[lev->z.dnum].proto,
                    rnd((int)sp->rndlevs));
        } else
            strcpy(protofile, dungeons[lev->z.dnum].proto);

    } else
        strcpy(protofile, "");

    if (*protofile) {
        if (!strcmp(protofile, "orcus")) {
            gen_orcus(lev);
            return;
        } else if (!strcmp(protofile, "valley")) {
            gen_valley(lev);
            return;
        } else if (!strcmp(protofile, "juiblex")) {
            gen_juiblex(lev);
            return;
        } else if (!strcmp(protofile, "water")) {
            gen_waterlevel(lev);
            return;
        } else if (!strcmp(protofile, "fire")) {
            gen_firelevel(lev);
            return;
        }

        strcat(protofile, LEV_EXT);
        if (load_special(lev, protofile)) {
            fixup_special(lev);
            /* some levels can end up with monsters on dead mon list, including
               light source monsters */
            dmonsfree(lev);
            return;     /* no mazification right now */
        }
        impossible("Couldn't load \"%s\" - making a maze.", protofile);
    }

    lev->flags.is_maze_lev = TRUE;

    for (x = 2; x <= x_maze_max; x++)
        for (y = 2; y <= y_maze_max; y++)
            lev->locations[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;

    maze0xy(&mm);
    walkfrom(lev, mm.x, mm.y);
    /* put a boulder at the maze center */
    mksobj_at(BOULDER, lev, (int)mm.x, (int)mm.y, mkobj_normal);

    wallification(lev, 2, 2, x_maze_max, y_maze_max);
    mazexy(lev, &mm);
    mkstairs(lev, mm.x, mm.y, 1, NULL); /* up */
    if (!Invocation_lev(lev)) {
        mazexy(lev, &mm);
        mkstairs(lev, mm.x, mm.y, 0, NULL);     /* down */
    } else {    /* choose "vibrating square" location */
#define x_maze_min 2
#define y_maze_min 2
        /*
         * Pick a position where the stairs down to Moloch's Sanctum
         * level will ultimately be created.  At that time, an area
         * will be altered:  walls removed, moat and traps generated,
         * boulders destroyed.  The position picked here must ensure
         * that that invocation area won't extend off the map.
         *
         * We actually allow up to 2 squares around the usual edge of
         * the area to get truncated; see mkinvokearea(mklev.c).
         */
#define INVPOS_X_MARGIN (6 - 2)
#define INVPOS_Y_MARGIN (5 - 2)
#define INVPOS_DISTANCE 11
        int x_range =
            x_maze_max - x_maze_min - 2 * INVPOS_X_MARGIN - 1, y_range =
            y_maze_max - y_maze_min - 2 * INVPOS_Y_MARGIN - 1;

        do {
            x = rn1(x_range, x_maze_min + INVPOS_X_MARGIN + 1);
            y = rn1(y_range, y_maze_min + INVPOS_Y_MARGIN + 1);
            /* we don't want it to be too near the stairs, nor to be on a spot
               that's already in use (wall|trap) */
        } while (x == lev->upstair.sx || y == lev->upstair.sy ||
                 /* (direct line) */
                 abs(x - lev->upstair.sx) == abs(y - lev->upstair.sy) ||
                 distmin(x, y, lev->upstair.sx,
                         lev->upstair.sy) <= INVPOS_DISTANCE ||
                 !SPACE_POS(lev->locations[x][y].typ) || occupied(lev, x, y));
        maketrap(lev, x, y, VIBRATING_SQUARE);
#undef INVPOS_X_MARGIN
#undef INVPOS_Y_MARGIN
#undef INVPOS_DISTANCE
#undef x_maze_min
#undef y_maze_min
    }

    /* place branch stair or portal */
    place_branch(lev, Is_branchlev(lev), COLNO, ROWNO);

    for (x = rn1(8, 11); x; x--) {
        mazexy(lev, &mm);
        mkobj_at(rn2(2) ? GEM_CLASS : 0, lev, mm.x, mm.y, mkobj_artifact);
    }
    for (x = rn1(10, 2); x; x--) {
        mazexy(lev, &mm);
        mksobj_at(BOULDER, lev, mm.x, mm.y, mkobj_normal);
    }
    for (x = rn2(3); x; x--) {
        mazexy(lev, &mm);
        makemon(&mons[PM_MINOTAUR], lev, mm.x, mm.y, NO_MM_FLAGS);
    }
    for (x = rn1(5, 7); x; x--) {
        mazexy(lev, &mm);
        makemon(NULL, lev, mm.x, mm.y, NO_MM_FLAGS);
    }
    for (x = rn1(6, 7); x; x--) {
        mazexy(lev, &mm);
        mkgold(0L, lev, mm.x, mm.y);
    }
    for (x = rn1(6, 7); x; x--)
        mktrap(lev, 0, 1, NULL, NULL);
}


void
walkfrom(struct level *lev, int x, int y)
{
    int q, a, dir;
    int dirs[4];

    if (!IS_DOOR(lev->locations[x][y].typ)) {
        /* might still be on edge of MAP, so don't overwrite */
        lev->locations[x][y].typ = ROOM;
        lev->locations[x][y].flags = 0;
    }

    while (1) {
        q = 0;
        for (a = 0; a < 4; a++)
            if (okay(lev, x, y, a))
                dirs[q++] = a;
        if (!q)
            return;
        dir = dirs[rn2(q)];
        move(&x, &y, dir);
        lev->locations[x][y].typ = ROOM;
        move(&x, &y, dir);
        walkfrom(lev, x, y);
    }
}


static void
move(int *x, int *y, int dir)
{
    switch (dir) {
    case 0:
        --(*y);
        break;
    case 1:
        (*x)++;
        break;
    case 2:
        (*y)++;
        break;
    case 3:
        --(*x);
        break;
    default:
        panic("move: bad direction");
    }
}


/* find random point in generated corridors,
 * so we don't create items in moats, bunkers, or walls */
void
mazexy(struct level *lev, coord * cc)
{
    int cpt = 0;

    do {
        cc->x = 3 + 2 * rn2((x_maze_max >> 1) - 1);
        cc->y = 3 + 2 * rn2((y_maze_max >> 1) - 1);
        cpt++;
    } while (cpt < 100 && lev->locations[cc->x][cc->y].typ != ROOM);
    if (cpt >= 100) {
        int x, y;

        /* last try */
        for (x = 0; x < (x_maze_max >> 1) - 1; x++)
            for (y = 0; y < (y_maze_max >> 1) - 1; y++) {
                cc->x = 3 + 2 * x;
                cc->y = 3 + 2 * y;
                if (lev->locations[cc->x][cc->y].typ == ROOM)
                    return;
            }
        panic("mazexy: can't find a place!");
    }
    return;
}

/* put a non-diggable boundary around the initial portion of a level map.
 * assumes that no level will initially put things beyond the isok() range.
 *
 * we can't bound unconditionally on the last line with something in it,
 * because that something might be a niche which was already reachable,
 * so the boundary would be breached
 *
 * we can't bound unconditionally on one beyond the last line, because
 * that provides a window of abuse for WALLIFIED_MAZE special levels
 */
void
bound_digging(struct level *lev)
{
    int x, y;
    unsigned typ;
    struct rm *loc;
    boolean found, nonwall;
    int xmin, xmax, ymin, ymax;

    if (lev == sp_lev(sl_earth))
        return; /* everything diggable here */

    found = nonwall = FALSE;
    for (xmin = 0; !found; xmin++) {
        loc = &lev->locations[xmin][0];
        for (y = 0; y <= ROWNO - 1; y++, loc++) {
            typ = loc->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ))
                    nonwall = TRUE;
            }
        }
    }
    xmin -= (nonwall || !lev->flags.is_maze_lev) ? 2 : 1;
    if (xmin < 0)
        xmin = 0;

    found = nonwall = FALSE;
    for (xmax = COLNO - 1; !found; xmax--) {
        loc = &lev->locations[xmax][0];
        for (y = 0; y <= ROWNO - 1; y++, loc++) {
            typ = loc->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ))
                    nonwall = TRUE;
            }
        }
    }
    xmax += (nonwall || !lev->flags.is_maze_lev) ? 2 : 1;
    if (xmax >= COLNO)
        xmax = COLNO - 1;

    found = nonwall = FALSE;
    for (ymin = 0; !found; ymin++) {
        loc = &lev->locations[xmin][ymin];
        for (x = xmin; x <= xmax; x++, loc += ROWNO) {
            typ = loc->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ))
                    nonwall = TRUE;
            }
        }
    }
    ymin -= (nonwall || !lev->flags.is_maze_lev) ? 2 : 1;

    found = nonwall = FALSE;
    for (ymax = ROWNO - 1; !found; ymax--) {
        loc = &lev->locations[xmin][ymax];
        for (x = xmin; x <= xmax; x++, loc += ROWNO) {
            typ = loc->typ;
            if (typ != STONE) {
                found = TRUE;
                if (!IS_WALL(typ))
                    nonwall = TRUE;
            }
        }
    }
    ymax += (nonwall || !lev->flags.is_maze_lev) ? 2 : 1;

    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++)
            if (y <= ymin || y >= ymax || x <= xmin || x >= xmax) {
                lev->locations[x][y].wall_info |= W_NONDIGGABLE;
            }
}

struct trap *
mkportal(struct level *lev, xchar x, xchar y, struct level *dest)
{
    /* a portal "trap" must be matched by a */
    /* portal in the destination dungeon/dlevel */
    struct trap *ttmp = maketrap(lev, x, y, MAGIC_PORTAL);

    if (!ttmp) {
        impossible("portal on top of portal??");
        return NULL;
    }
    ttmp->dest = dest;
    return ttmp;
}

/* when moving in water, possibly (1 in 3) alter the intended destination */
void
water_friction(schar * udx, schar * udy)
{
    int x, y, dx, dy;
    boolean eff = FALSE;

    if (Swimming && rn2(4))
        return; /* natural swimmers have advantage */

    if (*udx && !rn2(!*udy ? 3 : 6)) {  /* 1/3 chance or half that */
        /* cancel delta x and choose an arbitrary delta y value */
        x = u.ux;
        do {
            dy = rn2(3) - 1;    /* -1, 0, 1 */
            y = u.uy + dy;
        } while (dy && (!isok(x, y) || !is_pool(level, x, y)));
        *udx = 0;
        *udy = dy;
        eff = TRUE;
    } else if (*udy && !rn2(!*udx ? 3 : 5)) {   /* 1/3 or 1/5*(5/6) */
        /* cancel delta y and choose an arbitrary delta x value */
        y = u.uy;
        do {
            dx = rn2(3) - 1;    /* -1 .. 1 */
            x = u.ux + dx;
        } while (dx && (!isok(x, y) || !is_pool(level, x, y)));
        *udy = 0;
        *udx = dx;
        eff = TRUE;
    }
    if (eff)
        pline("Water turbulence affects your movements.");
}

static const char *
waterbody_impl(xchar x, xchar y, boolean article)
{
    struct rm *loc;
    schar ltyp;

    if (!isok(x, y)) /* should never happen */
        return msgcat(article ? "a " : "", "drink");
    loc = &level->locations[x][y];
    ltyp = loc->typ;

    if (is_lava(level, x, y))
        return "lava";
    else if (is_ice(level, x, y))
        return "ice";
    else if (is_moat(level, x, y))
        return msgcat(article ? "a " : "", "moat");
    else if ((ltyp != POOL) && (ltyp != WATER) && level == sp_lev(sl_juiblex))
        return msgcat(article ? "a " : "", "swamp");
    else if (ltyp == POOL)
        return msgcat(article ? "a " : "", "pool of water");
    else
        return "water";
}


const char *
a_waterbody(xchar x, xchar y)
{
    return waterbody_impl(x, y, TRUE);
}


const char *
waterbody_name(xchar x, xchar y)
{
    return waterbody_impl(x, y, FALSE);
}

/*mkmaze.c*/
