#!/bin/bash
rm -rf 1.out 2.out
cd ../Pass/build/
cmake -DCMAKE_BUILD_TYPE=Release ../Transforms/ValueNumbering
make -j4
cd -
echo "-----------Start------------------"
clang -Xclang -disable-O0-optnone 1.c -O0 -S -emit-llvm -o 1.ll
opt 1.ll -mem2reg -S -o 1.bc
opt -load ../Pass/build/libLLVMValueNumberingPass.so  -ValueNumbering < 1.bc > /dev/null
echo "-----------End--------------------"
echo "-----------Start------------------"
clang -Xclang -disable-O0-optnone 2.c -O0 -S -emit-llvm -o 2.ll
opt 2.ll -mem2reg -S -o 2.bc
opt -load ../Pass/build/libLLVMValueNumberingPass.so  -ValueNumbering < 2.bc > /dev/null
echo "-----------End--------------------"
