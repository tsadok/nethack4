/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-12-02 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dgn_file.h"
#include "dlb.h"
#include "lev.h"


#define DUNGEON_FILE    "dungeon"

#define X_START         "x-strt"
#define X_LOCATE        "x-loca"
#define X_GOAL          "x-goal"

struct proto_dungeon {
    struct tmpdungeon tmpdungeon[MAXDUNGEON];
    struct tmplevel tmplevel[LEV_LIMIT];
    s_level *final_lev[LEV_LIMIT];      /* corresponding level pointers */
    struct tmpbranch tmpbranch[BRANCH_LIMIT];

    int start;  /* starting index of current dungeon sp levels */
    int n_levs; /* number of tmplevel entries */
    int n_brs;  /* number of tmpbranch entries */
};

int n_dgns;     /* number of dungeons (used here, and mklev.c) */

static branch *branches = NULL; /* dungeon branch list */

struct lchoice {
    int idx;
    schar lev[MAXLINFO];
    schar playerlev[MAXLINFO];
    xchar dgn[MAXLINFO];
    char menuletter;
};

static void Fread(void *, int, int, dlb *);
static xchar dname_to_dnum(const char *);
static int find_branch(const char *, struct proto_dungeon *);
static xchar parent_dnum(const char *, struct proto_dungeon *);
static int level_range(xchar, int, int, int, struct proto_dungeon *, int *);
static xchar parent_dlevel(const char *, struct proto_dungeon *);
static int correct_branch_type(struct tmpbranch *);
static branch *add_branch(int, int, struct proto_dungeon *);
static void add_level(s_level *);
static void init_level(int, int, struct proto_dungeon *);
static int possible_places(int, boolean *, struct proto_dungeon *);
static xchar pick_level(boolean *, int);
static boolean place_level(int, struct proto_dungeon *);
static const char *br_string(int);
static void print_branch(struct nh_menulist *menu, int dnum, int lower_bound,
                         int upper_bound, boolean bymenu,
                         struct lchoice *lchoices);


static void
freelevchn(void)
{
    s_level *tmplev, *tmplev2;

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev2) {
        tmplev2 = tmplev->next;
        free(tmplev);
    }
    sp_levchn = NULL;
}


void
free_dungeon(void)
{
    branch *curr, *next;

    for (curr = branches; curr; curr = next) {
        next = curr->next;
        free(curr);
    }
    branches = NULL;

    freelevchn();
}


void
save_d_flags(struct memfile *mf, d_flags f)
{
    unsigned int dflags;

    /* bitfield layouts are architecture and compiler-dependent, so d_flags
       can't be written directly */
    dflags = (f.town << 31) | (f.hellish << 30) | (f.maze_like << 29) |
        (f.rogue_like << 28) | (f.  align << 25);
    mwrite32(mf, dflags);
}

static void
save_dungeon_struct(struct memfile *mf, const dungeon * dgn)
{
    mtag(mf, dgn->ledger_start, MTAG_DUNGEONSTRUCT);
    mwrite(mf, dgn->dname, sizeof (dgn->dname));
    mwrite(mf, dgn->proto, sizeof (dgn->proto));
    mwrite8(mf, dgn->boneid);
    save_d_flags(mf, dgn->flags);
    mwrite8(mf, dgn->entry_lev);
    mwrite8(mf, dgn->num_dunlevs);
    mwrite8(mf, dgn->dunlev_ureached);
    mwrite32(mf, dgn->ledger_start);
    mwrite32(mf, dgn->depth_start);
}

void
save_dlevel(struct memfile *mf, d_level d)
{
    mwrite8(mf, d.dnum);
    mwrite8(mf, d.dlevel);
}

static void
save_branch(struct memfile *mf, const branch * b)
{
    mtag(mf, b->id, MTAG_BRANCH);
    mwrite32(mf, b->id);
    mwrite32(mf, b->type);
    save_dlevel(mf, b->end1);
    save_dlevel(mf, b->end2);
    mwrite8(mf, b->end1_up);
}

static void
save_dungeon_topology(struct memfile *mf)
{
    mtag(mf, 0, MTAG_DUNGEON_TOPOLOGY);
    for (int i = 0; i < num_special_levels; ++i) {
        struct level *lev = dungeon_topology.special_levels[i];
        mwrite8(mf, lev ? ledger_no(&lev->z) : -1);
    }
    mwrite8(mf, dungeon_topology.d_tower_dnum);
    mwrite8(mf, dungeon_topology.d_sokoban_dnum);
    mwrite8(mf, dungeon_topology.d_mines_dnum);
    mwrite8(mf, dungeon_topology.d_quest_dnum);
    mwrite8(mf, dungeon_topology.d_endgame_dnum);
}

static void
savelevchn(struct memfile *mf)
{
    s_level *tmplev;
    int cnt = 0;

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev->next)
        cnt++;
    mwrite32(mf, cnt);

    for (tmplev = sp_levchn; tmplev; tmplev = tmplev->next) {
        save_d_flags(mf, tmplev->flags);
        save_dlevel(mf, tmplev->dlevel);
        mwrite(mf, tmplev->proto, sizeof (tmplev->proto));
        mwrite8(mf, tmplev->boneid);
        mwrite8(mf, tmplev->rndlevs);
    }
}

/* Save the dungeon structures. */
void
save_dungeon(struct memfile *mf)
{
    branch *curr;
    int count, i;

    mfmagic_set(mf, DGN_MAGIC);
    mtag(mf, 0, MTAG_DUNGEON);

    mwrite32(mf, n_dgns);
    for (i = 0; i < n_dgns; i++)
        save_dungeon_struct(mf, &dungeons[i]);

    /* store dungeon layout */
    savelevchn(mf);

    /* store levels */
    xchar ltmp;
    for (ltmp = 0; ltmp <= maxledgerno(); ltmp++) {
        if (!levels[ltmp])
            panic("No level when attempting to save!");
        mtag(mf, ltmp, MTAG_LEVELS);
        savelev(mf, ltmp);      /* actual level */
    }

    /* writing dungeon_topology directly should be ok, it's just a * fancy
       collection of single byte values */
    save_dungeon_topology(mf);
    mwrite(mf, tune, sizeof tune);

    for (count = 0, curr = branches; curr; curr = curr->next)
        count++;
    mwrite32(mf, count);

    for (curr = branches; curr; curr = curr->next)
        save_branch(mf, curr);
}

d_flags
restore_d_flags(struct memfile *mf)
{
    unsigned int dflags;
    d_flags f;

    dflags = mread32(mf);
    f.town = (dflags >> 31) & 1;
    f.hellish = (dflags >> 30) & 1;
    f.maze_like = (dflags >> 29) & 1;
    f.rogue_like = (dflags >> 28) & 1;
    f.align = (dflags >> 25) & 7;

    return f;
}

static void
restore_dungeon_struct(struct memfile *mf, dungeon * dgn)
{
    mread(mf, dgn->dname, sizeof (dgn->dname));
    mread(mf, dgn->proto, sizeof (dgn->proto));
    dgn->boneid = mread8(mf);
    dgn->flags = restore_d_flags(mf);
    dgn->entry_lev = mread8(mf);
    dgn->num_dunlevs = mread8(mf);
    dgn->dunlev_ureached = mread8(mf);
    dgn->ledger_start = mread32(mf);
    dgn->depth_start = mread32(mf);
}

void
restore_dlevel(struct memfile *mf, d_level *d)
{
    d->dnum = mread8(mf);
    d->dlevel = mread8(mf);
}

static void
restore_branch(struct memfile *mf, branch * b)
{
    b->next = NULL;
    b->id = mread32(mf);
    b->type = mread32(mf);
    restore_dlevel(mf, &b->end1);
    restore_dlevel(mf, &b->end2);
    b->end1_up = mread8(mf);
}

static void
restore_dungeon_topology(struct memfile *mf)
{
    int i;
    for (i = 0; i < num_special_levels; ++i) {
        int8_t ledger = mread8(mf);
        dungeon_topology.special_levels[i] =
            ledger == -1 ? NULL : levels[ledger];
    }
    dungeon_topology.d_tower_dnum = mread8(mf);
    dungeon_topology.d_sokoban_dnum = mread8(mf);
    dungeon_topology.d_mines_dnum = mread8(mf);
    dungeon_topology.d_quest_dnum = mread8(mf);
    dungeon_topology.d_endgame_dnum = mread8(mf);
}

static void
restlevchn(struct memfile *mf)
{
    int cnt;
    s_level *tmplev, *x;

    sp_levchn = NULL;
    cnt = mread32(mf);
    for (; cnt > 0; cnt--) {
        tmplev = malloc(sizeof (s_level));
        tmplev->flags = restore_d_flags(mf);
        restore_dlevel(mf, &tmplev->dlevel);
        mread(mf, tmplev->proto, sizeof (tmplev->proto));
        tmplev->boneid = mread8(mf);
        tmplev->rndlevs = mread8(mf);

        if (!sp_levchn)
            sp_levchn = tmplev;
        else {
            for (x = sp_levchn; x->next; x = x->next) ;
            x->next = tmplev;
        }
        tmplev->next = NULL;
    }
}

