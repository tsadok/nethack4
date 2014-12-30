/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-12-30 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ZONE_H
# define ZONE_H
# include <limits.h>

# include "global.h"
# include "coord.h"
# include "rect.h"
/* for min and max */
# include "hack.h"

#pragma clang diagnostic ignored "-Wstatic-in-inline"

/* An anonymous enum is used here to create a constant which can be used in
 * constant expressions, because C's definition of an integer constant
 * expression is overly restrictive and does not allow static const variables to
 * be used. */
enum {
    /* A zone is long enough to store ROWNO * COLNO bits. */
    zone_length = ((ROWNO * COLNO) + (CHAR_BIT - 1)) / CHAR_BIT
};

/* A predicate for use in zone functions: is a square good? */
typedef boolean (*zn_pred_fn) (struct coord loc, void *arg);

/* A zone is a lightweight structure for storing the concept of "some subset" of
 * the map. The internal implementation is as a bit array, and functions can be
 * used to do manipulations, rather than 
 */
struct zone {
    uchar dat[zone_length];
};

inline int
zn_byte_index(struct coord loc)
{
    return (loc.x + loc.y * COLNO) / CHAR_BIT;
}

inline int
zn_bit_index(struct coord loc)
{
    return (loc.x + loc.y * COLNO) % CHAR_BIT;
}

inline struct coord
zn_from_indices(int byte, int bit)
{
    int idx = byte * CHAR_BIT + bit;
    return (struct coord){idx % COLNO, idx / COLNO};
}

/**** Basic set-theoretic constructions ****/

inline struct zone
zn_empty(void)
{
    return (struct zone){{0}};
}

inline struct zone
zn_union(struct zone zn1, struct zone zn2)
{
    struct zone zn;
    int i;
    for (i = 0; i < zone_length; ++i)
        zn.dat[i] = zn1.dat[i] | zn2.dat[i];
    return zn;
}

inline struct zone
zn_intersect(struct zone zn1, struct zone zn2)
{
    struct zone zn;
    int i;
    for (i = 0; i < zone_length; ++i)
        zn.dat[i] = zn1.dat[i] & zn2.dat[i];
    return zn;
}

inline struct zone
zn_subtract(struct zone zn1, struct zone zn2)
{
    struct zone zn;
    int i;
    for (i = 0; i < zone_length; ++i)
        zn.dat[i] = zn1.dat[i] & ~zn2.dat[i];
    return zn;
}

inline struct zone
zn_sym_diff(struct zone zn1, struct zone zn2)
{
    struct zone zn;
    int i;
    for (i = 0; i < zone_length; ++i)
        zn.dat[i] = zn1.dat[i] ^ zn2.dat[i];
    return zn;
}

/**** Tests ****/

inline boolean
zn_contains(struct zone zn, struct coord loc)
{
    return zn.dat[zn_byte_index(loc)] & (1 << zn_bit_index(loc));
}

inline boolean
zn_is_empty(struct zone zn)
{
    int i;
    for (i = 0; i < zone_length - 1; ++i)
        if (zn.dat[i])
            return FALSE;
    /* The last byte may have some high bits we don't want to check */
    if (zn.dat[i] << ((ROWNO * COLNO) % CHAR_BIT))
        return FALSE;
    return TRUE;
}

/**** Constructions with single locations ****/

inline struct zone
zn_add(struct zone zn, struct coord loc)
{
    zn.dat[zn_byte_index(loc)] |= 1 << zn_bit_index(loc);
    return zn;
}

inline struct zone
zn_loc(struct coord loc) {
    return zn_add(zn_empty(), loc);
}

inline struct zone
zn_del(struct zone zn, struct coord loc)
{
    zn.dat[zn_byte_index(loc)] &= ~(1 << zn_bit_index(loc));
    return zn;
}

/**** Constructions with callbacks ****/

inline struct zone
zn_add_pred(struct zone zn, zn_pred_fn pred, void *arg)
{
    int x, y;
    for (y = 0; y < ROWNO; ++y) {
        for (x = 0; x < COLNO; ++x) {
            struct coord c = {x, y};
            if (!zn_contains(zn, c) && pred(c, arg))
                zn = zn_add(zn, c);
        }
    }
    return zn;
}

inline struct zone
zn_del_pred(struct zone zn, zn_pred_fn pred, void *arg)
{
    int x, y;
    for (y = 0; y < ROWNO; ++y) {
        for (x = 0; x < COLNO; ++x) {
            struct coord c = {x, y};
            if (zn_contains(zn, c) && pred(c, arg))
                zn = zn_del(zn, c);
        }
    }
    return zn;
}

inline struct zone
zn_except_pred(struct zone zn, zn_pred_fn pred, void *arg)
{
    int x, y;
    for (y = 0; y < ROWNO; ++y) {
        for (x = 0; x < COLNO; ++x) {
            struct coord c = {x, y};
            if (zn_contains(zn, c) && !pred(c, arg))
                zn = zn_del(zn, c);
        }
    }
    return zn;
}

inline struct zone
zn_pred(zn_pred_fn pred, void *arg)
{
    return zn_add_pred(zn_empty(), pred, arg);
}

