/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-30 */
/* nh4-scripts: LEVGEN */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "rm.h"
#include "endgame_gen.h"
#include "makemon.h"
#include "levelgen_dsl.h"

#pragma clang diagnostic ignored "-Wunused"

static void setup_waterlevel(struct level *lev);
static void save_waterlevel(struct level *lev, struct memfile *mf);
static void restore_waterlevel(struct level *lev, struct memfile *mf);
static void free_waterlevel(struct level *lev);
static short firelevel_pick_mon(const struct level *, char class,
                                boolean levgen);
static short waterlevel_pick_mon(const struct level *, char class,
                                 boolean levgen);

/*=============================================================================
 * Level managers.
 */

struct level_manager firelevel_manager = {
    .turn_effects = no_level_effects,
    .save_extra = no_level_data,
    .restore_extra = no_level_data,
    .free_extra = no_level_effects,
    .pick_monster = firelevel_pick_mon,
    .intervene = wiz_intervene,
};

struct level_manager waterlevel_manager = {
    .turn_effects = movebubbles,
    .save_extra = save_waterlevel,
    .restore_extra = restore_waterlevel,
    .free_extra = free_waterlevel,
    .pick_monster = waterlevel_pick_mon,
    .intervene = wiz_intervene,
};

/*=============================================================================
 * Level generation functions.
 */

void gen_firelevel(struct level *lev) {
    lev->mgr = &firelevel_manager;
    INIT_LEVGEN(lev);

    NOTELEPORT;
    HARDFLOOR;
    SHORTSIGHTED;

    MAP("..............................................................................."
        "......LLLLLLLL............L.......................LLL.........................."
        ".....LL...................L......................LLLL................LL........"
        ".....L.............LLLL...LL....LL...............LLLLL.............LLL........."
        "...LLLL..............LL....L.....LLL..............LLLL..............LLLL......."
        "LL..........LLLL...LLLL...LLL....LLL......L........LLLL....LL........LLL......."
        "L.........LLLLLLL...LL.....L......L......LL.........LL......LL........LL...L..."
        "L.........LL..LLL..LL......LL......LLLL..L.........LL......LLL............LL..L"
        "......L..LL....LLLLL.................LLLLLLL.......L......LL............LLLLLLL"
        "......L..L.....LL.LLLL.......L............L........LLLLL.LL......LL.........LL."
        "......LL........L...LL......LL.............LLL.....L...LLL.......LLL.........LL"
        ".......LLLLLL........L.......LLL.............L....LL...L.LLL......LLLLLLL......"
        "............LLLL............LL.L.............L....L...LL.........LLL..LLL......"
        ".............................LLLLL...........LL...L...L........LLLL..LLLLLL...."
        ".......LLLL.............LL....LL.......LLL...LL.......L..LLL....LLLLLLL........"
        "L........LLL.........LLLLLLLLLLL......LLLLL...L...........LL...LL...LL........."
        "L..........LL.......LL.........LL.......LLL....L..LLL....LL.........LL........."
        "L...........LLLLLLLLL...........LL....LLL.......LLLLL.....LL........LL........."
        "LL.................L.............LLLLLL............LL...LLLL.........LL........"
        "LL.................................LL....................LL...................."
        "..L....LL.L.......................LL......................LLL..................");

    TELEPORT_REGION(UP, ZR(71, 16, 71, 16));
    PORTAL("water", zn_subtract(EVERYWHERE, ZR(67, 13, 78, 20)));

    REPEAT(40)
        TRAP(FIRE_TRAP, EVERYWHERE);

    REPEAT(19)
        MON(PM_FIRE_ELEMENTAL, EVERYWHERE, HOSTILE);
    REPEAT(8)
        MON(PM_SALAMANDER, EVERYWHERE);
    REPEAT(6) {
        MON(PM_FIRE_VORTEX, EVERYWHERE);
        MON(PM_HELL_HOUND, EVERYWHERE);
    }
    REPEAT(5)
        MON(PM_FIRE_GIANT, EVERYWHERE);
    REPEAT(3) {
        MON(PM_PIT_FIEND, EVERYWHERE);
        MON(PM_PIT_VIPER, EVERYWHERE);
        MON(PM_BARBED_DEVIL, EVERYWHERE);
    }
    REPEAT(2) {
        MON(PM_STONE_GOLEM, EVERYWHERE);
        MON(PM_STEAM_VORTEX, EVERYWHERE);
    }
    MON(PM_RED_DRAGON, EVERYWHERE);
    MON(PM_BALROG, EVERYWHERE);
    MON(PM_SCORPION, EVERYWHERE);
    MON(PM_DUST_VORTEX, EVERYWHERE);
    MON(PM_MINOTAUR, EVERYWHERE);

    REPEAT(5)
        OBJ(BOULDER, EVERYWHERE);

    FINISH_LEV;
}