/* Restore the dungeon structures. */
void
restore_dungeon(struct memfile *mf)
{
    branch *curr, *last;
    int count, i, j;

    mfmagic_check(mf, DGN_MAGIC);
    n_dgns = mread32(mf);
    for (i = 0; i < n_dgns; i++) {
        restore_dungeon_struct(mf, &dungeons[i]);
        for (j = 0; j < dungeons[i].num_dunlevs; ++j) {
            levels[dungeons[i].ledger_start + j] =
                alloc_level(&(struct d_level){.dnum = i, .dlevel = j + 1});
        }
    }

    /* restore dungeon */
    restlevchn(mf);

    /* restore levels */
    xchar ltmp;
    for (ltmp = 0; ltmp <= maxledgerno(); ltmp++) {
        getlev(mf, levels[ltmp], FALSE);
        if (!ltmp == ledger_no(&levels[ltmp]->z))
            panic("dungeon structure corrupt when restoring");
    }

    restore_dungeon_topology(mf);
    mread(mf, tune, sizeof tune);

    last = branches = NULL;

    count = mread32(mf);
    for (i = 0; i < count; i++) {
        curr = malloc(sizeof (branch));
        restore_branch(mf, curr);
        if (last)
            last->next = curr;
        else
            branches = curr;
        last = curr;
    }
}

static void
Fread(void *ptr, int size, int nitems, dlb * stream)
{
    int cnt;

    if ((cnt = dlb_fread(ptr, size, nitems, stream)) != nitems) {
        panic
            ("Premature EOF on dungeon description file!\r\nExpected %d bytes "
             "- got %d.",
             (size * nitems), (size * cnt));
    }
}

static xchar
dname_to_dnum(const char *s)
{
    xchar i;

    for (i = 0; i < n_dgns; i++)
        if (!strcmp(dungeons[i].dname, s))
            return i;

    panic("Couldn't resolve dungeon number for name \"%s\".", s);
    /* NOT REACHED */
    return (xchar) 0;
}

struct level *
sp_lev(enum special_level sl) {
    return dungeon_topology.special_levels[sl];
}

s_level *
find_level(const char *s)
{
    s_level *curr;

    for (curr = sp_levchn; curr; curr = curr->next)
        if (!strcmpi(s, curr->proto))
            break;
    return curr;
}

/* Find the branch that links the named dungeon. */
static int
find_branch(const char *s,      /* dungeon name */
            struct proto_dungeon *pd)
{
    int i;

    if (pd) {
        for (i = 0; i < pd->n_brs; i++)
            if (!strcmp(pd->tmpbranch[i].name, s))
                break;
        if (i == pd->n_brs)
            panic("find_branch: can't find %s", s);
    } else {
        /* support for level tport by name */
        branch *br;
        const char *dnam;

        for (br = branches; br; br = br->next) {
            dnam = dungeons[br->end2.dnum].dname;
            if (!strcmpi(dnam, s) ||
                (!strncmpi(dnam, "The ", 4) && !strcmpi(dnam + 4, s)))
                break;
        }
        i = br ? ((ledger_no(&br->end1) << 8) | ledger_no(&br->end2)) : -1;
    }
    return i;
}


/*
 * Find the "parent" by searching the prototype branch list for the branch
 * listing, then figuring out to which dungeon it belongs.
 */
static xchar
parent_dnum(const char *s,      /* dungeon name */
            struct proto_dungeon *pd)
{
    int i;
    xchar pdnum;

    i = find_branch(s, pd);
    /* 
     * Got branch, now find parent dungeon.  Stop if we have reached
     * "this" dungeon (if we haven't found it by now it is an error).
     */
    for (pdnum = 0; strcmp(pd->tmpdungeon[pdnum].name, s); pdnum++)
        if ((i -= pd->tmpdungeon[pdnum].branches) < 0)
            return pdnum;

    panic("parent_dnum: couldn't resolve branch.");
    /* NOT REACHED */
    return (xchar) 0;
}

/*
 * Return a starting point and number of successive positions a level
 * or dungeon entrance can occupy.
 *
 * Note: This follows the acouple (instead of the rcouple) rules for a
 *       negative random component (rand < 0).  These rules are found
 *       in dgn_comp.y.  The acouple [absolute couple] section says that
 *       a negative random component means from the (adjusted) base to the
 *       end of the dungeon.
 */
static int
level_range(xchar dgn, int base, int rrand, int chain, struct proto_dungeon *pd,
            int *adjusted_base)
{
    int lmax = dungeons[dgn].num_dunlevs;

    if (chain >= 0) {   /* relative to a special level */
        s_level *levtmp = pd->final_lev[chain];

        if (!levtmp)
            panic("level_range: empty chain level!");

        base += levtmp->dlevel.dlevel;
    } else {    /* absolute in the dungeon */
        /* from end of dungeon */
        if (base < 0)
            base = (lmax + base + 1);
    }

    if (base < 1 || base > lmax)
        panic("level_range: base value out of range");

    *adjusted_base = base;

    if (rrand == -1) {  /* from base to end of dungeon */
        return lmax - base + 1;
    } else if (rrand) {
        /* make sure we don't run off the end of the dungeon */
        return ((base + rrand - 1) > lmax) ? lmax - base + 1 : rrand;
    }   /* else only one choice */
    return 1;
}

static xchar
parent_dlevel(const char *s, struct proto_dungeon *pd)
{
    int i, j, num, base, dnum = parent_dnum(s, pd);
    branch *curr;


    i = find_branch(s, pd);
    num =
        level_range(dnum, pd->tmpbranch[i].lev.base, pd->tmpbranch[i].lev.rand,
                    pd->tmpbranch[i].chain, pd, &base);

    /* KMH -- Try our best to find a level without an existing branch */
    i = j = rn2(num);
    do {
        if (++i >= num)
            i = 0;
        for (curr = branches; curr; curr = curr->next)
            if ((curr->end1.dnum == dnum && curr->end1.dlevel == base + i) ||
                (curr->end2.dnum == dnum && curr->end2.dlevel == base + i))
                break;
    } while (curr && i != j);
    return base + i;
}

/* Convert from the temporary branch type to the dungeon branch type. */
static int
correct_branch_type(struct tmpbranch *tbr)
{
    switch (tbr->type) {
    case TBR_STAIR:
        return BR_STAIR;
    case TBR_NO_UP:
        return tbr->up ? BR_NO_END1 : BR_NO_END2;
    case TBR_NO_DOWN:
        return tbr->up ? BR_NO_END2 : BR_NO_END1;
    case TBR_PORTAL:
        return BR_PORTAL;
    }
    impossible("correct_branch_type: unknown branch type");
    return BR_STAIR;
}

/*
 * Add the given branch to the branch list.  The branch list is ordered
 * by end1 dungeon and level followed by end2 dungeon and level.  If
 * extract_first is true, then the branch is already part of the list
 * but needs to be repositioned.
 */
void
insert_branch(branch * new_branch, boolean extract_first)
{
    branch *curr, *prev;
    long new_val, curr_val, prev_val;

    if (extract_first) {
        for (prev = 0, curr = branches; curr; prev = curr, curr = curr->next)
            if (curr == new_branch)
                break;

        if (!curr)
            panic("insert_branch: not found");
        if (prev)
            prev->next = curr->next;
        else
            branches = curr->next;
    }
    new_branch->next = NULL;

/* Convert the branch into a unique number so we can sort them. */
#define branch_val(bp)                                                  \
    ((((long)(bp)->end1.dnum * (MAXLEVEL+1) +                           \
       (long)(bp)->end1.dlevel) * (MAXDUNGEON+1) * (MAXLEVEL+1)) +      \
     ((long)(bp)->end2.dnum * (MAXLEVEL+1) + (long)(bp)->end2.dlevel))

    /* 
     * Insert the new branch into the correct place in the branch list.
     */
    prev = NULL;
    prev_val = -1;
    new_val = branch_val(new_branch);
    for (curr = branches; curr;
         prev_val = curr_val, prev = curr, curr = curr->next) {
        curr_val = branch_val(curr);
        if (prev_val < new_val && new_val <= curr_val)
            break;
    }
    if (prev) {
        new_branch->next = curr;
        prev->next = new_branch;
    } else {
        new_branch->next = branches;
        branches = new_branch;
    }
}

/* Add a dungeon branch to the branch list. */
static branch *
add_branch(int dgn, int child_entry_level, struct proto_dungeon *pd)
{
    int branch_num;
    branch *new_branch;

    branch_num = find_branch(dungeons[dgn].dname, pd);
    new_branch = malloc(sizeof (branch));
    memset(new_branch, 0, sizeof (branch));
    new_branch->next = NULL;
    new_branch->id = branch_id++;
    new_branch->type = correct_branch_type(&pd->tmpbranch[branch_num]);
    new_branch->end1.dnum = parent_dnum(dungeons[dgn].dname, pd);
    new_branch->end1.dlevel = parent_dlevel(dungeons[dgn].dname, pd);
    new_branch->end2.dnum = dgn;
    new_branch->end2.dlevel = child_entry_level;
    new_branch->end1_up = pd->tmpbranch[branch_num].up ? TRUE : FALSE;

    insert_branch(new_branch, FALSE);
    return new_branch;
}

