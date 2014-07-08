# useful for testing purposes, compare native build to js build
~/Dev/fastcomp/hybridbuild/Release+Asserts/bin/clang -std=c++0x -nostdinc++ test.cpp libcxxabi/src/*.cpp libcxx/*.cpp -Ilibcxxabi/include -I../include/libcxx -lpthread -g

