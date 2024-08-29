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
    cc -o tmp tmp.s test_func.o
    ./tmp
    actual="$?"

    if [ "$actual" == "$expected" ];then
        echo "$input => $actual"
    else
        echo "$input => $expected expected,but got $actual"
        exit 1;
    fi
}

assert 0 '0;'
assert 42 '42;'

echo === simlest eval ===
assert 21 '5+20-4;'

echo === spaced eval ===
assert 60 '52 + 30 - 22;'
assert 43 '5 + 3 * 12 + 2;'

echo === priority eval ===
assert 47 '5+6*7;'
assert 77 '(5+6)*7;'
assert 19 '2*(5+3)+3;'
assert 12 '(2+1)*(1+2)+3;'

echo === unary ===
assert 3 '(1-2)*-3;'
assert 1 '+3+-2;'

echo === compare ===
assert 1 '1==1;'
assert 1 '123==123;'
assert 1 '-1==-1;'
assert 0 '1==2;'
assert 0 '-1==-2;'

assert 1 '-1!=-2;'
assert 1 '1!=2;'
assert 0 '1!=1;'
assert 0 '123!=123;'
assert 0 '-1!=-1;'

assert 1 '1<2;'
assert 0 '2<1;'
assert 0 '1<1;'

assert 1 '1<=2;'
assert 1 '1<=1;'
assert 1 '-2<=-2;'
assert 1 '-3<=-2;'
assert 0 '-2<=-3;'
assert 0 '2<=1;'

assert 1 '2>1;';
assert 1 '-2>-3;'
assert 0 '1>2;';
assert 0 '1>1;';

assert 1 '2>=1;'
assert 1 '1>=1;'
assert 0 '1>=2;'
assert 0 '-3>=-2;'

echo === statement ===
assert 10 'a=10;'
assert 10 'a=10;a;'
assert 4 'a = 10;a = 4;a;'
assert 1 'z=-4;z+10==6;'
assert 1 'a=z=-4;a==z;'
assert 0 'a=z=-4;a+z+8;'

echo === multiple variable name ===
assert 10 'abc=10;abc;'
assert 0 'num=-10;num+10;'

echo === return ===
assert 10 'return 10;'
assert 200 'return 10*20;'
assert 6 'a = 10/2; return a+1;'

echo === if statement ===
assert 2 'if(1) 2;'
assert 8 'if(1) 8; else 3;'
assert 102 'a=0;if(1) a=102;else a=3; a;'

assert 2 'if(0) 1; else 2;'
assert 2 'a=0;if(0) a=1; else a=2; a;'
assert 2 'a=0;if(0) a=1;else a=2; a;'

assert 3 'a=0;
if(1)
    if(0) 2;
    else 3;
 else
    4;
'

assert 20 'if(0) 0; else if(1) 20;'
assert 2 'if(0) 0; else if(0) 1; else 2;'

echo === for statement ===
assert 10 'a=0;for(i=0;i<10;i=i+1) a=a+1; return a;'
assert 45 'a=0;for(i=0;i<10;i=i+1) a=a+i; return a;'
assert 10 'a=0;for(i=0;i<10;i=i+1) return 10;'
assert 45 'for(;;) return 45;'

echo === while statement ===
assert 10 'i=10;while(i>0)i=i-1; return i+10;'
assert 10 'while(1) return 10; return 4;'
assert 10 'while(0) return 4; return 10;'

echo === block statement ===
assert 15 '{a=0; a=a+10; return a+5;}'
assert 10 '{} return 10;'
assert 10 'while(1){a=a*2; return 10;}'
assert 3 '
i = 0;
while(1){
    i = i+1;
    if(i==3){
        return i;
    }
}'

assert 55 'a = 0;b = 0;
for(i=0;i<10;i=i+1){
    a=a+1;
    b=a+b;
}
return b;'

echo === call function ===
assert 100 'testnoarg();'
assert 15 'testIdentity(15);'
assert 6 '1+testAdd(2,3);'
assert 112 'testAdd(100,12);'
assert 110 'testAdd(100, 2+8);'
assert 10 'testAdd(testAdd(1,2),testAdd(3,4));'
assert 6 'testWeightedSum(1,2,3,4,5,6);'

echo === throw compile error ===
# assert 1 '1 + a'
# assert 1 '1 + +'

echo OK