/*
 * Add new level to special level chain.  Insert it in level order with the
 * other levels in this dungeon.  This assumes that we are never given a
 * level that has a dungeon number less than the dungeon number of the
 * last entry.
 */
static void
add_level(s_level * new_lev)
{
    s_level *prev, *curr;

    prev = NULL;
    for (curr = sp_levchn; curr; curr = curr->next) {
        if (curr->dlevel.dnum == new_lev->dlevel.dnum &&
            curr->dlevel.dlevel > new_lev->dlevel.dlevel)
            break;
        prev = curr;
    }
    if (!prev) {
        new_lev->next = sp_levchn;
        sp_levchn = new_lev;
    } else {
        new_lev->next = curr;
        prev->next = new_lev;
    }
}

static void
init_level(int dgn, int proto_index, struct proto_dungeon *pd)
{
    s_level *new_level;
    struct tmplevel *tlevel = &pd->tmplevel[proto_index];

    pd->final_lev[proto_index] = NULL;  /* no "real" level */
    if (!wizard)
        if (tlevel->chance <= rn2(100))
            return;

    pd->final_lev[proto_index] = new_level = malloc(sizeof (s_level));
    memset(pd->final_lev[proto_index], 0, sizeof (s_level));

    /* load new level with data */
    strcpy(new_level->proto, tlevel->name);
    new_level->boneid = tlevel->boneschar;
    new_level->dlevel.dnum = dgn;
    new_level->dlevel.dlevel = 0;       /* for now */

    new_level->flags.town = ! !(tlevel->flags & TOWN);
    new_level->flags.hellish = ! !(tlevel->flags & HELLISH);
    new_level->flags.maze_like = ! !(tlevel->flags & MAZELIKE);
    new_level->flags.rogue_like = ! !(tlevel->flags & ROGUELIKE);
    new_level->flags.align = ((tlevel->flags & D_ALIGN_MASK) >> 4);
    if (!new_level->flags.align)
        new_level->flags.align =
            ((pd->tmpdungeon[dgn].flags & D_ALIGN_MASK) >> 4);

    new_level->rndlevs = tlevel->rndlevs;
    new_level->next = NULL;
}

static int
possible_places(int idx,        /* prototype index */
                boolean * map,  /* array MAXLEVEL+1 in length */
                struct proto_dungeon *pd)
{
    int i, start, count;
    s_level *lev = pd->final_lev[idx];

    /* init level possibilities */
    for (i = 0; i <= MAXLEVEL; i++)
        map[i] = FALSE;

    /* get base and range and set those entried to true */
    count =
        level_range(lev->dlevel.dnum, pd->tmplevel[idx].lev.base,
                    pd->tmplevel[idx].lev.rand, pd->tmplevel[idx].chain, pd,
                    &start);
    for (i = start; i < start + count; i++)
        map[i] = TRUE;

    /* mark off already placed levels */
    for (i = pd->start; i < idx; i++) {
        if (pd->final_lev[i] && map[pd->final_lev[i]->dlevel.dlevel]) {
            map[pd->final_lev[i]->dlevel.dlevel] = FALSE;
            --count;
        }
    }

    return count;
}

/* Pick the nth TRUE entry in the given boolean array. */
static xchar
pick_level(boolean * map,       /* an array MAXLEVEL+1 in size */
           int nth)
{
    int i;

    for (i = 1; i <= MAXLEVEL; i++)
        if (map[i] && !nth--)
            return (xchar) i;
    panic("pick_level:  ran out of valid levels");
    return 0;
}


/*
 * Place a level.  First, find the possible places on a dungeon map
 * template.  Next pick one.  Then try to place the next level->  If
 * sucessful, we're done.  Otherwise, try another (and another) until
 * all possible places have been tried.  If all possible places have
 * been exausted, return false.
 */
static boolean
place_level(int proto_index, struct proto_dungeon *pd)
{
    boolean map[MAXLEVEL + 1];  /* valid levels are 1..MAXLEVEL inclusive */
    s_level *lev;
    int npossible;

    if (proto_index == pd->n_levs)
        return TRUE;    /* at end of proto levels */

    lev = pd->final_lev[proto_index];

    /* No level created for this prototype, goto next. */
    if (!lev)
        return place_level(proto_index + 1, pd);

    npossible = possible_places(proto_index, map, pd);

    for (; npossible; --npossible) {
        lev->dlevel.dlevel = pick_level(map, rn2(npossible));
        if (place_level(proto_index + 1, pd))
            return TRUE;

        map[lev->dlevel.dlevel] = FALSE;        /* this choice didn't work */
    }
    return FALSE;
}


static const char *level_map[]  = {
    [sl_astral] = "astral",
    [sl_earth] = "earth",
    [sl_water] = "water",
    [sl_fire] = "fire",
    [sl_air] = "air",
    [sl_oracle] = "oracle",
    [sl_bigroom] = "bigroom",
    [sl_rogue] = "rogue",
    [sl_medusa] = "medusa",
    [sl_castle] = "castle",
    [sl_valley] = "valley",
    [sl_juiblex] = "juiblex",
    [sl_orcus] = "orcus",
    [sl_baal] = "ballz",
    [sl_asmo] = "asmodeus",
    [sl_wiztower1] = "wizard1",
    [sl_wiztower2] = "wizard2",
    [sl_wiztower3] = "wizard3",
    [sl_wiztower_portal] = "fakewiz1",
    [sl_sanctum] = "sanctum",
    [sl_quest_start] = X_START,
    [sl_quest_locate] = X_LOCATE,
    [sl_quest_goal] = X_GOAL,
    [sl_fort_ludios] = "knox",
};

