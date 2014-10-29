/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-29 */
/* Copyright (c) Sean Hunt, 2014.                                 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MAKEMON_H
# define MAKEMON_H

# include "global.h"
# include "rm.h"

/* The callback type for gen_monster_type.
 * It is passed the level, the monster index, and its callback data. The
 * callback should give the probability of that type of monster being generated,
 * as a relative weight. If the probability is 0, the monster will not be
 * generated. 
 */
typedef uchar (*gen_prob_callback)(const struct level *lev, short mndx,
                                   void *dat);

struct gen_prob_wrapper {
    gen_prob_callback callback;
    void *dat;
};

/* Pick a random monster, using the specified callback to get its generation
 * probability.
 *
 * Regardless of the flags passed, an extinct unique monster will never, ever be
 * selected, nor will a genocided monster or a placeholder. Otherwise, the
 * callback is responsible for checking for extinction, unique monsters, and
 * non-generated monsters.
 */
int pick_monster(const struct level *lev, gen_prob_callback callback,
                 void *dat);

/* Pick a random monster of a given class, using the specified callback to get
 * its generation probability. See pick_monster() for details. If the class is
 * 0, it behaves identically to pick_monster(), for convenience.
 */
int pick_monster_class(const struct level *lev, char cls,
                       gen_prob_callback callback, void *dat);

/* Get a shift from 0 to 5 that should happen to a monster because of its
 * alignment relative to the dungeon aligment. The stronger the match, the
 * higher the value. */
uchar align_shift(const struct level *lev, short mndx);

/* Determine whether the monster is out-of-depth for a given level. If
 * consider_xlvl is TRUE, the player's experience level is taken into account.
 * Not that a monster which is too weak is also considered out of depth.
 */
boolean out_of_depth(const struct level *lev, short mndx, boolean consider_xlvl);

struct default_gen_flags {
    boolean consider_xlvl : 1;
    boolean ignore_extinct : 1;
    boolean ignore_nogen : 1;
    boolean ignore_uniq : 1;
};

/* Produce the default monster generation probability. The dat parameter should
 * be a pointer to a default_gen_flags that indicates whether or not we should be
 * taking the player's experience level into account (normally false during
 * level generation, and true otherwise), and whether to ignore
 * NOGEN/UNIQ/EXTINCT monsters.
 *
 * This takes into account the monster's level, alignment, and the hellishness
 * of the level, as well as possibly its NOGEN/UNIQ/EXTINCT status and/or the
 * experience level of the player.
 */
uchar default_gen_prob(const struct level *lev, short mndx, void *flags);

#endif /* MAKEMON_H */

