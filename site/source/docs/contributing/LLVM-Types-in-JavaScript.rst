.. _LLVM-Types-in-JavaScript:

======================================
LLVM Types in JavaScript (wiki-import)
======================================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

This is an overview of how LLVM types are implemented in JS by
Emscripten. This stuff has changed quite a bit over time (if you read
this and it has not been updated in a while, better to check the
source).

Overflow corrections:

::

    i32: | 0
    less: & (2^bits-1)

Sign corrections (done right before a math operation if necessary, and
to extend):

::

    i32 signed: | 0
    i32 unsigned: >>> 0
    less signed: << (32-bits) >> (32-bits)
    less unsigned: & (2^bits-1)

Truncate:

::

    & (2&bits-1)

Rounding the output of a division:

::

    i32 or less, signed: & -1 (could be |0 as well)
    unsigned: Math.floor(.)
    >i32 or float-to-int: x >= 0 ? Math.floor(x) : Math.ceil(x)

Bitwise cast of float to int and vice versa:

::

    Write to heap as int, read as float (and vice versa)

Note that there are some ugly optimizations for i64s as emulated
doubles, e.g. a sign correction could look like

::

    (x >= 0 ? x : (2^64)+x)

Also, if we do precise i64 (not with emulated doubles), we use
[low,high] (i.e. a js array) as the representation in some cases.

LLVM has "non-C struct" types, that we implement as JS objects with
fields. These are typically used only in odd things like exception
handling, checking for explicit overflows, etc., so not usually
performance sensitive.
