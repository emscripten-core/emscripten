# e.g.
#   ~/Dev/emscripten/tests/fuzz$ CSMITH=~/Dev/csmith/src/csmith CSMITH_PATH=~/Dev/csmith python ./csmith_driver.py
# to find failures, then check those out with this script

echo "0"
gcc $1 -I/home/alon/Dev/csmith/runtime
~/Dev/emscripten/emcc $1 -I/home/alon/Dev/csmith/runtime
./a.out
mozjs a.out.js
echo "1"
gcc -O1 $1 -I/home/alon/Dev/csmith/runtime
~/Dev/emscripten/emcc -O1 $1 -I/home/alon/Dev/csmith/runtime
./a.out
mozjs a.out.js
echo "2"
gcc -O2 $1 -I/home/alon/Dev/csmith/runtime
~/Dev/emscripten/emcc -O2 $1 -I/home/alon/Dev/csmith/runtime
./a.out
mozjs a.out.js

