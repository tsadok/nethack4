/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-12-07 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ENDGAME_H
#define ENDGAME_H

#include "rm.h"

void gen_waterlevel(struct level *lev);
void gen_firelevel(struct level *lev);

extern struct level_manager waterlevel_manager;
extern struct level_manager firelevel_manager;

#endif
