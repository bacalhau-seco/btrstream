#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "playlist.h"

FILE *playlist_open(const char *path)
{
    return fopen(path, "r");
}

int playlist_next(FILE *playlist, char *line, size_t size)
{
    char *tracks[1024];
    char buffer[4096];
    size_t count = 0;
    size_t index;
    index = rand() % count;

    rewind(playlist);

    while (fgets(buffer, sizeof(buffer), playlist)) {
        if (buffer[0] == '#' || buffer[0] == '\n')
            continue;

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (count >= 1024)
            break;

        tracks[count] = malloc(strlen(buffer) + 1);

        if (!tracks[count])
            return 0;

        strcpy(tracks[count], buffer);
        count++;
    }

    if (count == 0)
        return 0;

    index = rand() % count;
    snprintf(line, size, "%s", tracks[index]);

    for (size_t i = 0; i < count; i++)
        free(tracks[i]);

    return 1;
}

void playlist_close(FILE *playlist)
{
    fclose(playlist);
}
