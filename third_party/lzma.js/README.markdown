
lzma.js
=======

An LZMA implementation in JavaScript, compiled from lzip using Emscripten
with the goal of having a small and fast implementation as much as
possible.

lzip is GPL, additional code is MIT


Building
--------

Run doit.sh. It will build both a native build, lzma-native, and lzma.js.
Both are useful because you can use the native build to compress locally,
and the JS build to decompress on the client browser. But, you can use
whichever you want in either location.


Usage
-----

LZMA.compress(data) receives an array of 8-bit data and returns an
    array of compressed data.

LZMA.decompress(data) receives an array of compressed 8-bit data and
    returns an array of decompressed data.


Use lzma-decoder.js if you just need to decode. It's 66K, or 23K
if your webserver does gzip. If you need compression too, use
lzma-full.js which is a little larger.

See test-decoder.js, test-full.js and test-full.html for example
uses.

