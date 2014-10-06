/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-07 */
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
extern inline struct coord zn_rand(struct zone zn);
