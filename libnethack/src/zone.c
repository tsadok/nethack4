/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-12-30 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "zone.h"

extern inline int zn_byte_index(struct coord loc);
extern inline int zn_bit_index(struct coord loc);
extern inline struct coord zn_from_indices(int byte, int bit);
extern inline struct zone zn_empty(void);
extern inline struct zone zn_union(struct zone zn1, struct zone zn2);
extern inline struct zone zn_intersect(struct zone zn1, struct zone zn2);
extern inline struct zone zn_subtract(struct zone zn1, struct zone zn2);
extern inline struct zone zn_sym_diff(struct zone zn1, struct zone zn2);
extern inline boolean zn_contains(struct zone zn, struct coord loc);
extern inline boolean zn_is_empty(struct zone zn);
extern inline struct zone zn_add(struct zone zn, struct coord loc);
extern inline struct zone zn_loc(struct coord loc);
extern inline struct zone zn_del(struct zone zn, struct coord loc);
extern inline struct zone zn_add_pred(struct zone zn, zn_pred_fn pred,
                                      void *arg);
extern inline struct zone zn_del_pred(struct zone zn, zn_pred_fn pred,
                                      void *arg);
extern inline struct zone zn_except_pred(struct zone zn, zn_pred_fn pred,
                                         void *arg);
extern inline struct zone zn_pred(zn_pred_fn pred, void *arg);
extern inline struct zone zn_add_rect(struct zone zn, struct rect r);
extern inline struct zone zn_rect(struct rect r);
extern inline struct zone zn_whole(void);
extern inline struct zone zn_del_rect(struct zone zn, struct rect r);
extern inline struct zone zn_except_rect(struct zone zn, struct rect r);
extern inline struct zone zn_shift(struct zone zn, struct coord delta);
extern inline uchar count_bits(uchar bits);
extern inline uchar nth_set_bit(uchar bits, uchar n);

struct zone
zn_open_circle(struct coord center, int radius)
{
    struct zone zn;

    int dx, dy;
    for (dx = -radius + 1; dx <= radius - 1; ++dx) {
        for (dy = -radius + 1; dy <= radius - 1; ++dy) {
            int x = center.x + dx, y = center.y + dy;
            if (isok(x, y) && dx * dx + dy * dy < radius * radius)
                zn_add(zn, (struct coord){x, y});
        }
    }

    return zn;
}

struct coord
zn_rand(struct zone zn)
{
    int i, count = 0;
    for (i = 0; i < zone_length; ++i)
        count += count_bits(zn.dat[i]);

    if (count == 0)
        return (struct coord){COLNO, ROWNO};

    int n = rn2(count);
    count = 0;
    for (i = 0; i < zone_length; ++i) {
        count += count_bits(zn.dat[i]);
        if (count > n) {
            int bit = nth_set_bit(zn.dat[i], count - n - 1);
            return zn_from_indices(i, bit);
        }
    }

    impossible ("Did not pick a valid coordinate from zone?");
    return (struct coord){COLNO, ROWNO};
}

struct coord
zn_rand_near(struct zone zn, struct coord center)
{
    if (zn_contains(zn, center))
        return center;

    int r, bound = max(max(center.x, center.y),
                       max(COLNO - center.x, ROWNO - center.y));
    for (r = 1; r <= bound; ++r) {
        struct zone circle = zn_intersect(zn, zn_open_circle(center, r));
        if (!zn_is_empty(circle))
            return zn_rand(circle);
    }

    return (struct coord){COLNO, ROWNO};
}