void gen_waterlevel(struct level *lev) {
    lev->mgr = &waterlevel_manager;
    INIT_LEVGEN(lev);
    FILL_MAP('W');

    NOTELEPORT;
    HARDFLOOR;
    SHORTSIGHTED;
    FORGETFUL;

    /* create the bubbles */
    setup_waterlevel(lev);

    TELEPORT_REGION(UP, ZR(0,0,25,20));
    PORTAL("astral", ZR(51, 0, 78, 20));

    REPEAT(19)
        MON(PM_WATER_ELEMENTAL, EVERYWHERE, HOSTILE);
    REPEAT(9)
        MON(PM_KRAKEN, EVERYWHERE);
    REPEAT(8) {
        MON(PM_GIANT_EEL, EVERYWHERE);
        MON(PM_ELECTRIC_EEL, EVERYWHERE);
    }
    REPEAT(4) {
        MON(PM_SHARK, EVERYWHERE);
        MON(PM_PIRANHA, EVERYWHERE);
        MON(PM_JELLYFISH, EVERYWHERE);
        MON(RANDOM_MON_OF(';'), EVERYWHERE);
    }

    FINISH_LEV;
}

/*=============================================================================
 * Monster generation functions.
 */

static uchar
waterlevel_gen_prob(const struct level *lev, short mndx, void *flags) {
    uchar prob = default_gen_prob(lev, mndx, flags);
    const struct permonst *mdat = &mons[mndx];

    /* Respect a default probability of 0 */
    if (!prob)
        return 0;

    /* Only swimming monsters should be present. */
    if (!is_swimmer(mdat))
        return 0;

    /* Gremlins lead to degenerate growth. */
    if (mndx == PM_GREMLIN)
        return 0;

    return prob;
}

short
waterlevel_pick_mon(const struct level *lev, char class, boolean levgen) {
    struct default_gen_flags flags = {
        .consider_xlvl = !levgen,
        .ignore_extinct = FALSE,
        .ignore_nogen = FALSE,
        .ignore_uniq = FALSE,
    };

    return pick_monster_class(lev, class, waterlevel_gen_prob, &flags);
}

static uchar
firelevel_gen_prob(const struct level *lev, short mndx, void *flags) {
    uchar prob = default_gen_prob(lev, mndx, flags);
    const struct permonst *mdat = &mons[mndx];

    /* Respect a default probability of 0 */
    if (!prob)
        return 0;

    /* Only fire resistant monsters should be present. */
    if (!pm_resistance(mdat, MR_FIRE))
        return 0;

    return prob;
}

short
firelevel_pick_mon(const struct level *lev, char class, boolean levgen) {
    struct default_gen_flags flags = {
        .consider_xlvl = !levgen,
        .ignore_extinct = FALSE,
        .ignore_nogen = FALSE,
        .ignore_uniq = FALSE,
    };

    return pick_monster_class(lev, class, firelevel_gen_prob, &flags);
}


/*=============================================================================
 * Special waterlevel stuff in endgame (TH).
 *
 * Some of these functions would probably logically belong to some
 * other source files, but they are all so nicely encapsulated here.
 */

enum bub_content {
    bc_obj,
    bc_mon,
    bc_hero,
    bc_trap,
};

struct container {
    struct container *next;
    xchar x, y;
    enum bub_content what;
    void *list;
};

struct bubble {
    xchar x, y; /* coordinates of the upper left corner */
    schar dx, dy;       /* the general direction of the bubble's movement */
    const uchar *bm;    /* pointer to the bubble bit mask */
    struct bubble *prev, *next; /* need to traverse the list up and down */
    struct container *cons;
};

struct bub_info {
    /* We alternate the order in which we traverse the bubbles. This keeps track
     * of which way to go. */
    boolean fwd;
    /* Linked list of bubbles */
    struct bubble *first, *last;
};

static const int xmin = 0;
static const int ymin = 0;
static const int xmax = COLNO - 1;
static const int ymax = ROWNO - 1;

/* bubble movement boundaries */
#define bxmin (xmin + 1)
#define bymin (ymin + 1)
#define bxmax (xmax - 1)
#define bymax (ymax - 1)

static void mk_bubble(struct level *lev, int x, int y, int n);
static void mv_bubble(struct level *lev, struct bubble *b, int dx, int dy,
                      boolean ini);

