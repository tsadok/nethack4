/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* Last modified by Sean Hunt, 2014-08-25 */
/* Copyright (c) Sean Hunt, 2014. */
/* NetHack may be freely redistributed.  See license for details. */

#include "levelgen_dsl.h"

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
lg_fill_map(struct level *lev, char c, int line, const char * file) {
    int val = lg_what_map_char(c);

    if (val == INVALID_TYPE) {
        impossible("Invalid fill character at line %d of %s", line, file);
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
lg_new_map (struct coord size, const char *text, int line, const char * file,
            struct maparea **chain) {
    char *buf = malloc(size.x * size.y + 1);

    int len = strlen(text);
    if (len < size.x * size.y)
        impossible("Map diagram is too small at line %d of %s", line, file);

    memset(buf, ' ', size.x * size.y);
    buf[size.x * size.y] = '\0';
    memcpy(buf, text, len);

    char *c;
    for (c = buf; c < buf + size.x * size.y; ++buf) {
        char typ = lg_what_map_char(*c);
        if (typ == INVALID_TYPE) {
            impossible("Invalid fill character 'c' "
                        "in map at line %d of %s", line, file);
            *c = ' ';
        } else
            *c = typ;
    }

    struct maparea *map = malloc(sizeof(struct maparea));
    map->locs = buf;
    map->area.tlx = map->area.tly = 0;
    map->area.brx = size.x - 1;
    map->area.bry = size.y - 1;
    map->nextmap = *chain;
    *chain = map;
    return map;
}

void
lg_place_at(struct level *lev, struct maparea *map, struct coord loc) {
    /* Reset the area position so that it can be used as a baseline */
    map->area.brx -= map->area.tlx;
    map->area.bry -= map->area.tly;
    map->area.tlx = loc.x;
    map->area.brx = loc.x;
    map->area.tly += loc.y;
    map->area.bry += loc.y;

    if (map->area.tlx < 0 || map->area.tly < 0 ||
        map->area.brx >= COLNO || map->area.bry >= ROWNO) {
        impossible("Map area does not fit on map! Not placing!");
        return;
    }

    int x, y;
    for (x = map->area.tlx; x <= map->area.brx; ++x) {
        for (y = map->area.tly; y <= map->area.bry; ++y) {
            int ix = (y - map->area.tly) * (map->area.brx - map->area.tlx + 1) +
                     (x - map->area.brx);
            lev->locations[x][y].typ = map->locs[ix];
            lev->locations[x][y].flags = 0;
            lev->locations[x][y].horizontal = 0;
        }
    }
}
