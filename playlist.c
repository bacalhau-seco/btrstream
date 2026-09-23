#include <stdio.h>
#include <string.h>

#include "playlist.h"

FILE *playlist_open(const char *path)
{
    return fopen(path, "r");
}

int playlist_next(FILE *playlist, char *line, size_t size)
{
    while (fgets(line, size, playlist)) {
        if (line[0] == '#' || line[0] == '\n')
            continue;

        line[strcspn(line, "\r\n")] = '\0';
        return 1;
    }

    return 0;
}

void playlist_close(FILE *playlist)
{
    fclose(playlist);
}