/*
 * These bit masks make visually pleasing bubbles on a normal aspect
 * 25x80 terminal, which naturally results in them being mathematically
 * anything but symmetric.  For this reason they cannot be computed
 * in situ, either.  The first two elements tell the dimensions of
 * the bubble's bounding box.
 */
static const uchar bm2[] = { 2, 1, 0x3 }, bm3[] = {
3, 2, 0x7, 0x7}, bm4[] = {
4, 3, 0x6, 0xf, 0x6}, bm5[] = {
5, 3, 0xe, 0x1f, 0xe}, bm6[] = {
6, 4, 0x1e, 0x3f, 0x3f, 0x1e}, bm7[] = {
7, 4, 0x3e, 0x7f, 0x7f, 0x3e}, bm8[] = {
8, 4, 0x7e, 0xff, 0xff, 0x7e}, *const bmask[] =
    { bm2, bm3, bm4, bm5, bm6, bm7, bm8 };

static void
save_waterlevel(struct level *lev, struct memfile *mf)
{
    struct bubble *b;
    int i, n;

    if (lev != sp_lev(sl_water)) {
        impossible("calling save_waterlevel not on the water level");
        return;
    }

    struct bub_info *bi = (struct bub_info *)lev->extra;

    n = 0;
    for (b = bi->first; b; b = b->next)
        ++n;
    /* We're assuming that the number of bubbles stays constant, as we can't
       reasonably tag it anyway. If they don't, we just end up with a diff
       that's longer than it needs to be. */
    mtag(mf, 0, MTAG_WATERLEVEL);
    mwrite8(mf, bi->fwd);
    mwrite32(mf, n);
    for (b = bi->first; b; b = b->next) {
        mwrite8(mf, b->x);
        mwrite8(mf, b->y);
        mwrite8(mf, b->dx);
        mwrite8(mf, b->dy);
        for (i = 0; i < SIZE(bmask); i++)
            if (b->bm == bmask[i])
                mwrite8(mf, i);
    }
}

static void
restore_waterlevel(struct level *lev, struct memfile *mf)
{
    struct bubble *b = NULL, *btmp;
    int i, idx;
    int n;

    if (lev != sp_lev(sl_water)) {
        impossible("calling restore_waterlevel not on the water level");
        return;
    }

    struct bub_info *bi = malloc(sizeof(struct bub_info));

    bi->fwd = mread8(mf);
    bi->first = NULL;
    n = mread32(mf);

    for (i = 0; i < n; i++) {
        btmp = b;
        b = malloc(sizeof (struct bubble));
        b->x = mread8(mf);
        b->y = mread8(mf);
        b->dx = mread8(mf);
        b->dy = mread8(mf);
        idx = mread8(mf);
        b->bm = bmask[idx];
        b->next = NULL;
        b->cons = NULL;

        if (bi->first) {
            btmp->next = b;
            b->prev = btmp;
        } else {
            bi->first = b;
            b->prev = NULL;
        }
    }
    bi->last = b;

    lev->extra = bi;
}

