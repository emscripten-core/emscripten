#!/bin/sh

echo "relooper"
g++ Relooper.cpp -c -g
g++ Relooper.cpp -c -g -DDEBUG -o RelooperDebug.o

echo "test"
g++ test.cpp -c -o test.o -g
g++ Relooper.o test.o -o test

echo "test 2"
gcc test2.c -c -o test2.o -g
g++ Relooper.o test2.o -o test2

echo "test 3"
gcc test3.c -c -o test3.o -g
g++ Relooper.o test3.o -o test3

echo "test debug"
gcc test_debug.cpp -c -o test_debug.o -g -DDEBUG
g++ RelooperDebug.o test_debug.o -o test_debug

echo "test dead"
gcc test_dead.cpp -c -o test_dead.o -g
g++ Relooper.o test_dead.o -o test_dead

echo "test 4"
g++ test4.cpp -c -o test4.o -g
g++ Relooper.o test4.o -o test4

echo "test 5"
g++ test5.cpp -c -o test5.o -g
g++ Relooper.o test5.o -o test5

echo "test 6"
g++ test6.cpp -c -o test6.o -g
g++ Relooper.o test6.o -o test6

echo "test inf"
g++ test_inf.cpp -c -o test_inf.o -g
g++ Relooper.o test_inf.o -o test_inf

echo "test fuzz1"
g++ test_fuzz1.cpp -c -o test_fuzz1.o -g
g++ Relooper.o test_fuzz1.o -o test_fuzz1

echo "test fuzz2"
g++ test_fuzz2.cpp -c -o test_fuzz2.o -g
g++ Relooper.o test_fuzz2.o -o test_fuzz2

echo "test fuzz3"
g++ test_fuzz3.cpp -c -o test_fuzz3.o -g
g++ Relooper.o test_fuzz3.o -o test_fuzz3

echo "test fuzz4"
g++ test_fuzz4.cpp -c -o test_fuzz4.o -g
g++ Relooper.o test_fuzz4.o -o test_fuzz4

echo "test fuzz5"
g++ test_fuzz5.cpp -c -o test_fuzz5.o -g
g++ Relooper.o test_fuzz5.o -o test_fuzz5

echo "test fuzz6"
g++ test_fuzz6.cpp -c -o test_fuzz6.o -g
g++ Relooper.o test_fuzz6.o -o test_fuzz6

echo
echo "============================="
echo

