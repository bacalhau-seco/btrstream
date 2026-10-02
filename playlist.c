#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "playlist.h"
#include "config.h"

static int get_artist(const char *path, char *artist, size_t size)
{
    const char *start = strstr(path, "/Music/");
    const char *end;
    size_t length;

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

static int artist_played_recently(struct playlist *playlist, const char *track)
{
    char artist[256];
    char other[256];
    size_t window = artistrepeatwindow;
    size_t start;

    if (!get_artist(track, artist, sizeof(artist)))
        return 0;

    start = playlist->history_count > window ? playlist->history_count - window : 0;

    for (size_t i = start; i < playlist->history_count; i++) {
        if (get_artist(playlist->history[i], other, sizeof(other)) && strcmp(artist, other) == 0)
            return 1;
    }

    return 0;
}

static int track_played_recently(struct playlist *playlist, const char *track, size_t track_count)
{
    size_t window = track_count * repeatpercent / 100;
    size_t start;

    if (window > playlist->history_count)
        window = playlist->history_count;

    start = playlist->history_count - window;

    for (size_t i = start; i < playlist->history_count; i++) {
        if (strcmp(track, playlist->history[i]) == 0)
            return 1;
    }

    return 0;
}

static int add_to_history(struct playlist *playlist, const char *track)
{
    char *copy = strdup(track);

    if (!copy)
        return 0;

    if (playlist->history_count == MAX_HISTORY) {
        free(playlist->history[0]);
        memmove(playlist->history, playlist->history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        playlist->history_count--;
    }

    playlist->history[playlist->history_count++] = copy;

    return 1;
}

static void free_tracks(char **tracks, size_t count)
{
    for (size_t i = 0; i < count; i++)
        free(tracks[i]);

    free(tracks);
}

int playlist_open(struct playlist *playlist, const char *path, unsigned int seed)
{
    memset(playlist, 0, sizeof(*playlist));
    playlist->seed = seed;
    playlist->file = fopen(path, "r");

    return playlist->file != NULL;
}

void playlist_close(struct playlist *playlist)
{
    for (size_t i = 0; i < playlist->history_count; i++)
        free(playlist->history[i]);

    if (playlist->file)
        fclose(playlist->file);
}

int playlist_next(struct playlist *playlist, char *track, size_t size)
{
    char **tracks = NULL;
    size_t *candidates = NULL;
    size_t track_count = 0;
    size_t capacity = 0;
    size_t candidate_count = 0;
    size_t chosen;
    char line[4096];
    int ok = 0;

    rewind(playlist->file);

    while (fgets(line, sizeof(line), playlist->file)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '#' || line[0] == '\0')
            continue;

        if (track_count == capacity) {
            size_t new_capacity = capacity ? capacity * 2 : 128;
            char **bigger = realloc(tracks, new_capacity * sizeof(char *));

            if (!bigger)
                goto done;

            tracks = bigger;
            capacity = new_capacity;
        }

        tracks[track_count] = strdup(line);

        if (!tracks[track_count])
            goto done;

        track_count++;
    }

    if (track_count == 0)
        goto done;

    candidates = malloc(track_count * sizeof(size_t));

    if (!candidates)
        goto done;

    for (size_t i = 0; i < track_count; i++) {
        if (!track_played_recently(playlist, tracks[i], track_count) &&
            !artist_played_recently(playlist, tracks[i]))
            candidates[candidate_count++] = i;
    }

    if (candidate_count == 0) {
        for (size_t i = 0; i < track_count; i++) {
            if (!track_played_recently(playlist, tracks[i], track_count))
                candidates[candidate_count++] = i;
        }
    }

    if (candidate_count == 0) {
        for (size_t i = 0; i < track_count; i++)
            candidates[candidate_count++] = i;
    }

    chosen = candidates[rand_r(&playlist->seed) % candidate_count];

    if (!add_to_history(playlist, tracks[chosen]))
        goto done;

    snprintf(track, size, "%s", tracks[chosen]);
    ok = 1;

done:
    free(candidates);
    free_tracks(tracks, track_count);

    return ok;
}