void
init_dungeons(void)
{       /* initialize the "dungeon" structs */
    dlb *dgn_file;
    int i, cl = 0, cb = 0;
    s_level *x;
    struct proto_dungeon pd;
    struct version_info vers_info;

    pd.n_levs = pd.n_brs = 0;

    dgn_file = dlb_fopen(DUNGEON_FILE, RDBMODE);
    if (!dgn_file) {
        const char *tbuf = msgcat_many(
            "Cannot open dungeon description - \"", DUNGEON_FILE, "\" from\n",
            "\"", fqn_prefix[DATAPREFIX] ? fqn_prefix[DATAPREFIX] : "",
            DLBFILE, "\" file!", NULL);
        panic("%s", tbuf);
    }

    /* validate the data's version against the program's version */
    Fread(&vers_info, sizeof vers_info, 1, dgn_file);

    if (!check_version(&vers_info, DUNGEON_FILE, TRUE))
        panic("Dungeon description not valid.");

    /* 
     * Read in each dungeon and transfer the results to the internal
     * dungeon arrays.
     */
    sp_levchn = NULL;
    Fread(&n_dgns, sizeof (int), 1, dgn_file);
    if (n_dgns >= MAXDUNGEON)
        panic("init_dungeons: too many dungeons");

    for (i = 0; i < n_dgns; i++) {
        Fread(&pd.tmpdungeon[i], sizeof (struct tmpdungeon), 1, dgn_file);
        if (!wizard)
            if (pd.tmpdungeon[i].chance &&
                (pd.tmpdungeon[i].chance <= rn2(100))) {
                int j;

                /* skip over any levels or branches */
                for (j = 0; j < pd.tmpdungeon[i].levels; j++)
                    Fread(&pd.tmplevel[cl], sizeof (struct tmplevel), 1,
                          dgn_file);

                for (j = 0; j < pd.tmpdungeon[i].branches; j++)
                    Fread(&pd.tmpbranch[cb], sizeof (struct tmpbranch), 1,
                          dgn_file);
                n_dgns--;
                i--;
                continue;
            }

        strcpy(dungeons[i].dname, pd.tmpdungeon[i].name);
        strcpy(dungeons[i].proto, pd.tmpdungeon[i].protoname);
        dungeons[i].boneid = pd.tmpdungeon[i].boneschar;

        if (pd.tmpdungeon[i].lev.rand)
            dungeons[i].num_dunlevs =
                (xchar) rn1(pd.tmpdungeon[i].lev.rand,
                            pd.tmpdungeon[i].lev.base);
        else
            dungeons[i].num_dunlevs = (xchar) pd.tmpdungeon[i].lev.base;

        if (!i) {
            dungeons[i].ledger_start = 0;
            dungeons[i].depth_start = 1;
            dungeons[i].dunlev_ureached = 1;
        } else {
            dungeons[i].ledger_start =
                dungeons[i - 1].ledger_start + dungeons[i - 1].num_dunlevs;
            dungeons[i].dunlev_ureached = 0;
        }

        int j;
        for (j = 0; j < dungeons[i].num_dunlevs; ++j)
            levels[dungeons[i].ledger_start + j] =
                alloc_level(&(struct d_level){.dnum = i, .dlevel = j + 1});

        dungeons[i].flags.hellish = ! !(pd.tmpdungeon[i].flags & HELLISH);
        dungeons[i].flags.maze_like = ! !(pd.tmpdungeon[i].flags & MAZELIKE);
        dungeons[i].flags.rogue_like = ! !(pd.tmpdungeon[i].flags & ROGUELIKE);
        dungeons[i].flags.align =
            ((pd.tmpdungeon[i].flags & D_ALIGN_MASK) >> 4);
        /* 
         * Set the entry level for this dungeon.  The pd.tmpdungeon entry
         * value means:
         *          < 0     from bottom (-1 == bottom level)
         *            0     default (top)
         *          > 0     actual level (1 = top)
         *
         * Note that the entry_lev field in the dungeon structure is
         * redundant.  It is used only here and in print_dungeon().
         */
        if (pd.tmpdungeon[i].entry_lev < 0) {
            dungeons[i].entry_lev =
                dungeons[i].num_dunlevs + pd.tmpdungeon[i].entry_lev + 1;
            if (dungeons[i].entry_lev <= 0)
                dungeons[i].entry_lev = 1;
        } else if (pd.tmpdungeon[i].entry_lev > 0) {
            dungeons[i].entry_lev = pd.tmpdungeon[i].entry_lev;
            if (dungeons[i].entry_lev > dungeons[i].num_dunlevs)
                dungeons[i].entry_lev = dungeons[i].num_dunlevs;
        } else {        /* default */
            dungeons[i].entry_lev = 1;  /* defaults to top level */
        }

        if (i) {        /* set depth */
            branch *br;
            schar from_depth;
            boolean from_up;

            br = add_branch(i, dungeons[i].entry_lev, &pd);

            /* Get the depth of the connecting end. */
            if (br->end1.dnum == i) {
                from_depth = depth(&br->end2);
                from_up = !br->end1_up;
            } else {
                from_depth = depth(&br->end1);
                from_up = br->end1_up;
            }

            /* 
             * Calculate the depth of the top of the dungeon via
             * its branch.  First, the depth of the entry point:
             *
             *      depth of branch from "parent" dungeon
             *      + -1 or 1 depending on a up or down stair or
             *        0 if portal
             *
             * Followed by the depth of the top of the dungeon:
             *
             *      - (entry depth - 1)
             *
             * We'll say that portals stay on the same depth.
             */
            dungeons[i].depth_start =
                from_depth + (br->type == BR_PORTAL ? 0 : (from_up ? -1 : 1))
                - (dungeons[i].entry_lev - 1);
        }

        /* this is redundant - it should have been flagged by dgn_comp */
        if (dungeons[i].num_dunlevs > MAXLEVEL)
            dungeons[i].num_dunlevs = MAXLEVEL;

        pd.start = pd.n_levs;   /* save starting point */
        pd.n_levs += pd.tmpdungeon[i].levels;
        if (pd.n_levs > LEV_LIMIT)
            panic("init_dungeon: too many special levels");

        /* 
         * Read in the prototype special levels.  Don't add generated
         * special levels until they are all placed.
         */
        for (; cl < pd.n_levs; cl++) {
            Fread(&pd.tmplevel[cl], sizeof (struct tmplevel), 1, dgn_file);
            init_level(i, cl, &pd);
        }

        /* 
         * Recursively place the generated levels for this dungeon.  This
         * routine will attempt all possible combinations before giving
         * up.
         */
        if (!place_level(pd.start, &pd))
            panic("init_dungeon:  couldn't place levels");

        for (; pd.start < pd.n_levs; pd.start++)
            if (pd.final_lev[pd.start])
                add_level(pd.final_lev[pd.start]);


        pd.n_brs += pd.tmpdungeon[i].branches;
        if (pd.n_brs > BRANCH_LIMIT)
            panic("init_dungeon: too many branches");
        for (; cb < pd.n_brs; cb++)
            Fread(&pd.tmpbranch[cb], sizeof (struct tmpbranch), 1, dgn_file);
    }
    dlb_fclose(dgn_file);

    for (i = 0; i < 5; i++)
        tune[i] = 'A' + rn2(7);
    tune[5] = 0;

    /* 
     * Find most of the special levels and dungeons so we can access their
     * locations quickly.
     */
    for (i = 0; i < num_special_levels; ++i) {
        /* 
         * suppress finding the rogue level if it is not enabled
         * this makes x == sp_lev(sl_rogue) fail
         */
        if (!strncmp(level_map[i], "rogue", 6) && !flags.rogue_enabled)
            continue;

        x = find_level(level_map[i]);
        if (x) {
            dungeon_topology.special_levels[i] = levels[ledger_no(&x->dlevel)];
            if (!strncmp(level_map[i], "x-", 2)) {
                /* This is where the name substitution on the levels of the
                   quest dungeon occur. */
                snprintf(x->proto, SIZE(x->proto), "%s%s", urole.filecode,
                         &level_map[i][1]);
            } else if (i == sl_fort_ludios) {
                branch *br;

                /* 
                 * Kludge to allow floating Knox entrance.  We
                 * specify a floating entrance by the fact that
                 * its entrance (end1) has a bogus dnum, namely
                 * n_dgns.
                 */
                for (br = branches; br; br = br->next)
                    if (on_level(&br->end2,
                                 &dungeon_topology.special_levels[i]->z))
                        break;

                if (br)
                    br->end1.dnum = n_dgns;
                /* adjust the branch's position on the list */
                insert_branch(br, TRUE);
            }
        }
    }

    if (!flags.rogue_enabled) {
        /* 
         * remove rogue level from the special level chain
         * This makes Is_special(x) return NULL, so the level will never be
         * instantiated
         */
        s_level *s_curr, *s_prev;

        s_prev = NULL;
        for (s_curr = sp_levchn; s_curr; s_curr = s_curr->next) {
            if (!strncmp(s_curr->proto, "rogue", 6)) {
                if (s_prev)
                    s_prev->next = s_curr->next;
                else
                    sp_levchn = s_curr->next;

                break;
            }
            s_prev = s_curr;
        }
    }

/*
 * I hate hardwiring these names. :-(
 */
    dungeon_topology.d_quest_dnum = dname_to_dnum("The Quest");
    dungeon_topology.d_sokoban_dnum = dname_to_dnum("Sokoban");
    dungeon_topology.d_mines_dnum = dname_to_dnum("The Gnomish Mines");
    dungeon_topology.d_tower_dnum = dname_to_dnum("Vlad's Tower");
    dungeon_topology.d_endgame_dnum = dname_to_dnum("The Elemental Planes");

    /* one special fixup for dummy surface level */
    if ((x = find_level("dummy")) != 0) {
        i = x->dlevel.dnum;
        /* the code above puts earth one level above dungeon level #1, making
           the dummy level overlay level 1; but the whole reason for having the 
           dummy level is to make earth have depth -1 instead of 0, so adjust
           the start point to shift endgame up */
        if (dunlevs_in_dungeon(&x->dlevel) > 1 - dungeons[i].depth_start)
            dungeons[i].depth_start -= 1;
        /* TO DO: strip "dummy" out all the way here, so that it's hidden from
           <ctrl/O> feedback. */
    }
}

xchar
dunlev(const struct level *lev)
{       /* return the level number for lev in *this* dungeon */
    return lev->z.dlevel;
}

/* return the lowest level number for *this* dungeon*/
xchar
dunlevs_in_dungeon(const d_level *dlev)
{
    return dungeons[dlev->dnum].num_dunlevs;
}

/* return the lowest level explored in the game*/
xchar
deepest_lev_reached(boolean noquest)
{
    /* this function is used for three purposes: to provide a factor of
       difficulty in monster generation; to provide a factor of difficulty in
       experience calculations (botl.c and end.c); and to insert the deepest
       level reached in the game in the topten display.  the 'noquest' arg
       switch is required for the latter. from the player's point of view,
       going into the Quest is _not_ going deeper into the dungeon -- it is
       going back "home", where the dungeon starts at level 1.  given the setup 
       in dungeon.def, the depth of the Quest (thought of as starting at level
       1) is never lower than the level of entry into the Quest, so we exclude
       the Quest from the topten "deepest level reached" display calculation.
       _However_ the Quest is a difficult dungeon, so we include it in the
       factor of difficulty calculations. */
    int i;
    d_level tmp;
    schar ret = 0;

    for (i = 0; i < n_dgns; i++) {
        if ((tmp.dlevel = dungeons[i].dunlev_ureached) == 0)
            continue;
        if (!strcmp(dungeons[i].dname, "The Quest") && noquest)
            continue;

        tmp.dnum = i;
        if (depth(&tmp) > ret)
            ret = depth(&tmp);
    }
    return (xchar) ret;
}

/* return a bookkeeping level number for purpose of comparisons and
 * save/restore */
xchar
ledger_no(const d_level * lev)
{
    return (xchar) (lev->dlevel + dungeons[lev->dnum].ledger_start - 1);
}

