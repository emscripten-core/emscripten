#!/bin/bash

echo "test"
./test &> test.out
diff -w -U 5 test.txt test.out

echo "test 2"
./test2 &> test2.out
diff -w -U 5 test2.txt test2.out

echo "test 3"
./test3 &> test3.out
diff -w -U 5 test3.txt test3.out

echo "test debug"
./test_debug &> test_debug.out
diff -w -U 5 test_debug.txt test_debug.out

echo "test dead"
./test_dead &> test_dead.out
diff -w -U 5 test_dead.txt test_dead.out

echo "test 4"
./test4 &> test4.out
diff -w -U 5 test4.txt test4.out

echo "test 5"
./test5 &> test5.out
diff -w -U 5 test5.txt test5.out

echo "test 6"
./test6 &> test6.out
diff -w -U 5 test6.txt test6.out

echo "test inf"
./test_inf &> test_inf.out
diff -w -U 5 test_inf.txt test_inf.out

echo "test fuzz1"
./test_fuzz1 &> test_fuzz1.out
diff -w -U 5 test_fuzz1.txt test_fuzz1.out

echo "test fuzz2"
./test_fuzz2 &> test_fuzz2.out
diff -w -U 5 test_fuzz2.txt test_fuzz2.out

echo "test fuzz3"
./test_fuzz3 &> test_fuzz3.out
diff -w -U 5 test_fuzz3.txt test_fuzz3.out

echo "test fuzz4"
./test_fuzz4 &> test_fuzz4.out
diff -w -U 5 test_fuzz4.txt test_fuzz4.out

echo "test fuzz5"
./test_fuzz5 &> test_fuzz5.out
diff -w -U 5 test_fuzz5.txt test_fuzz5.out

echo "test fuzz6"
./test_fuzz6 &> test_fuzz6.out
diff -w -U 5 test_fuzz6.txt test_fuzz6.out

