#/bin/bash
set -e 
VG="valgrind  --leak-check=full --show-leak-kinds=all "
VG=""
#-------------------
cd ../lua && luajit gen_so.lua test1 && cd -
gcc -g  $QC_FLAGS test1.c ../lua/libtest1.so -I../xgen_inc/ -I../inc
$VG  ./a.out
#-------------------
cd ../lua && luajit gen_so.lua test2 && cd -
gcc -g  $QC_FLAGS test2.c ../lua/libtest2.so -I../xgen_inc/ -I../inc
$VG  ./a.out
#-------------------
cd ../lua && luajit gen_so.lua test2 && cd -
gcc -g  $QC_FLAGS test2n.c ../lua/libtest2.so -I../xgen_inc/ -I../inc
$VG  ./a.out
#-------------------
gcc $QC_FLAGS test_fastdiv.c -I../inc/
$VG  ./a.out

echo SUCCESS