/*
 * The last level in the bookkeeping list of level is the bottom of the last
 * dungeon in the dungeons[] array.
 *
 * Maxledgerno() -- which is the max number of levels in the bookkeeping
 * list, should not be confused with dunlevs_in_dungeon(lev) -- which
 * returns the max number of levels in lev's dungeon, and both should
 * not be confused with deepest_lev_reached() -- which returns the lowest
 * depth visited by the player.
 */
xchar
maxledgerno(void)
{
    return (xchar) (dungeons[n_dgns - 1].ledger_start +
                    dungeons[n_dgns - 1].num_dunlevs - 1);
}

/* return the dungeon that this ledgerno exists in */
xchar
ledger_to_dnum(xchar ledgerno)
{
    int i;

    /* find i such that (i->base + 1) <= ledgerno <= (i->base + i->count) */
    for (i = 0; i < n_dgns; i++)
        if (dungeons[i].ledger_start <= ledgerno &&
            ledgerno < dungeons[i].ledger_start + dungeons[i].num_dunlevs)
            return (xchar) i;

    panic("level number out of range [ledger_to_dnum(%d)]", (int)ledgerno);
    /* NOT REACHED */
    return (xchar) 0;
}

/* return the level of the dungeon this ledgerno exists in */
xchar
ledger_to_dlev(xchar ledgerno)
{
    return (xchar) (ledgerno - dungeons[ledger_to_dnum(ledgerno)].ledger_start);
}


/* returns the depth of a level, in floors below the surface    */
/* (note levels in different dungeons can have the same depth). */
schar
depth(const d_level * lev)
{
    return (schar) (dungeons[lev->dnum].depth_start + lev->dlevel - 1);
}

/* are "lev1" and "lev2" actually the same? */
boolean
on_level(const d_level * lev1, const d_level * lev2)
{
    return (boolean) ((lev1->dnum == lev2->dnum) &&
                      (lev1->dlevel == lev2->dlevel));
}


/* is this level referenced in the special level chain? */
s_level *
Is_special(const d_level * lev)
{
    s_level *levtmp;

    for (levtmp = sp_levchn; levtmp; levtmp = levtmp->next)
        if (on_level(lev, &levtmp->dlevel))
            return levtmp;

    return NULL;
}

/*
 * Is this a multi-dungeon branch level?  If so, return a pointer to the
 * branch.  Otherwise, return null.
 */
branch *
Is_branchlev(const struct level *lev)
{
    branch *curr;

    for (curr = branches; curr; curr = curr->next) {
        if (on_level(&lev->z, &curr->end1) || on_level(&lev->z, &curr->end2))
            return curr;
    }
    return NULL;
}

/* goto the next level (or appropriate dungeon) */
void
next_level(boolean at_stairs)
{
    if (at_stairs && u.ux == level->sstairs.sx && u.uy == level->sstairs.sy) {
        /* Taking a down dungeon branch. */
        goto_level(&level->sstairs.tolev, at_stairs, FALSE, FALSE);
    } else {
        /* Going down a stairs or jump in a trap door. */
        d_level newlevel;

        newlevel.dnum = level->z.dnum;
        newlevel.dlevel = level->z.dlevel + 1;
        goto_level(&newlevel, at_stairs, !at_stairs, FALSE);
    }
}

/* goto the previous level (or appropriate dungeon) */
void
prev_level(boolean at_stairs)
{
    if (at_stairs && u.ux == level->sstairs.sx && u.uy == level->sstairs.sy) {
        /* Taking an up dungeon branch. */
        /* KMH -- Upwards branches are okay if not level 1 */
        /* (Just make sure it doesn't go above depth 1) */
        if (!level->z.dnum && level->z.dlevel == 1 && !Uhave_amulet)
            done(ESCAPED, NULL);
        else
            goto_level(&level->sstairs.tolev, at_stairs, FALSE, FALSE);
    } else {
        /* Going up a stairs or rising through the ceiling. */
        d_level newlevel;

        newlevel.dnum = level->z.dnum;
        newlevel.dlevel = level->z.dlevel - 1;
        goto_level(&newlevel, at_stairs, FALSE, FALSE);
    }
}

void
u_on_newpos(int x, int y)
{
    if (!isok(u.ux, u.uy))
        panic("placing player outside the map boundaries");
    u.ux = x;
    u.uy = y;
    /* ridden steed always shares hero's location */
    if (u.usteed)
        u.usteed->mx = u.ux, u.usteed->my = u.uy;
}

static boolean
badspot(xchar x, xchar y)
{
    return (level->locations[x][y].typ != ROOM &&
            level->locations[x][y].typ != CORR) || MON_AT(level, x, y);
}

/* place you on the special staircase */
void
u_on_sstairs(void)
{

    if (isok(level->sstairs.sx, level->sstairs.sy)) {
        u_on_newpos(level->sstairs.sx, level->sstairs.sy);
    } else {
        /* code stolen from goto_level */
        int trycnt = 0;
        xchar x, y;

        do {
            x = rn2(COLNO);
            y = rn2(ROWNO);
            if (!badspot(x, y)) {
                u_on_newpos(x, y);
                return;
            }
        } while (++trycnt <= 500);
        panic("u_on_sstairs: could not relocate player!");
    }
}

/* place you on upstairs (or special equivalent) */
void
u_on_upstairs(void)
{
    if (isok(level->upstair.sx, level->upstair.sy)) {
        u_on_newpos(level->upstair.sx, level->upstair.sy);
    } else
        u_on_sstairs();
}

/* place you on level->dnstairs (or special equivalent) */
void
u_on_dnstairs(void)
{
    if (isok(level->dnstair.sx, level->dnstair.sy)) {
        u_on_newpos(level->dnstair.sx, level->dnstair.sy);
    } else
        u_on_sstairs();
}

boolean
On_stairs(xchar x, xchar y)
{
    return (boolean) ((x == level->upstair.sx && y == level->upstair.sy) ||
                      (x == level->dnstair.sx && y == level->dnstair.sy) ||
                      (x == level->dnladder.sx && y == level->dnladder.sy) ||
                      (x == level->upladder.sx && y == level->upladder.sy) ||
                      (x == level->sstairs.sx && y == level->sstairs.sy));
}

boolean
Is_botlevel(const struct level *lev)
{
    return (lev->z.dlevel == dungeons[lev->z.dnum].num_dunlevs);
}

boolean
can_dig_down(const struct level * lev)
{
    return (!lev->flags.hardfloor && !Is_botlevel(lev) &&
            !Invocation_lev(lev));
}

/*
 * Like Can_dig_down (above), but also allows falling through on the
 * stronghold level->  Normally, the bottom level of a dungeon resists
 * both digging and falling.
 */
boolean
can_fall_thru(const struct level * lev)
{
    return (can_dig_down(lev) || lev == sp_lev(sl_castle));
}

/*
 * True if one can rise up a level (e.g. cursed gain level).
 * This happens on intermediate dungeon levels or on any top dungeon
 * level that has a stairwell style branch to the next higher dungeon.
 * Checks for amulets and such must be done elsewhere.
 */
boolean
Can_rise_up(int x, int y, const struct level *lev)
{
    /* can't rise up from inside the top of the Wizard's tower */
    /* KMH -- or in sokoban */
    if (In_endgame(lev) || In_sokoban(lev) ||
        (lev == sp_lev(sl_wiztower1) && In_W_tower(x, y, lev)))
        return FALSE;
    return (boolean) (lev->z.dlevel > 1 ||
                      (dungeons[lev->z.dnum].entry_lev == 1 &&
                       ledger_no(&lev->z) != 1 &&
                       isok(lev->sstairs.sx, level->sstairs.sy) &&
                       lev->sstairs.up));
}

/*
 * It is expected that the second argument of get_level is a depth value,
 * either supplied by the user (teleport control) or randomly generated.
 * But more than one level can be at the same depth.  If the target level
 * is "above" the present depth location, get_level must trace "up" from
 * the player's location (through the ancestors dungeons) the dungeon
 * within which the target level is located.  With only one exception
 * which does not pass through this routine (see level_tele), teleporting
 * "down" is confined to the current dungeon.  At present, level teleport
 * in dungeons that build up is confined within them.
 */
void
get_level(d_level * newlevel, int levnum)
{
    branch *br;
    xchar dgn = level->z.dnum;

    if (levnum <= 0) {
        /* can only currently happen in endgame */
        levnum = level->z.dlevel;
    } else if (levnum >
               dungeons[dgn].depth_start + dungeons[dgn].num_dunlevs - 1) {
        /* beyond end of dungeon, jump to last level */
        levnum = dungeons[dgn].num_dunlevs;
    } else {
        /* The desired level is in this dungeon or a "higher" one. */

        /* 
         * Branch up the tree until we reach a dungeon that contains the
         * levnum.
         */
        if (levnum < dungeons[dgn].depth_start) {

            do {
                /* 
                 * Find the parent dungeon of this dungeon.
                 *
                 * This assumes that end2 is always the "child" and it is
                 * unique.
                 */
                for (br = branches; br; br = br->next)
                    if (br->end2.dnum == dgn)
                        break;
                if (!br)
                    panic("get_level: can't find parent dungeon");

                dgn = br->end1.dnum;
            } while (levnum < dungeons[dgn].depth_start);
        }

        /* We're within the same dungeon; calculate the level. */
        levnum = levnum - dungeons[dgn].depth_start + 1;
    }

    newlevel->dnum = dgn;
    newlevel->dlevel = levnum;
}


