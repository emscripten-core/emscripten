# e.g.
#   ~/Dev/emscripten/tests/fuzz$ CSMITH=~/Dev/csmith/src/csmith CSMITH_PATH=~/Dev/csmith python ./csmith_driver.py
# to find failures, then check those out with this script

echo "builds"
gcc $@ -I/home/alon/Dev/csmith/runtime -o n1.out &> /dev/null
/home/alon/Dev/fastcomp/build/Release/bin/clang $@ -I/home/alon/Dev/csmith/runtime -o n2.out &> /dev/null
/home/alon/Dev/fastcomp/build/Release/bin/clang $@ -I/home/alon/Dev/csmith/runtime -emit-llvm -c -o bc.bc &> o
~/Dev/emscripten/emcc $@ -I/home/alon/Dev/csmith/runtime -o js.out.js &> /dev/null
#~/Dev/emscripten/emcc $@ -s UNALIGNED_MEMORY=1 -I/home/alon/Dev/csmith/runtime -o ua.out.js &> /dev/null
#~/Dev/emscripten/emcc $@ -s SAFE_HEAP=1 -I/home/alon/Dev/csmith/runtime -o sh.out.js &> /dev/null
EMCC_FAST_COMPILER=1 ~/Dev/emscripten/emcc $@ -I/home/alon/Dev/csmith/runtime -o fc.out.js &> /dev/null
echo "run n1"
./n1.out &> n1
echo "run n2"
./n2.out &> n2
echo "run bc"
/home/alon/Dev/fastcomp/build/Release/bin/lli bc.bc &> bc
echo "run js"
mozjs js.out.js &> js
echo "run ua"
#mozjs ua.out.js &> ua
echo "run sh"
#mozjs sh.out.js &> sh
echo "run fc"
mozjs fc.out.js &> fc
echo "n/n"
diff n1 n2
echo "n/bc"
diff n1 bc
echo "n/js"
diff n1 js | grep -v warning
echo "n/js-ua"
#diff n1 ua | grep -v warning
echo "n/js-sh"
#diff n1 sh | grep -v warning
echo "js/js"
diff js fc | grep -v warning

