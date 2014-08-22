/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-10-17 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*      Common include file for save and restore routines */

#ifndef LEV_H
# define LEV_H

# include "global.h"

/* used in light.c */
typedef struct ls_t {
    struct ls_t *next;
    xchar x, y; /* source's position */
    short range;        /* source's current range */
    short flags;
    short type; /* type of light source */
    void *id;   /* source's identifier */
} light_source;

extern int n_dgns;

#endif /* LEV_H */