boolean
In_quest(const struct level *lev)
{       /* are you in the quest dungeon? */
    return (boolean) (lev->z.dnum == dungeon_topology.d_quest_dnum);
}


boolean
In_mines(const struct level *lev)
{       /* are you in the mines dungeon? */
    return (boolean) (lev->z.dnum == dungeon_topology.d_mines_dnum);
}

/*
 * Return the branch for the given dungeon.
 *
 * This function assumes:
 *      + This is not called with "Dungeons of Doom".
 *      + There is only _one_ branch to a given dungeon.
 *      + Field end2 is the "child" dungeon.
 */
branch *
dungeon_branch(const char *s)
{
    branch *br;
    xchar dnum;

    dnum = dname_to_dnum(s);

    /* Find the branch that connects to dungeon i's branch. */
    for (br = branches; br; br = br->next)
        if (br->end2.dnum == dnum)
            break;

    if (!br)
        panic("dgn_entrance: can't find entrance to %s", s);

    return br;
}

/*
 * This returns true if the hero is on the same level as the entrance to
 * the named dungeon.
 *
 * Called from do.c and mklev.c.
 *
 * Assumes that end1 is always the "parent".
 */
boolean
at_dgn_entrance(const struct level *lev, const char *s)
{
    branch *br;

    br = dungeon_branch(s);
    return (boolean) (on_level(&lev->z, &br->end1) ? TRUE : FALSE);
}

boolean
In_V_tower(const struct level *lev)
{       /* is `lev' part of Vlad's tower? */
    return (boolean) (lev->z.dnum == dungeon_topology.d_tower_dnum);
}

boolean
On_W_tower_level(const struct level *lev)
{       /* is `lev' a level containing the Wizard's tower? */
    return lev == sp_lev(sl_wiztower1) || lev == sp_lev(sl_wiztower2) ||
           lev == sp_lev(sl_wiztower3);
}

/* is <x,y> of `lev' inside the Wizard's tower? */
boolean
In_W_tower(int x, int y, const struct level *lev)
{
    if (!On_W_tower_level(lev))
        return FALSE;
    /* 
     * Both of the exclusion regions for arriving via level teleport
     * (from above or below) define the tower's boundary.
     *      assert( updest.nIJ == dndest.nIJ for I={l|h},J={x|y} );
     */
    if (lev->dndest.nlx > 0)
        return (boolean) within_bounded_area(x, y, lev->dndest.nlx,
                                             lev->dndest.nly,
                                             lev->dndest.nhx,
                                             lev->dndest.nhy);
    else
        impossible("No boundary for Wizard's Tower?");
    return FALSE;
}


boolean
In_hell(const struct level *lev)
{       /* are you in one of the Hell levels? */
    return (boolean) (dungeons[lev->z.dnum].flags.hellish);
}


void
assign_level(d_level * dest, const d_level * src)
{       /* equivalent to dest = source */
    dest->dnum = src->dnum;
    dest->dlevel = src->dlevel;
}

/* dest = src + rn1(range) */
void
assign_rnd_level(d_level * dest, const d_level * src, int range)
{
    dest->dnum = src->dnum;
    dest->dlevel = src->dlevel + ((range > 0) ? rnd(range) : -rnd(-range));

    if (dest->dlevel > dunlevs_in_dungeon(dest))
        dest->dlevel = dunlevs_in_dungeon(dest);
    else if (dest->dlevel < 1)
        dest->dlevel = 1;
}


int
induced_align(const struct level *lev, int pct)
{
    s_level *slev = Is_special(&lev->z);
    aligntyp al;

    if (slev && slev->flags.align)
        if (rn2(100) < pct)
            return slev->flags.align;

    if (dungeons[lev->z.dnum].flags.align)
        if (rn2(100) < pct)
            return dungeons[lev->z.dnum].flags.align;

    al = rn2(3) - 1;
    return Align2amask(al);
}


boolean
Invocation_lev(const struct level *lev)
{
    return (boolean) (In_hell(lev) &&
                      lev->z.dlevel == (dungeons[lev->z.dnum].num_dunlevs - 1));
}

/* use instead of depth() wherever a degree of difficulty is made
 * dependent on the location in the dungeon (eg. monster creation).
 */
xchar
level_difficulty(const struct level *lev)
{
    if (In_endgame(lev))
        return (xchar) (depth(&sp_lev(sl_sanctum)->z) + u.ulevel/2);
    else if (Uhave_amulet)
        return deepest_lev_reached(FALSE);
    else
        return (xchar) depth(&lev->z);
}

/* Take one word and try to match it to a level.

   Recognized levels are as shown by print_dungeon(). */
schar
lev_by_name(const char *nam)
{
    return 0;
#if 0 /* FIXME: Make this function work again */
    schar lev = 0;
    s_level *slev;
    d_level dlev;
    const char *p;
    int idx, idxtoo;

    /* allow strings like "the oracle level" to find "oracle" */
    if (!strncmpi(nam, "the ", 4))
        nam += 4;
    if ((p = strstri(nam, " level")) != 0 && p == nam + strlen(nam) - 6)
        nam = msgchop(nam, -6);

    /* hell is the old name, and wouldn't match; gehennom would match its
       branch, yielding the castle level instead of the valley of the dead */
    if (!strcmpi(nam, "gehennom") || !strcmpi(nam, "hell")) {
        if (In_V_tower(level))
            nam = " to Vlad's tower";   /* branch to... */
        else
            nam = "valley";
    }

    if ((slev = find_level(nam)) != 0) {
        dlev = slev->dlevel;
        idx = ledger_no(&dlev);
        if ((dlev.dnum == level->z.dnum ||
             /* within same branch, or else main dungeon <-> gehennom */
             (level->z.dnum == valley_level.dnum && dlev.dnum == medusa_level.dnum)
             || (level->z.dnum == medusa_level.dnum &&
                 dlev.dnum == valley_level.dnum)) &&
            /* either wizard mode or else seen and not forgotten */
            /* FIXME: This assumes all generated levels have been seen. That's
             * false. */
            (wizard || (levels[idx]->generated && !levels[idx]->flags.forgotten))) {
            lev = depth(&slev->dlevel);
        }
    } else {    /* not a specific level; try branch names */
        idx = find_branch(nam, NULL);
        /* "<branch> to Xyzzy" */
        if (idx < 0 && (p = strstri(nam, " to ")) != 0)
            idx = find_branch(p + 4, NULL);

        if (idx >= 0) {
            /* FIXME: WTF */
            idxtoo = (idx >> 8) & 0x00FF;
            idx &= 0x00FF;
            if (        /* either wizard mode, or else _both_ sides of branch
                           seen */
                   wizard || ((levels[idx]->generated &&
                               !levels[idx]->flags.forgotten) &&
                              (levels[idxtoo]->generated &&
                               !levels[idxtoo]->flags.forgotten))) {
                if (ledger_to_dnum(idxtoo) == level->z.dnum)
                    idx = idxtoo;
                dlev.dnum = ledger_to_dnum(idx);
                dlev.dlevel = ledger_to_dlev(idx);
                lev = depth(&dlev);
            }
        }
    }
    return lev;
#endif
}


/* Convert a branch type to a string usable by print_dungeon(). */
static const char *
br_string(int type)
{
    switch (type) {
    case BR_PORTAL:
        return "portal";
    case BR_NO_END1:
        return "connection";
    case BR_NO_END2:
        return "one way stair";
    case BR_STAIR:
        return "stair";
    }
    return " (unknown)";
}


/* Print all child branches between the lower and upper bounds. */
static void
print_branch(struct nh_menulist *menu, int dnum,
             int lower_bound, int upper_bound,
             boolean bymenu, struct lchoice *lchoices)
{
    branch *br;
    const char *buf;

    /* This assumes that end1 is the "parent". */
    for (br = branches; br; br = br->next) {
        if (br->end1.dnum == dnum && lower_bound < br->end1.dlevel &&
            br->end1.dlevel <= upper_bound) {
            buf = msgprintf("   %s to %s: %d", br_string(br->type),
                            dungeons[br->end2.dnum].dname, depth(&br->end1));
            if (bymenu) {
                lchoices->lev[lchoices->idx] = br->end1.dlevel;
                lchoices->dgn[lchoices->idx] = br->end1.dnum;
                lchoices->playerlev[lchoices->idx] = depth(&br->end1);

                add_menuitem(menu, lchoices->idx + 1, buf, lchoices->menuletter,
                             FALSE);
                if (lchoices->menuletter == 'z')
                    lchoices->menuletter = 'A';
                else if (lchoices->menuletter == 'Z')
                    lchoices->menuletter = 'a';
                else
                    lchoices->menuletter++;
                lchoices->idx++;
            } else
                add_menutext(menu, buf);
        }
    }
}

