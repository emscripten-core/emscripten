# e.g.
#   ~/Dev/emscripten/tests/fuzz$ CSMITH=~/Dev/csmith/src/csmith CSMITH_PATH=~/Dev/csmith python ./csmith_driver.py
# to find failures, then check those out with this script

echo "builds"
rm *.out *.bc *.js
g++ $@ -m32 -I/home/alon/Dev/csmith/runtime -o n1.out &> /dev/null
/home/alon/Dev/fastcomp/build/Release+Asserts/bin/clang++ -m32 -I/home/alon/Dev/csmith/runtime -o n2.out $@ &> /dev/null
/home/alon/Dev/fastcomp/build/Release+Asserts/bin/clang++ -m32 -I/home/alon/Dev/csmith/runtime -emit-llvm -c -o bc.bc $@ &> o
~/Dev/emscripten/em++ $@ -I/home/alon/Dev/csmith/runtime -s PRECISE_F32=1 -o fc.out.js &> /dev/null
~/Dev/emscripten/em++ $@ -s SAFE_HEAP=1 -I/home/alon/Dev/csmith/runtime -s PRECISE_F32=1 -o fc-sh.out.js &> /dev/null
echo "run n1"
./n1.out &> n1
echo "run n2"
./n2.out &> n2
echo "run bc"
/home/alon/Dev/fastcomp/build/Release/bin/lli bc.bc &> bc
#echo "run js"
#mozjs js.out.js &> js
#echo "run ua"
#mozjs ua.out.js &> ua
#echo "run sh"
#mozjs sh.out.js &> sh
echo "run fc"
mozjs fc.out.js &> fc
echo "run fc-sh"
mozjs fc-sh.out.js &> fc-sh
echo "n/n"
diff n1 n2
echo "n/bc"
diff n1 bc
#echo "n/js"
#diff n1 js | grep -v warning
#echo "n/js-ua"
#diff n1 ua | grep -v warning
#echo "n/js-sh"
#diff n1 sh | grep -v warning
#echo "js/fc"
#diff fc js | grep -v warning
#echo "js/fc-sh"
#diff fc-sh js | grep -v warning
echo "native/fc"
grep -v warning fc > fclean
diff n1 fclean | grep -v warning
echo "native2/fc"
grep -v warning fc > fclean
diff n2 fclean | grep -v warning

