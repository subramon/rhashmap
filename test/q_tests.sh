#!/bin/bash
set -e 
rm -f _*
if [ $# != 0 ] && [ $# != 1 ]; then 
  echo "ERROR. Usage is $0 or $0 <N> where N is test num"; exit 1; 
fi
if [ $# = 1 ]; then
  TESTNUM=$1
else
  TESTNUM=0
fi

CFLAGS=" -std=gnu99 -Wall -Wextra -Werror -fopenmp -g -D_GNU_SOURCE -D_DEFAULT_SOURCE -Wno-unknown-warning-option  -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings -Wold-style-definition -Wsuggest-attribute=noreturn -Wduplicated-cond -Wmisleading-indentation -Wnull-dereference -Wduplicated-branches -Wrestrict "
VG=" valgrind --leak-check=full --show-leak-kinds=all  "

cat invariants.tmpl.c | \
  sed s'/__KV__/I8_I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' | \
  sed s'/__VALTYPE__/int64_t/g ' > _invariants_I8_I8.c
cat invariants.tmpl.h | \
  sed s'/__KV__/I8_I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' | \
  sed s'/__VALTYPE__/int64_t/g ' > _invariants_I8_I8.h
cat ../src/q_rhashmap.tmpl.h | \
  sed s'/__KV__/I8_I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' | \
  sed s'/__VALTYPE__/int64_t/g ' > _q_rhashmap_I8_I8.h
cat ../src/q_rhashmap_mk_hash.tmpl.h | \
  sed s'/__K__/I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' > _q_rhashmap_mk_hash_I8.h
cat ../src/q_rhashmap_mk_hash.tmpl.c | \
  sed s'/__K__/I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' > _q_rhashmap_mk_hash_I8.c
cat ../src/q_rhashmap_struct.tmpl.h | \
  sed s'/__KV__/I8_I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' | \
  sed s'/__VALTYPE__/int64_t/g ' > _q_rhashmap_struct_I8_I8.h
cat ../src/q_rhashmap.tmpl.c | \
  sed s'/__KV__/I8_I8/g ' | \
  sed s'/__KEYTYPE__/uint64_t/g ' | \
  sed s'/__VALTYPE__/int64_t/g ' > _q_rhashmap_I8_I8.c
FILES=" _q_rhashmap_mk_hash_I8.c _q_rhashmap_I8_I8.c ../src/q_rhashmap_mk_tid.c ../src/q_rhashmap_mk_loc.c ../src/murmurhash.c _invariants_I8_I8.c  "

VG="" # Uncomment this line if you do not want Valgrind to run

if [ $TESTNUM = 0 ]; then 
  ls test_q_rhashmap_*.c > _files
else
  ls test_q_rhashmap_*.c | grep $TESTNUM > _files
fi
nfiles=0
while read line; do
  echo "Testing $line"
  gcc $CFLAGS -I. -I../src/ $FILES $line -o a.out
  $VG ./a.out 1> _out 2>_err
  cat _out
  if [ "$VG" != "" ]; then 
    grep "0 errors from 0 contexts" _err 1>/dev/null 2>&1
  fi
  nfiles=`expr $nfiles + 1 `
done < _files
if [ $nfiles = 0 ]; then echo "ERROR: No files to test"; exit 1; fi

##--------
gcc -std=gnu99 -I../src/ test_fastdiv.c; ./a.out
##--- Generate documentation
ofile_prefix=_q_tests
texfile=${ofile_prefix}.tex
pdffile=${ofile_prefix}.pdf
cp preamble.tex $texfile
cat invariants.tex >> $texfile
while read line; do
  echo "Documenting $line to $texfile"

  cat $line | grep '\/\/C'  | \
  sed s'/\/\/C/ /'g | \
  sed s'/^test_q_rha*.*:/ /'g | \
  sed s'/^[ ]*//'g  > _tempf
  cat _tempf >> $texfile
  
done < _files

echo "\\end{document}" >> $texfile
latex2pdf $texfile > $pdffile
cp $pdffile /tmp/

rm a.out _*.tex _out _err _files \
  _tempf _q_tests.out _q_tests.aux _q_tests.log
echo "SUCCESS: Q tests for rhashmap succeeded"