/* Print available dungeon information. */
schar
print_dungeon(boolean bymenu, schar * rlev, xchar * rdgn)
{
    int i, last_level, nlev;
    boolean first;
    s_level *slev;
    dungeon *dptr;
    branch *br;
    struct lchoice lchoices;
    struct nh_menulist menu;
    const char *buf;

    init_menulist(&menu);

    if (bymenu) {
        lchoices.idx = 0;
        lchoices.menuletter = 'a';
    }

    for (i = 0, dptr = dungeons; i < n_dgns; i++, dptr++) {
        nlev = dptr->num_dunlevs;
        if (nlev > 1)
            buf = msgprintf("%s: levels %d to %d", dptr->dname,
                            dptr->depth_start, dptr->depth_start + nlev - 1);
        else
            buf = msgprintf("%s: level %d", dptr->dname, dptr->depth_start);

        /* Most entrances are uninteresting. */
        if (dptr->entry_lev != 1) {
            if (dptr->entry_lev == nlev)
                buf = msgcat(buf, ", entrance from below");
            else
                buf = msgprintf("%s, entrance on %d", buf,
                                dptr->depth_start + dptr->entry_lev - 1);
        }
        if (bymenu) {
            add_menuheading(&menu, buf);
        } else
            add_menutext(&menu, buf);

        /* 
         * Circle through the special levels to find levels that are in
         * this dungeon.
         */
        for (slev = sp_levchn, last_level = 0; slev; slev = slev->next) {
            if (slev->dlevel.dnum != i)
                continue;

            /* print any branches before this level */
            print_branch(&menu, i, last_level, slev->dlevel.dlevel, bymenu,
                         &lchoices);

            buf = msgprintf("   %s: %d", slev->proto, depth(&slev->dlevel));
            if (on_level(&slev->dlevel, &sp_lev(sl_castle)->z))
                buf = msgprintf("%s (tune %s)", buf, tune);
            if (bymenu) {
                /* If other floating branches are added, this will need to
                   change */
                if (i != sp_lev(sl_fort_ludios)->z.dnum) {
                    lchoices.lev[lchoices.idx] = slev->dlevel.dlevel;
                    lchoices.dgn[lchoices.idx] = i;
                } else {
                    lchoices.lev[lchoices.idx] = depth(&slev->dlevel);
                    lchoices.dgn[lchoices.idx] = 0;
                }
                lchoices.playerlev[lchoices.idx] = depth(&slev->dlevel);

                add_menuitem(&menu, lchoices.idx + 1, buf, lchoices.menuletter,
                             FALSE);
                if (lchoices.menuletter == 'z')
                    lchoices.menuletter = 'A';
                else if (lchoices.menuletter == 'Z')
                    lchoices.menuletter = 'a';
                else
                    lchoices.menuletter++;
                lchoices.idx++;
            } else
                add_menutext(&menu, buf);

            last_level = slev->dlevel.dlevel;
        }
        /* print branches after the last special level */
        print_branch(&menu, i, last_level, MAXLEVEL, bymenu, &lchoices);
    }

    /* Print out floating branches (if any). */
    for (first = TRUE, br = branches; br; br = br->next) {
        if (br->end1.dnum == n_dgns) {
            if (first) {
                if (!bymenu) {
                    add_menutext(&menu, "");
                    add_menutext(&menu, "Floating branches");
                }
                first = FALSE;
            }
            buf = msgprintf("   %s to %s", br_string(br->type),
                            dungeons[br->end2.dnum].dname);
            if (!bymenu)
                add_menutext(&menu, buf);
        }
    }

    if (bymenu) {
        int n;
        const int *selected;
        int idx;

        n = display_menu(&menu, "Level teleport to where:",
                         PICK_ONE, PLHINT_ANYWHERE, &selected);
        if (n > 0) {
            idx = selected[0] - 1;
            if (rlev && rdgn) {
                *rlev = lchoices.lev[idx];
                *rdgn = lchoices.dgn[idx];
                return lchoices.playerlev[idx];
            }
        }
        return 0;
    }

    /* 
     * The following is based on the assumption that the inter-level portals
     * created by the level compiler (not the dungeon compiler) only exist
     * one per level (currently true, of course).
     */
    else if (level == sp_lev(sl_earth) || level == sp_lev(sl_water)
             || level == sp_lev(sl_fire) || level == sp_lev(sl_air)) {
        struct trap *trap;

        for (trap = level->lev_traps; trap; trap = trap->ntrap)
            if (trap->ttyp == MAGIC_PORTAL)
                break;

        add_menutext(&menu, "");
        if (trap)
            buf = msgprintf("Portal @ (%d,%d), hero @ (%d,%d)",
                            trap->tx, trap->ty, u.ux, u.uy);
        else
            buf = "No portal found.";
        add_menutext(&menu, buf);
    }

    display_menu(&menu, "Level teleport to where:", PICK_NONE,
                 PLHINT_ANYWHERE, NULL);
    return 0;
}


/* add a custom name to the current level */
int
donamelevel(const struct nh_cmd_arg *arg)
{
    const char *query, *buf;

    if (level->levname[0])
        query = msgprintf("Replace previous name \"%s\" with?", level->levname);
    else
        query = "What do you want to call this dungeon level?";
    buf = getarglin(arg, query);

    if (buf[0] == '\033')
        return 0;

    strncpy(level->levname, buf, sizeof (level->levname) - 1);
    level->levname[sizeof (level->levname) - 1] = '\0';
    return 0;
}


static boolean
overview_is_interesting(const struct level *lev, const struct overview_info *oi)
{
    /* not interesting if it hasn't been created yet */
    if (!lev)
        return FALSE;

    /* interesting if you're on this level */
    if (lev == level)
        return TRUE;

    /* interesting if it is named ("stash", "danger, demon!") */
    if (*lev->levname)
        return TRUE;

    /* if overview_scan found _anything_ the level is also interesting */
    if (oi->fountains || oi->sinks || oi->thrones || oi->trees || oi->temples ||
        oi->altars || oi->shopcount || oi->branch || oi->portal)
        return TRUE;

    /* "boring" describes this level very well */
    return FALSE;
}


static void
overview_scan(const struct level *lev, struct overview_info *oi)
{
    int x, y, rnum, rtyp;
    struct trap *trap;
    boolean seen_shop[MAXNROFROOMS * 2];

    memset(seen_shop, 0, sizeof (seen_shop));
    memset(oi, 0, sizeof (struct overview_info));
    if (!lev)
        return;

    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            if (!lev->locations[x][y].seenv)
                continue;

            switch (lev->locations[x][y].mem_bg) {
            case S_upstair:
            case S_dnstair:
            case S_upladder:
            case S_dnladder:
            case S_upsstair:
            case S_dnsstair:
                if (lev->sstairs.sx == x && lev->sstairs.sy == y &&
                    lev->sstairs.tolev.dnum != lev->z.dnum) {
                    oi->branch = TRUE;
                    if (levels[ledger_no(&lev->sstairs.tolev)]->generated) {
                        oi->branch_dst_known = TRUE;
                        oi->branch_dst = lev->sstairs.tolev;
                    } else {
                        oi->branch_dst_known = FALSE;
                    }
                }
                break;

            case S_fountain:
                oi->fountains++;
                break;

            case S_sink:
                oi->sinks++;
                break;

            case S_throne:
                oi->thrones++;  /* don't care about throne rooms */
                break;

            case S_altar:
                if (lev->locations[x][y].altarmask & AM_SANCTUM) {
                    oi->high_altars++;
                    /* Don't count high altars as altars to avoid leaking
                       alignment information. */
                    break;
                }
                oi->altars++;
                /* Check altar's alignment. */
                if (lev->locations[x][y].altarmask & AM_LAWFUL)
                    oi->lawful_altar = TRUE;
                else if (lev->locations[x][y].altarmask & AM_NEUTRAL)
                    oi->neutral_altar = TRUE;
                else if (lev->locations[x][y].altarmask & AM_CHAOTIC)
                    oi->chaotic_altar = TRUE;
                else
                    oi->unaligned_altar = TRUE;
                rnum = lev->locations[x][y].roomno;
                if (rnum >= ROOMOFFSET &&
                    lev->rooms[rnum - ROOMOFFSET].rtype == TEMPLE)
                    oi->temples++;
                break;

            case S_tree:
                oi->trees++;
                break;

            case S_room:
                rnum = lev->locations[x][y].roomno;
                if (rnum < ROOMOFFSET)
                    break;

                rtyp = lev->rooms[rnum - ROOMOFFSET].rtype;
                if (rtyp >= SHOPBASE && !seen_shop[rnum]) {
                    seen_shop[rnum] = TRUE;
                    if (oi->shopcount == 0)
                        oi->shoptype = rtyp - SHOPBASE;
                    else
                        oi->shoptype = -1;      /* multiple shops */
                    oi->shopcount++;
                }
            }
        }
    }

    /* find the magic portal, if it exists */
    for (trap = lev->lev_traps; trap; trap = trap->ntrap)
        if (trap->tseen && trap->ttyp == MAGIC_PORTAL) {
            oi->portal = TRUE;
            if (levels[ledger_no(&trap->dst)]->generated) {
                oi->portal_dst_known = TRUE;
                oi->portal_dst = trap->dst;
            } else {
                oi->portal_dst_known = FALSE;
            }
        }
}


