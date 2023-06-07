# romjudge
A utility to judge an N64 ROM for correctness, for Windows and Linux.

Windows users should download the pre-built EXE: [romjudge.exe](https://github.com/jkbenaim/romjudge/raw/master/romjudge.exe)

Usage:
```
usage: romjudge [-c ipl] [-r region] -f romfile
  -c will force an IPL and rewrite checksums.
  -r will overwrite the region letter.
```

`romjudge` will indicate if an N64 ROM has:
- the correct **byte order**,
- typical **PI timings**,
- a sane **file size**,
- the correct **CRCs**, and
- a recognized **IPL3/bootcode**.

Written in C. X11 license. `sha1.c` and `sha1.h` are public domain.
