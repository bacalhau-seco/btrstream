#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "playlist.h"
#include "config.h"

#define MAX_TRACKS 1024

static int get_artist(const char *path, char *artist, size_t size)
{
    const char *start;
    const char *end;
    size_t length;

    start = strstr(path, "/Music/");

    if (!start)
        return 0;

    start += strlen("/Music/");
    end = strchr(start, '/');

    if (!end)
        return 0;

    length = end - start;

    if (length >= size)
        return 0;

    memcpy(artist, start, length);
    artist[length] = '\0';

    return 1;
}

static int artist_recent(const char *track, char **history, size_t history_count)
{
    char artist[256];
    char previous_artist[256];
    size_t start;

    if (!get_artist(track, artist, sizeof(artist)))
        return 0;

    if (history_count < artistrepeatwindow)
        start = 0;
    else
        start = history_count - artistrepeatwindow;

    for (size_t i = start; i < history_count; i++) {
        if (!get_artist(history[i], previous_artist, sizeof(previous_artist)))
            continue;

        if (strcmp(artist, previous_artist) == 0)
            return 1;
    }

    return 0;
}

static int track_recent(const char *track, char **history, size_t history_count, size_t playlist_size)
{
    size_t window;
    size_t start;

    window = playlist_size * repeatpercent / 100;

    if (window > history_count)
        window = history_count;

    start = history_count - window;

    for (size_t i = start; i < history_count; i++) {
        if (strcmp(track, history[i]) == 0)
            return 1;
    }

    return 0;
}

static void free_tracks(char **tracks, size_t count)
{
    for (size_t i = 0; i < count; i++)
        free(tracks[i]);
}

int playlist_next(FILE *playlist, char *line, size_t size)
{
    static char *history[MAX_TRACKS];
    static size_t history_count;

    char *tracks[MAX_TRACKS];
    size_t candidates[MAX_TRACKS];
    char buffer[4096];
    size_t track_count = 0;
    size_t candidate_count = 0;
    size_t index;

    rewind(playlist);

    while (fgets(buffer, sizeof(buffer), playlist)) {
        if (buffer[0] == '#' || buffer[0] == '\n')
            continue;

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (track_count >= MAX_TRACKS)
            break;

        tracks[track_count] = malloc(strlen(buffer) + 1);

        if (!tracks[track_count]) {
            free_tracks(tracks, track_count);
            return 0;
        }

        strcpy(tracks[track_count], buffer);
        track_count++;
    }

    if (track_count == 0) {
        free_tracks(tracks, track_count);
        return 0;
    }

    for (size_t i = 0; i < track_count; i++) {
        if (track_recent(tracks[i], history, history_count, track_count))
            continue;

        if (artist_recent(tracks[i], history, history_count))
            continue;

        candidates[candidate_count++] = i;
    }

    if (candidate_count == 0) {
        for (size_t i = 0; i < track_count; i++)
            candidates[candidate_count++] = i;
    }

    index = candidates[rand() % candidate_count];

    if (history_count == MAX_TRACKS) {
        free(history[0]);

        memmove(history, history + 1, (MAX_TRACKS - 1) * sizeof(*history));
        history_count--;
    }

    history[history_count] = malloc(strlen(tracks[index]) + 1);

    if (!history[history_count]) {
        free_tracks(tracks, track_count);
        return 0;
    }

    strcpy(history[history_count], tracks[index]);
    history_count++;

    snprintf(line, size, "%s", tracks[index]);

    free_tracks(tracks, track_count);

    return 1;
}

FILE *playlist_open(const char *path)
{
    return fopen(path, "r");
}

void playlist_close(FILE *playlist)
{
    fclose(playlist);
}
