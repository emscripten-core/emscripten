#!/bin/sh
while true; do (/bin/echo -en "te\x01\xff\x79st\x02") | nc -vvvl 127.0.0.1 8990; done;