/**** Constructions with rectangles ****/

inline struct zone
zn_add_rect(struct zone zn, struct rect r)
{
    int y, x;
    /* This loop does the data assignment one byte at a time (not counting
     * overlap at the end of a row and the start of the next for very big
     * rectangles).
     */
    for (y = r.ly; y <= r.hy; ++y) {
        x = zn_byte_index((struct coord){r.lx, y});
        if (x == zn_byte_index((struct coord){r.hx, y})) {
            /* Set bits that are equal to or higer than the low bit index *and*
             * equal to or less than the high bit index. */
            zn.dat[x] |= ~((1 << zn_bit_index((struct coord){r.lx, y})) - 1) &
                         ((1 << (zn_bit_index((struct coord){r.hx, y}) + 1)) - 1);
        } else {
            /* Set the bits that are equal to or higher than the bit index. */
            zn.dat[x] |= ~((1 << zn_bit_index((struct coord){r.lx, y})) - 1);
            /* Set whole bytes */
            for (; x < zn_byte_index((struct coord){r.hx, y}); ++x)
                zn.dat[x] = ~(uchar)0;
            /* Set the bits that are equal to or lesser than the bit index. */
            zn.dat[x] |= (1 << (zn_bit_index((struct coord){r.hx, y}) + 1)) - 1;
        }
    }

    return zn;
}

inline struct zone
zn_rect(struct rect r)
{
    return zn_add_rect(zn_empty(), r);
}

inline struct zone
zn_whole(void)
{
    return zn_rect((struct rect){0, 0, COLNO - 1, ROWNO - 1});
}

inline struct zone
zn_del_rect(struct zone zn, struct rect r)
{
    return zn_subtract(zn, zn_rect(r));
}

inline struct zone
zn_except_rect(struct zone zn, struct rect r)
{
    return zn_intersect(zn, zn_rect(r));
}

/* This one shifts an entire region by the amount specified.
 *
 * Note that delta may contain negative values, in which case a negative shift
 * is performed.
 */
inline struct zone
zn_shift(struct zone zn, struct coord delta)
{
    /* When performing shifts, we must truncate to the range possible, or else
     * we may wrap coordinates in the zone to the other side, outside the new
     * zone. */
    struct rect rect_bounds = {
        max(delta.x, 0), max(delta.y, 0),
        COLNO + min(delta.x, 0), ROWNO + min(delta.y, 0),
    };
    struct zone znr = zn_empty();
    int byte_shift = zn_byte_index(delta), bit_shift = zn_bit_index(delta);
    int i;

    /* Now we shift the entire array. */
    if (bit_shift == 0) {
        /* If we're lucky and we get a total number of bits divisible by 8, we
         * can just copy the memory over directly.
         */
        if (byte_shift >= 0)
            memcpy(znr.dat + byte_shift, zn.dat, zone_length - byte_shift);
        else
            memcpy(znr.dat, zn.dat + byte_shift, zone_length + byte_shift);
    } else if (bit_shift > 0) {
        znr.dat[byte_shift] = zn.dat[0] << bit_shift;
        for (i = 1; i < zone_length - byte_shift; ++i)
            znr.dat[i + byte_shift] = (zn.dat[i] << bit_shift) |
                                      (zn.dat[i - 1] >> (8 - bit_shift));
    } else {
        byte_shift -= byte_shift;
        bit_shift -= bit_shift;
        znr.dat[0] = zn.dat[byte_shift] >> bit_shift;
        for (i = 1; i < zone_length - byte_shift; ++i)
            znr.dat[i] = (zn.dat[i + byte_shift] >> bit_shift) |
                         (zn.dat[i + byte_shift - 1] << (8 - bit_shift));
    }

    return zn_except_rect(znr, rect_bounds);
}

/* FIXME: Move this somewhere better (hack.h?) */
inline uchar
count_bits(uchar bits)
{
    bits -= (bits >> 1) & 0x55;
    bits = (bits & 0x33) + ((bits >> 2) & 0x33);
    bits = (bits & 0x0F) + (bits >> 4);
    return bits;
}

inline uchar
nth_set_bit(uchar bits, uchar n)
{
    int i;
    for (i = 0; n || !(bits & 1); ++i, bits >>= 1)
        if (bits & 1)
            --n;
    return i;
}


/* Produces a zone featuring the open circle of radius r, centered at c. This
 * includes exactly the points of Euclidean distance strictly less than r from
 * c. */
extern struct zone zn_open_circle(struct coord center, int radius);

/* Returns a random spot in the zone, or {COLNO, ROWNO} if that isn't possible. */
extern struct coord zn_rand(struct zone zn);

/* Return a random location in zn that is as close to c as possible. For this
 * purpose, "as close as possible" is not truly Euclidean distance; rather, for
 * each n, all points in the open circle of radius n are considered equidistant.
 * This definition was chosen to get a nice balance between picking a spot
 * that's actually close and not showing too much preference for specific
 * squares. In particular, if c is not available, it chooses equally from all of
 * the points adjacent to c.  */
extern struct coord zn_rand_near(struct zone zn, struct coord center);

#endif /* ZONE_H */
