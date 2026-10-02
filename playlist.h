#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stddef.h>
#include <stdio.h>

#define MAX_HISTORY 1024

struct playlist {
    FILE *file;
    char *history[MAX_HISTORY];
    size_t history_count;
    unsigned int seed;
};

int playlist_open(struct playlist *playlist, const char *path, unsigned int seed);
int playlist_next(struct playlist *playlist, char *track, size_t size);
void playlist_close(struct playlist *playlist);

#endif
