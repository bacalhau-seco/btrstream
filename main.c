#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <shout/shout.h>
#include "config.h"
#include "playlist.h"

#define STREAM_COUNT (sizeof(mounts) / sizeof(mounts[0]))
#define RETRY_SECONDS 5

struct stream {
    shout_t *shout;
    const char *playlist_name;
    unsigned int seed;
};

enum play_result {
    PLAYED,
    FILE_ERROR,
    SEND_ERROR
};

static volatile sig_atomic_t running = 1;

static void stop(int sig)
{
    (void)sig;
    running = 0;
}

static void wait_seconds(int seconds)
{
    while (seconds-- > 0 && running)
        sleep(1);
}

static enum play_result play_file(struct stream *stream, const char *path)
{
    unsigned char buffer[8192];
    size_t bytes;
    FILE *file = fopen(path, "rb");

    if (!file) {
        printf("Could not open track: %s\n", path);
        return FILE_ERROR;
    }

    printf("[%s] Playing %s\n", stream->playlist_name, path);

    while (running && (bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (shout_send(stream->shout, buffer, bytes) != SHOUTERR_SUCCESS) {
            printf("[%s] Error sending: %s\n", stream->playlist_name, shout_get_error(stream->shout));
            fclose(file);
            return SEND_ERROR;
        }

        shout_sync(stream->shout);
    }

    fclose(file);
    return PLAYED;
}

static void *stream_play(void *arg)
{
    struct stream *stream = arg;
    struct playlist playlist;
    char track[4096];
    int connected = 0;

    if (!playlist_open(&playlist, stream->playlist_name, stream->seed)) {
        printf("Could not open playlist: %s\n", stream->playlist_name);
        return NULL;
    }

    while (running) {
        if (!connected) {
            if (shout_open(stream->shout) != SHOUTERR_SUCCESS) {
                printf("[%s] Could not connect: %s\n", stream->playlist_name, shout_get_error(stream->shout));
                wait_seconds(RETRY_SECONDS);
                continue;
            }

            connected = 1;
            printf("[%s] Connected\n", stream->playlist_name);
        }

        if (!playlist_next(&playlist, track, sizeof(track))) {
            printf("[%s] Playlist is empty or unreadable\n", stream->playlist_name);
            wait_seconds(RETRY_SECONDS);
            continue;
        }

        switch (play_file(stream, track)) {
        case FILE_ERROR:
            wait_seconds(1);
            break;
        case SEND_ERROR:
            shout_close(stream->shout);
            connected = 0;
            wait_seconds(RETRY_SECONDS);
            break;
        case PLAYED:
            break;
        }
    }

    if (connected)
        shout_close(stream->shout);

    playlist_close(&playlist);
    return NULL;
}

static int stream_setup(struct stream *stream, size_t i)
{
    shout_t *shout = shout_new();

    if (!shout) {
        printf("Could not allocate shout_t\n");
        return 0;
    }

    if (shout_set_host(shout, ip) != SHOUTERR_SUCCESS ||
        shout_set_port(shout, port) != SHOUTERR_SUCCESS ||
        shout_set_user(shout, username) != SHOUTERR_SUCCESS ||
        shout_set_password(shout, password) != SHOUTERR_SUCCESS ||
        shout_set_mount(shout, mounts[i]) != SHOUTERR_SUCCESS ||
        shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS ||
        shout_set_meta(shout, SHOUT_META_NAME, playlists[i]) != SHOUTERR_SUCCESS ||
        shout_set_content_format(shout, SHOUT_FORMAT_OGG, SHOUT_USAGE_UNKNOWN, NULL) != SHOUTERR_SUCCESS) {
        printf("Error setting up %s: %s\n", mounts[i], shout_get_error(shout));
        shout_free(shout);
        return 0;
    }

    stream->shout = shout;
    stream->playlist_name = playlists[i];
    stream->seed = (unsigned int)time(NULL) + (unsigned int)i;

    return 1;
}

int main(void)
{
    struct stream streams[STREAM_COUNT] = {0};
    pthread_t threads[STREAM_COUNT];
    struct sigaction action = {.sa_handler = stop};
    size_t started = 0;
    int status = 0;

    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    signal(SIGPIPE, SIG_IGN);

    shout_init();

    for (size_t i = 0; i < STREAM_COUNT; i++) {
        if (!stream_setup(&streams[i], i)) {
            status = 1;
            goto cleanup;
        }
    }

    for (; started < STREAM_COUNT; started++) {
        if (pthread_create(&threads[started], NULL, stream_play, &streams[started]) != 0) {
            printf("Could not create thread\n");
            running = 0;
            status = 1;
            break;
        }
    }

    for (size_t i = 0; i < started; i++)
        pthread_join(threads[i], NULL);

cleanup:
    for (size_t i = 0; i < STREAM_COUNT; i++) {
        if (streams[i].shout)
            shout_free(streams[i].shout);
    }

    shout_shutdown();

    return status;
}
