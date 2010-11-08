echo "gccing... (run me in bullet/demos/HelloWorld)"
~/Dev/llvm-gcc-4.2-2.8.source/cbuild/install/bin/llvm-g++ -c HelloWorld.cpp -I../../src -emit-llvm -o HelloWorld.o
#echo "linking..."
#~/Dev/llvm-2.8/cbuild/Release/bin/llvm-link HelloWorld.o /dev/shm/tmp/libBullet.bc -o /dev/shm/tmp/bulletTest.bc
#echo "dissing..."
#~/Dev/llvm-2.8/cbuild/Release/bin/llvm-dis -show-annotations /dev/shm/tmp/bulletTest.bc
#echo "emscriptening..."
#python ~/Dev/emscripten/emscripten.py /dev/shm/tmp/bulletTest.ll > /dev/shm/tmp/bulletTest.js

echo "dissing..."
~/Dev/llvm-2.8/cbuild/Release/bin/llvm-dis -show-annotations HelloWorld.o
echo "emscriptening..."
python ~/Dev/emscripten/emscripten.py HelloWorld.o.ll ~/Dev/tracemonkey/js/src/js '{ "USE_TYPED_ARRAYS": 1 }' &> HelloWorld.js