static const char *
overview_print_dun(const struct level *lev)
{
    int dnum = lev->z.dnum;
    int depthstart = dungeons[dnum].depth_start;
    int entry_depth, reached_depth;
    const char *rv;

    if (In_quest(lev) || lev == sp_lev(sl_fort_ludios))
        /* The quest and knox should appear to be level 1 to match other text.
           */
        depthstart = 1;

    entry_depth = depthstart + dungeons[dnum].entry_lev - 1;
    reached_depth = depthstart + dungeons[dnum].dunlev_ureached - 1;
    if (entry_depth == reached_depth || In_endgame(lev))
        /* Suppress the negative numbers in the endgame. */
        rv = msgcat(dungeons[dnum].dname, ":");
    else {
        rv = msgprintf(
            "%s: levels %d to %d", dungeons[dnum].dname,
            entry_depth < reached_depth ? entry_depth : reached_depth,
            entry_depth < reached_depth ? reached_depth : entry_depth);
    }
    return rv;
}


static const char *
overview_print_lev(const struct level *lev)
{
    int i, depthstart;
    const char *buf;

    depthstart = dungeons[lev->z.dnum].depth_start;
    if (In_quest(lev) || lev == sp_lev(sl_fort_ludios))
        /* The quest and knox should appear to be level 1 to match other text.
           */
        depthstart = 1;

    /* calculate level number */
    i = depthstart + lev->z.dlevel - 1;
    if (lev == sp_lev(sl_astral))
        buf = "Astral Plane";
    else if (In_endgame(lev))
        /* Negative numbers are mildly confusing, since they are never shown to 
           the player, except in wizard mode.  We could show "Level -1" for the 
           earth plane, for example.  Instead, show "Plane 1" for the earth
           plane to differentiate from level 1.  There's not much to show, but
           maybe the player wants to #annotate them for some bizarre reason. */
        buf = msgprintf("Plane %i", -i);
    else
        buf = msgprintf("Level %d", i);

    if (*lev->levname)
        buf = msgprintf("%s (%s)", buf, lev->levname);

    if (lev == level)
        buf = msgcat(buf, (program_state.gameover ? " <- You were here" :
                           " <- You are here"));

    return buf;
}


static const char *
seen_string(xchar x, const char *obj)
{
    /* players are computer scientists: 0, 1, 2, n */
    switch (x) {
    case 0:
        return "no";
        /* an() returns too much.  index is ok in this case */
    case 1:
        return strchr(vowels, *obj) ? "an" : "a";
    case 2:
    case 3:
        return "some";
    default:
        return "many";
    }
}


#define COMMA (i++ > 0 ? ", " : "      ")
#define ADDNTOBUF(nam, var) do { \
        if (var)                                                        \
            buf = msgprintf("%s%s%s " nam "%s", buf, COMMA,             \
                            seen_string((var), (nam)),                  \
                            ((var) != 1 ? "s" : ""));                   \
    } while(0)

#if MAXRTYPE != CANDLESHOP
# warning you must extend the shopnames array!
#endif
static const char *const shopnames[] = {
    /* SHOPBASE */ "a general store",
    /* ARMORSHOP */ "an armor shop",
    /* SCROLLSHOP */ "a scroll shop",
    /* POTIONSHOP */ "a potion shop",
    /* WEAPONSHOP */ "a weapon shop",
    /* FOODSHOP */ "a delicatessen",
    /* RINGSHOP */ "a jewelry store",
    /* WANDSHOP */ "a wand shop",
    /* TOOLSHOP */ "a hardware shop",
    /* BOOKSHOP */ "a bookstore",
    /* CANDLESHOP */ "a lighting shop"
};

static const char *
overview_print_gods(const struct overview_info *oi)
{
    int i, num_gods = 0;
    const char *(god_names[4]);
    const char *buf = "";
    if (oi->lawful_altar) {
        god_names[num_gods] = align_gname(A_LAWFUL);
        num_gods++;
    }
    if (oi->neutral_altar) {
        god_names[num_gods] = align_gname(A_NEUTRAL);
        num_gods++;
    }
    if (oi->chaotic_altar) {
        god_names[num_gods] = align_gname(A_CHAOTIC);
        num_gods++;
    }
    if (oi->unaligned_altar) {
        god_names[num_gods] = "Moloch";
        num_gods++;
    }
    for (i = 0; i < num_gods; i++) {
        buf = msgcat(buf, god_names[i]);
        if (i < num_gods - 1 && num_gods > 2)
            buf = msgcat(buf, ",");
        buf = msgcat(buf, " ");
        if (i == num_gods - 2)
            buf = msgcat(buf, "and ");
    }
    return buf;
}

static const char *
overview_print_info(const struct overview_info *oi)
{
    int i = 0;

    const char *buf = "";

    if (oi->shopcount > 1)
        ADDNTOBUF("shop", oi->shopcount);
    else if (oi->shopcount == 1)
        buf = msgcat_many(buf, COMMA, shopnames[oi->shoptype], NULL);

    ADDNTOBUF("high altar", oi->high_altars);

    /* Temples + non-temple altars get munged into just "altars" */
    if (!oi->temples || oi->temples != oi->altars)
        ADDNTOBUF("altar", oi->altars);
    else
        ADDNTOBUF("temple", oi->temples);

    if (oi->altars) {
        buf = msgcat(buf, " to ");
        buf = msgcat(buf, overview_print_gods(oi));
    }

    ADDNTOBUF("fountain", oi->fountains);
    ADDNTOBUF("sink", oi->sinks);
    ADDNTOBUF("throne", oi->thrones);
    ADDNTOBUF("tree", oi->trees);

    return buf;
}


static const char *
overview_print_branch(const struct overview_info *oi)
{
    const char *buf;

    if (oi->portal) {
        if (oi->portal_dst_known) {
            buf = msgprintf("      portal to %s",
                            dungeons[oi->portal_dst.dnum].dname);
        } else {
            buf = "      a magic portal";
        }
    } else if (oi->branch) {
        if (oi->branch_dst_known) {
            buf = msgprintf("      stairs to %s",
                            dungeons[oi->branch_dst.dnum].dname);
        } else {
            buf = "      a long staircase";
        }
    } else {
        impossible("branch connected badly?");
        buf = "      a connection";
    }

    return buf;
}


/* print a dungeon overview */
int
dooverview(const struct nh_cmd_arg *arg)
{
    struct overview_info oinfo;
    struct nh_menulist menu;
    int i, n, x, y, dnum;
    const int *selected;
    struct level *lev;
    const char *buf;

    (void) arg;

    init_menulist(&menu);

    if (!program_state.gameover) {
        add_menutext(&menu, "Select a level to view it");
        add_menutext(&menu, "");
    }

    dnum = -1;
    for (i = 0; i <= maxledgerno(); i++) {
        if (!levels[i]->generated)
            continue;
        overview_scan(levels[i], &oinfo);

        if (levels[i]->z.dnum != dnum) {
            if (i > 0)
                add_menutext(&menu, "");
            buf = overview_print_dun(levels[i]);
            add_menuheading(&menu, buf);
            dnum = levels[i]->z.dnum;
        }

        /* "Level 3 (my level name)" */
        buf = overview_print_lev(levels[i]);
        add_menuitem(&menu, i + 1, buf, 0, FALSE);

        if (!overview_is_interesting(levels[i], &oinfo))
            continue;

        /* "some fountains, an altar" */
        buf = overview_print_info(&oinfo);
        if (*buf)
            add_menutext(&menu, buf);

        /* "Stairs to the Gnomish Mines" */
        if (oinfo.branch || oinfo.portal) {
            buf = overview_print_branch(&oinfo);
            add_menutext(&menu, buf);
        }
    }

    n = display_menu(&menu, "Dungeon overview:", PICK_ONE,
                     PLHINT_ANYWHERE, &selected);
    if (n <= 0)
        return 0;

    /* remote viewing */
    lev = levels[selected[0] - 1];
    if (level == lev)
        return 0;

    /* set the display buffer from the remembered */
    for (y = 0; y < ROWNO; y++)
        for (x = 0; x < COLNO; x++)
            dbuf_set_memory(lev, x, y);

    buf = overview_print_lev(lev);
    pline("Now viewing %s%s.  Press any key to return.",
          lev == sp_lev(sl_astral) ? "the " : "", buf);
    notify_levelchange(lev);
    flush_screen_nopos();
    win_pause_output(P_MAP);
    notify_levelchange(level);
    doredraw();

    return 0;
}

/*dungeon.c*/

