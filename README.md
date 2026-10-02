# btrStream
I hate ezstream so im making this for my web radio.
The goal is a streaming software to be used with icecast for webradio.

## Features
These are the defining features:
- 1 process for multiple playlists
- autoreload after playing all songs in the playlist
- no retarded config files
- extremly easy to use configuration will be done at compile time
- don't repeat artists in a row
- dont repeat the same song if it played X songs ago
- reconnects to icecast by itself if the connection drops

This will keep webradio fresh at the cost of nothing. Extremly ez to use, configure and manage. You're welcome!

## Development description
For the development of btrstream it will be used libshout to connect and stream the content to the icecast server.

## Dependencies
- libshout

## How to use
btrstream assumes you follow this:
`/mnt/storage/radio/Music/Death/Human/04 - Secret Face.ogg`

Meaning the artist name always comes after `Music`

Edit `config.h` (ip, port, password, playlists and mounts), then:

```
make
./btrstream
```
