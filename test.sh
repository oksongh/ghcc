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

assert_as_main(){
    assert "$1" "main(){$2}"
}

assert_as_main 0 '0;'
assert_as_main 42 '42;'

echo === simplest eval ===
assert_as_main 21 '5+20-4;'

echo === spaced eval ===
assert_as_main 60 '52 + 30 - 22;'
assert_as_main 43 '5 + 3 * 12 + 2;'

echo === priority eval ===
assert_as_main 47 '5+6*7;'
assert_as_main 77 '(5+6)*7;'
assert_as_main 19 '2*(5+3)+3;'
assert_as_main 12 '(2+1)*(1+2)+3;'

echo === unary ===
assert_as_main 3 '(1-2)*-3;'
assert_as_main 1 '+3+-2;'

echo === compare ===
assert_as_main 1 '1==1;'
assert_as_main 1 '123==123;'
assert_as_main 1 '-1==-1;'
assert_as_main 0 '1==2;'
assert_as_main 0 '-1==-2;'

assert_as_main 1 '-1!=-2;'
assert_as_main 1 '1!=2;'
assert_as_main 0 '1!=1;'
assert_as_main 0 '123!=123;'
assert_as_main 0 '-1!=-1;'

assert_as_main 1 '1<2;'
assert_as_main 0 '2<1;'
assert_as_main 0 '1<1;'

assert_as_main 1 '1<=2;'
assert_as_main 1 '1<=1;'
assert_as_main 1 '-2<=-2;'
assert_as_main 1 '-3<=-2;'
assert_as_main 0 '-2<=-3;'
assert_as_main 0 '2<=1;'

assert_as_main 1 '2>1;';
assert_as_main 1 '-2>-3;'
assert_as_main 0 '1>2;';
assert_as_main 0 '1>1;';

assert_as_main 1 '2>=1;'
assert_as_main 1 '1>=1;'
assert_as_main 0 '1>=2;'
assert_as_main 0 '-3>=-2;'

echo === statement ===
assert_as_main 10 'a=10;'
assert_as_main 10 'a=10;a;'
assert_as_main 4 'a = 10;a = 4;a;'
assert_as_main 1 'z=-4;z+10==6;'
assert_as_main 1 'a=z=-4;a==z;'
assert_as_main 0 'a=z=-4;a+z+8;'

echo === multiple variable name ===
assert_as_main 10 'abc=10;abc;'
assert_as_main 0 'num=-10;num+10;'

echo === return ===
assert_as_main 10 'return 10;'
assert_as_main 200 'return 10*20;'
assert_as_main 6 'a = 10/2; return a+1;'

echo === if statement ===
assert_as_main 2 'if(1) 2;'
assert_as_main 8 'if(1) 8; else 3;'
assert_as_main 102 'a=0;if(1) a=102;else a=3; a;'

assert_as_main 2 'if(0) 1; else 2;'
assert_as_main 2 'a=0;if(0) a=1; else a=2; a;'
assert_as_main 2 'a=0;if(0) a=1;else a=2; a;'

assert_as_main 3 'a=0;
if(1)
    if(0) 2;
    else 3;
 else
    4;
'

assert_as_main 20 'if(0) 0; else if(1) 20;'
assert_as_main 2 'if(0) 0; else if(0) 1; else 2;'

echo === for statement ===
assert_as_main 10 'a=0;for(i=0;i<10;i=i+1) a=a+1; return a;'
assert_as_main 45 'a=0;for(i=0;i<10;i=i+1) a=a+i; return a;'
assert_as_main 10 'a=0;for(i=0;i<10;i=i+1) return 10;'
assert_as_main 45 'for(;;) return 45;'

echo === while statement ===
assert_as_main 10 'i=10;while(i>0)i=i-1; return i+10;'
assert_as_main 10 'while(1) return 10; return 4;'
assert_as_main 10 'while(0) return 4; return 10;'

echo === block statement ===
assert_as_main 15 '{a=0; a=a+10; return a+5;}'
assert_as_main 10 '{} return 10;'
assert_as_main 10 'while(1){a=a*2; return 10;}'
assert_as_main 3 '
i = 0;
while(1){
    i = i+1;
    if(i==3){
        return i;
    }
}'

assert_as_main 55 'a = 0;b = 0;
for(i=0;i<10;i=i+1){
    a=a+1;
    b=a+b;
}
return b;'

echo === call function ===
assert_as_main 100 'testnoarg();'
assert_as_main 15 'testIdentity(15);'
assert_as_main 6 '1+testAdd(2,3);'
assert_as_main 112 'testAdd(100,12);'
assert_as_main 110 'testAdd(100, 2+8);'
assert_as_main 10 'testAdd(testAdd(1,2),testAdd(3,4));'
assert_as_main 6 'testWeightedSum(1,2,3,4,5,6);'

echo === define function ===
assert 10 '
main(){
    wrapprint(10);
    return 10;
}
wrapprint(n){
    println(n);
}
'
assert 10 '
main(){
    return identity(10);
}
identity(n){
    return n;
}
'
assert 22 '
main(){
    return addfunc(10,12);
}
addfunc(n,m){
    return n+m;
}
'
# recursive define func
# 0-index ,0 start fibonacci
assert 13 '
main(){
    return fibonacci(7);
}
fibonacci(n){
    return fib(0,1,n,0);
}
fib(a,b,n,i){
    println(a);
    if(n == i){
        return a;
    }
    return fib(b,a+b,n,i+1);
}
'

echo === throw compile error ===
# assert_as_main 1 '1 + a'
# assert_as_main 1 '1 + +'

echo OK
