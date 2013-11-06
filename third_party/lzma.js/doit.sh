cd lzip

export CXX=`../../../em-config LLVM_ROOT`/clang++

echo "native"
make clean
DECODER_ONLY=0 make lzip -j 4 # native build
case `uname` in
    *_NT*)
        mv lzip.exe ../lzma-native.exe
        ;;
    *)
        mv lzip ../lzma-native
        ;;
esac

exit # just build natively, that's it

echo "bitcode full (encoder+decoder)"
make clean
DECODER_ONLY=0 ../../../emmake make lzip -j 4
mv lzip lzip-full.bc

echo "bitcode decoder only"
make clean
DECODER_ONLY=1 ../../../emmake make lzip -j 4
mv lzip lzip-decoder.bc

cd ..

echo "javascript full"
../../emcc -O2 lzip/lzip-full.bc -o lzma-full.raw.js
# -s INLINING_LIMIT=0
cat pre.js > lzma-full.js
cat lzma-full.raw.js >> lzma-full.js
cat post.js >> lzma-full.js

echo "javascript decoder"
../../emcc -O2 lzip/lzip-decoder.bc -o lzma-decoder.raw.js
# -s INLINING_LIMIT=0
cat pre.js > lzma-decoder.js
cat lzma-decoder.raw.js >> lzma-decoder.js
cat post.js >> lzma-decoder.js

