#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stddef.h>
#include <stdio.h>

FILE *playlist_open(const char *path);
int playlist_next(FILE *playlist, char *line, size_t size);
void playlist_close(FILE *playlist);

#endif
