# spartadaily

- reader requires `<ncurses.h>` (from ncurses-dev, libncurses-dev or similar)

- `make` in src to build
- `make install` in src to put executables in bin

## server
in-memory: `$ ./archive <port>`

disk-backed: 
 1) `$ mkdir dbdir`
 2) `$ ./archive <port> dbdir`

## reader
usage: `reader <hostname> <port>`

Located at the bottom of the screen are the options and status bars. Options bar
tells you what keys you can (meaningfully) press and status bar tells you what
you have pressed, or what input is required to proceed.

The options pane is not 100% accurate in suggesting keys, in which case the
pressed key will not do anything or result in a nop.

It is possible to use up/down arrow keys to scroll in the lists. However, it is
not possible to use the arrows key when reading or creating articles and groups.
Backspace support is also very limited.
