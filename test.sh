#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./ghcc "$input" > tmp.s;
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ];then
        echo "$input => $actual"
    else
        echo "$input => $expected expected,but got $actual"
        exit 1;
    fi
}

assert 0 0
assert 42 42

echo === simlest eval ===
assert 21 '5+20-4'

echo === spaced eval ===
assert 60 '52 + 30 - 22'

echo OK
