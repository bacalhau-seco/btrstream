#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <shout/shout.h>
#include "config.h"
#include "playlist.h"

struct stream {
    shout_t *shout;
    const char *playlist_name;
};

void *stream_play(void *arg)
{
    struct stream *stream = arg;
    char line[4096];
    char buffer[8192];
    FILE *playlist = playlist_open(stream->playlist_name);

    if (!playlist) {
        printf("Could not open playlist: %s\n", stream->playlist_name);
        return NULL;
    }

    while (playlist_next(playlist, line, sizeof(line))) {
        FILE *file = fopen(line, "rb");
        size_t bytes;

        if (!file) {
            printf("Could not open track: %s\n", line);
            continue;
        }

        printf("Playing %s\n", line);

        while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (shout_send(stream->shout, (unsigned char *)buffer, bytes) != SHOUTERR_SUCCESS) {
                printf("Error sending %s: %s\n", line, shout_get_error(stream->shout));
                fclose(file);
                playlist_close(playlist);
                return NULL;
            }

            shout_sync(stream->shout);
        }

        fclose(file);
    }

    playlist_close(playlist);
    return NULL;
}


int main()
{
    shout_t *shouts[sizeof(mounts) / sizeof(mounts[0])];
    pthread_t threads[sizeof(mounts) / sizeof(mounts[0])]; // declare thread array
    struct stream streams[sizeof(mounts) / sizeof(mounts[0])]; 

    srand(time(NULL));
    shout_init();

    for (size_t i = 0; i < sizeof(mounts) / sizeof(mounts[0]); i++) {
        if (!(shouts[i] = shout_new())) {
            printf("Could not allocate shout_t\n");
            return 1;
        }

        if (shout_set_host(shouts[i], ip) != SHOUTERR_SUCCESS) {
            printf("Error setting hostname: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_protocol(shouts[i], SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
            printf("Error setting protocol: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_port(shouts[i], port) != SHOUTERR_SUCCESS) {
            printf("Error setting port: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_password(shouts[i], password) != SHOUTERR_SUCCESS) {
            printf("Error setting password: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_mount(shouts[i], mounts[i]) != SHOUTERR_SUCCESS) {
            printf("Error setting mount: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_user(shouts[i], username) != SHOUTERR_SUCCESS) {
            printf("Error setting user: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_set_content_format(shouts[i], SHOUT_FORMAT_OGG, SHOUT_USAGE_UNKNOWN, NULL) != SHOUTERR_SUCCESS) {
            printf("Error setting format: %s\n", shout_get_error(shouts[i]));
            return 1;
        }

        if (shout_open(shouts[i]) != SHOUTERR_SUCCESS) {
            printf("Error connecting to %s: %s\n", mounts[i], shout_get_error(shouts[i]));
            return 1;
        }

        printf("Mounted %s\n", mounts[i]);

        streams[i].shout = shouts[i];
        streams[i].playlist_name = playlists[i];

        if (pthread_create(&threads[i], NULL, stream_play, &streams[i]) != 0) { // creates the thread for every stream and plays the stream
            printf("Could not create thread\n");
            return 1; // closes if the thread fails to create
        }
    }

    for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++)
        pthread_join(threads[i], NULL);

    for (size_t i = 0; i < sizeof(shouts) / sizeof(shouts[0]); i++) {
        shout_close(shouts[i]);
        shout_free(shouts[i]);
    }

    shout_shutdown();

    return 0;
}
