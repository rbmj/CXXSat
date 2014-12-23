#!/bin/bash
set -e

TIME=`which time`

pad() {
    local len=${#1}
    local count=`expr $2 - $len`
    local i=0
    while [ $i -lt $count ]; do
        echo -n 0
        let i=i+1
    done
    echo $1
}

doz3() {
    local numbits=$1
    local mulbits=$2
    local extend=$3
    local n=$4
    $TIME -f '%U' z3 -in -smt2 2>&1 >/dev/null << EOF
(declare-const p (_ BitVec $numbits))
(declare-const q (_ BitVec $numbits))

(define-fun extend ((x (_ BitVec $numbits))) (_ BitVec $mulbits)
    (concat #x$extend x))

(assert (= 
    (bvmul (extend p) (extend q))
    #x$n))

(check-sat)
(get-model)
EOF
}

docxx() {
    local numbits=$1
    local mulbits=$2
    local n=$3
    $TIME -f '%U' ../build/cxxsat -function=mul -value=0x$n /dev/stdin -- -x c++ 2>&1 >/dev/null << EOF
#include <stdint.h>
uint${mulbits}_t mul(uint${numbits}_t x, uint${numbits}_t y) {
    uint${mulbits}_t a(x), b(y);
    return a*b;
}
EOF
}

docxx2() {
    local mulbits=$1
    local n=$2
    $TIME -f '%U' ../build/FactorTest $1 `python -c "print 0x${n}L"` 2>&1 >/dev/null
}

dominisat() {
    local numbits=$1
    local mulbits=$2
    local n=$3
    ../build/cxxsat -function=mul -value=0x$n /dev/stdin -dump -- -x c++ << EOF |
#include <stdint.h>
uint${mulbits}_t mul(uint${numbits}_t x, uint${numbits}_t y) {
    uint${mulbits}_t a(x), b(y);
    return a*b;
}
EOF
$TIME -f '%U' ./minisat.sh 2>&1 >/dev/null
}

dominisat2() {
    local mulbits=$1
    local n=$2
    ../build/FactorTest $1 `python -c "print 0x${n}L"` -dump | $TIME -f '%U' ./minisat.sh 2>&1 >/dev/null
}

docryptominisat() {
    local numbits=$1
    local mulbits=$2
    local n=$3
    ../build/cxxsat -function=mul -value=0x$n /dev/stdin -dump -- -x c++ << EOF |
#include <stdint.h>
uint${mulbits}_t mul(uint${numbits}_t x, uint${numbits}_t y) {
    uint${mulbits}_t a(x), b(y);
    return a*b;
}
EOF
$TIME -f '%U' ./cryptominisat.sh 2>&1 >/dev/null
}

docryptominisat2() {
    local mulbits=$1
    local n=$2
    ../build/FactorTest $1 `python -c "print 0x${n}L"` -dump | $TIME -f '%U' ./cryptominisat.sh 2>&1 >/dev/null
}

benchmark() {
    local p=`./genprime.py $1`
    local q=`./genprime.py $1`
    local n=`./mulprimes.py $p $q`
    local z3tm=''
    local extend=''
    local numbits=''
    local mulbits=''
    local n_pad=''
    local numbits_2=`expr ${#p} \* 4`
    local mulbits_2=`expr ${#n} \* 4`
    local extend_2=`pad '' ${#p}`

    if [ $1 -gt 16 ]; then
        numbits=32
        mulbits=64
        extend=`pad '' 8`
        n_pad=`pad $n 16`
    else
        numbits=16
        mulbits=32
        extend=`pad '' 4`
        n_pad=`pad $n 8`
    fi

    #z3_full=`doz3 $numbits $mulbits $extend $n_pad`
    z3_opt=`doz3 $numbits_2 $mulbits_2 $extend_2 $n`
    #cxx=`docxx $numbits $mulbits $n`
    cxx_opt=`docxx2 $mulbits_2 $n`
    #minisat=`dominisat $numbits $mulbits $n`
    minisat_opt=`dominisat2 $mulbits_2 $n`
    #cryptominisat=`docryptominisat $numbits $mulbits $n`
    cryptominisat_opt=`docryptominisat2 $mulbits_2 $n`

    #echo $z3_full,$z3_opt,$cxx,$cxx_opt,$minisat,$minisat_opt,$cryptominisat,$cryptominisat_opt
    #echo $cxx_opt,$minisat,$minisat_opt,$cryptominisat,$cryptominisat_opt
    echo $z3_opt,$cxx_opt,$minisat_opt,$cryptominisat_opt
}

for i in `seq 1 40`; do
    benchmark 20 >> results.csv
done
