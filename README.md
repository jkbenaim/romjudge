# romjudge
A utility to judge an N64 ROM for correctness.

```romjudge``` will indicate if an N64 ROM has:
- the correct **byte order**,
- typical **PI timings**,
- a sane **file size**, and
- a recognized **IPL3/bootcode**.

Written in C99. X11 license. ```sha1.c``` and ```sha1.h``` are public domain.
