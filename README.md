# romjudge
A utility to judge an N64 ROM for correctness.

Usage:
```
Usage: romjudge [-f] file
  -f will fix a broken ROM.
```

`romjudge` will indicate if an N64 ROM has:
- the correct **byte order**,
- typical **PI timings**,
- a sane **file size**,
- the correct **CRCs**, and
- a recognized **IPL3/bootcode**.

Written in C11. X11 license. `sha1.c` and `sha1.h` are public domain.
