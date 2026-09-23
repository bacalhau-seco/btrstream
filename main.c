#include <stdio.h>
#include <unistd.h>

#include <shout/shout.h>
#include "config.h"
#include "playlist.h"

int main()
{
    char line[4096];
    shout_t *shouts[sizeof(mounts) / sizeof(mounts[0])];

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
    }
    for (size_t i = 0; i < sizeof(playlists) / sizeof(playlists[0]); i++) {
        FILE *playlist;

        if (!(playlist = playlist_open(playlists[i]))) {
            printf("Could not open playlist: %s\n", playlists[i]);
            return 1;
        }

        while (playlist_next(playlist, line, sizeof(line)))
            printf("%s\n", line);

        playlist_close(playlist);
    }
    while (1)
        pause();

    return 0;
}
