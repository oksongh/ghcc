#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./ghcc "$input" > tmp.s;
    
    if [ "$?" == "1" ];then
        if [ "$expected" == "1" ];then
            return 
        else
            exit 1    
        fi
    fi
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
assert 43 '5 + 3 * 12 + 2'

echo === priority eval ===
assert 47 '5+6*7'
assert 77 '(5+6)*7'
assert 19 '2*(5+3)+3'
assert 12 '(2+1)*(1+2)+3'


echo === compile error ===
assert 1 '1 + a'

assert 1 '1 + +'

echo OK