/* FIXME: This should not need to be exposed. */
void
movebubbles(struct level *lev)
{
    struct bubble *b;
    int x, y, i, j;
    struct trap *btrap;

    if (lev != sp_lev(sl_water)) {
        impossible("calling movebubbles not on the water level");
        return;
    }

    struct bub_info *bi = lev->extra;

    static const struct rm water_pos = {
        S_water, 0, 0, 0, 0, 0, WATER /* typ */ ,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    vision_recalc(2);
    /* keep attached ball&chain separate from bubble objects */
    if (Punished)
        unplacebc();

    /*
     * Pick up everything inside of a bubble then fill all bubble
     * locations.
     */
    for (b = bi->fwd ? bi->first : bi->last; b;
         b = bi->fwd ? b->next : b->prev) {
        if (b->cons)
            panic("movebubbles: cons != null");
        for (i = 0, x = b->x; i < (int)b->bm[0]; i++, x++)
            for (j = 0, y = b->y; j < (int)b->bm[1]; j++, y++)
                if (b->bm[j + 2] & (1 << i)) {
                    if (!isok(x, y)) {
                        impossible("movebubbles: bad pos (%d,%d)", x, y);
                        continue;
                    }

                    /* pick up objects, monsters, hero, and traps */
                    if (OBJ_AT(x, y)) {
                        struct obj *olist = NULL, *otmp;
                        struct container *cons =
                            malloc(sizeof (struct container));

                        while ((otmp = level->objects[x][y]) != 0) {
                            remove_object(otmp);
                            otmp->ox = otmp->oy = 0;
                            otmp->nexthere = olist;
                            olist = otmp;
                        }

                        cons->x = x;
                        cons->y = y;
                        cons->what = bc_obj;
                        cons->list = olist;
                        cons->next = b->cons;
                        b->cons = cons;
                    }
                    if (MON_AT(level, x, y)) {
                        struct monst *mon = m_at(level, x, y);
                        struct container *cons =
                            malloc(sizeof (struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = bc_mon;
                        cons->list = mon;

                        cons->next = b->cons;
                        b->cons = cons;

                        if (mon->wormno)
                            remove_worm(mon);
                        else
                            remove_monster(level, x, y);

                        newsym(x, y);   /* clean up old position */
                        mon->mx = COLNO;
                        mon->my = ROWNO;
                    }
                    if (!Engulfed && x == u.ux && y == u.uy) {
                        struct container *cons =
                            malloc(sizeof (struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = bc_hero;
                        cons->list = NULL;

                        cons->next = b->cons;
                        b->cons = cons;
                    }
                    if ((btrap = t_at(level, x, y)) != 0) {
                        struct container *cons =
                            malloc(sizeof (struct container));

                        cons->x = x;
                        cons->y = y;
                        cons->what = bc_trap;
                        cons->list = btrap;

                        cons->next = b->cons;
                        b->cons = cons;
                    }

                    level->locations[x][y] = water_pos;
                    block_point(x, y);
                }
    }

    /*
     * Every second time traverse down.  This is because otherwise
     * all the junk that changes owners when bubbles overlap
     * would eventually end up in the last bubble in the chain.
     */
    bi->fwd = !bi->fwd;
    for (b = bi->fwd ? bi->first : bi->last; b;
         b = bi->fwd ? b->next : b->prev) {
        int rx = rn2(3), ry = rn2(3);

        mv_bubble(level, b, b->dx + 1 - (!b->dx ? rx : (rx ? 1 : 0)),
                  b->dy + 1 - (!b->dy ? ry : (ry ? 1 : 0)), FALSE);
    }

    /* put attached ball&chain back */
    if (Punished)
        placebc();
    turnstate.vision_full_recalc = TRUE;
}

/* FIXME: This shouldn't need to be exposed. */
static void
setup_waterlevel(struct level *lev)
{
    int x, y;
    int xskip, yskip;

    /* set hero's memory to water */
    for (x = xmin; x <= xmax; x++)
        for (y = ymin; y <= ymax; y++)
            lev->locations[x][y].mem_bg = S_water;

    /* make bubbles */
    lev->extra = calloc(sizeof(struct bub_info), 1);
    xskip = 10 + rn2(10);
    yskip = 4 + rn2(4);
    for (x = bxmin; x <= bxmax; x += xskip)
        for (y = bymin; y <= bymax; y += yskip)
            mk_bubble(lev, x, y, rn2(7));
}

static void
free_waterlevel(struct level* lev)
{
    struct bubble *b, *bb;

    if (lev != sp_lev(sl_water)) {
        impossible("calling free_waterlevel not on the water level");
        return;
    }

    struct bub_info *bi = lev->extra;

    /* free bubbles */
    for (b = bi->first; b; b = bb) {
        bb = b->next;
        free(b);
    }

    free(bi);
}


static void
mk_bubble(struct level *lev, int x, int y, int n)
{
    struct bubble *b;

    if (lev != sp_lev(sl_water)) {
        impossible("calling mk_bubble not on the water level");
        return;
    }

    struct bub_info *bi = lev->extra;

    if (x >= bxmax || y >= bymax)
        return;
    if (n >= SIZE(bmask)) {
        impossible("n too large (mk_bubble)");
        n = SIZE(bmask) - 1;
    }
    b = malloc(sizeof (struct bubble));
    if ((x + (int)bmask[n][0] - 1) > bxmax)
        x = bxmax - bmask[n][0] + 1;
    if ((y + (int)bmask[n][1] - 1) > bymax)
        y = bymax - bmask[n][1] + 1;
    b->x = x;
    b->y = y;
    b->dx = 1 - rn2(3);
    b->dy = 1 - rn2(3);
    b->bm = bmask[n];
    b->cons = 0;
    if (!bi->first)
        bi->first = b;
    if (bi->last) {
        bi->last->next = b;
        b->prev = bi->last;
    } else
        b->prev = NULL;
    b->next = NULL;
    bi->last = b;
    mv_bubble(lev, b, 0, 0, TRUE);
}

/*
 * The player, the portal and all other objects and monsters
 * float along with their associated bubbles.  Bubbles may overlap
 * freely, and the contents may get associated with other bubbles in
 * the process.  Bubbles are "sticky", meaning that if the player is
 * in the immediate neighborhood of one, he/she may get sucked inside.
 * This property also makes leaving a bubble slightly difficult.
 */
static void
mv_bubble(struct level *lev, struct bubble *b, int dx, int dy, boolean ini)
{
    int x, y, i, j, colli = 0;
    struct container *cons, *ctemp;

    /* move bubble */
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1) {
        /* pline("mv_bubble: dx = %d, dy = %d", dx, dy); */
        dx = sgn(dx);
        dy = sgn(dy);
    }

    /*
     * collision with level borders?
     *      1 = horizontal border, 2 = vertical, 3 = corner
     */
    if (b->x <= bxmin)
        colli |= 2;
    if (b->y <= bymin)
        colli |= 1;
    if ((int)(b->x + b->bm[0] - 1) >= bxmax)
        colli |= 2;
    if ((int)(b->y + b->bm[1] - 1) >= bymax)
        colli |= 1;

    if (b->x < bxmin) {
        pline("bubble xmin: x = %d, xmin = %d", b->x, bxmin);
        b->x = bxmin;
    }
    if (b->y < bymin) {
        pline("bubble ymin: y = %d, ymin = %d", b->y, bymin);
        b->y = bymin;
    }
    if ((int)(b->x + b->bm[0] - 1) > bxmax) {
        pline("bubble xmax: x = %d, xmax = %d", b->x + b->bm[0] - 1, bxmax);
        b->x = bxmax - b->bm[0] + 1;
    }
    if ((int)(b->y + b->bm[1] - 1) > bymax) {
        pline("bubble ymax: y = %d, ymax = %d", b->y + b->bm[1] - 1, bymax);
        b->y = bymax - b->bm[1] + 1;
    }

    /* bounce if we're trying to move off the border */
    if (b->x == bxmin && dx < 0)
        dx = -dx;
    if (b->x + b->bm[0] - 1 == bxmax && dx > 0)
        dx = -dx;
    if (b->y == bymin && dy < 0)
        dy = -dy;
    if (b->y + b->bm[1] - 1 == bymax && dy > 0)
        dy = -dy;

    b->x += dx;
    b->y += dy;

    /* void positions inside bubble */

    for (i = 0, x = b->x; i < (int)b->bm[0]; i++, x++)
        for (j = 0, y = b->y; j < (int)b->bm[1]; j++, y++)
            if (b->bm[j + 2] & (1 << i)) {
                lev->locations[x][y].typ = AIR;
                lev->locations[x][y].lit = 1;
                unblock_point(x, y);
            }

    /* replace contents of bubble */
    for (cons = b->cons; cons; cons = ctemp) {
        ctemp = cons->next;
        cons->x += dx;
        cons->y += dy;

        switch (cons->what) {
        case bc_obj:{
                struct obj *olist, *otmp;

                for (olist = (struct obj *)cons->list; olist; olist = otmp) {
                    otmp = olist->nexthere;
                    place_object(olist, lev, cons->x, cons->y);
                }
                break;
            }

        case bc_mon:{
                struct monst *mon = (struct monst *)cons->list;

                mnearto(mon, cons->x, cons->y, TRUE);
                break;
            }

        case bc_hero:{
                int ux0 = u.ux, uy0 = u.uy;

                /* change u.ux0 and u.uy0? */
                u.ux = cons->x;
                u.uy = cons->y;
                newsym(ux0, uy0);       /* clean up old position */

                if (MON_AT(lev, cons->x, cons->y)) {
                    mnexto(m_at(lev, cons->x, cons->y));
                }
                break;
            }

        case bc_trap:{
                struct trap *btrap = (struct trap *)cons->list;

                btrap->tx = cons->x;
                btrap->ty = cons->y;
                break;
            }

        default:
            impossible("mv_bubble: unknown bubble contents");
            break;
        }
        free(cons);
    }
    b->cons = 0;

    /* boing? */

    switch (colli) {
    case 1:
        b->dy = -b->dy;
        break;
    case 3:
        b->dy = -b->dy; /* fall through */
    case 2:
        b->dx = -b->dx;
        break;
    default:
        /* sometimes alter direction for fun anyway (higher probability for
           stationary bubbles) */
        if (!ini && ((b->dx || b->dy) ? !rn2(20) : !rn2(5))) {
            b->dx = 1 - rn2(3);
            b->dy = 1 - rn2(3);
        }
    }
}

/* endgame.c */
