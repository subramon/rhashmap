#!/bin/bash
set -e 
rm -f _*
CFLAGS=" -std=gnu99 -Wall -Wextra -Werror -fopenmp -g -D_GNU_SOURCE -D_DEFAULT_SOURCE -Wno-unknown-warning-option  -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings -Wold-style-definition -Wsuggest-attribute=noreturn -Wduplicated-cond -Wmisleading-indentation -Wnull-dereference -Wduplicated-branches -Wrestrict "
VG=" valgrind --leak-check=full --show-leak-kinds=all  "

FILES=" q_rhashmap.c murmurhash.c "
VG="" # Uncomment this line if you do not want Valgrind to run

ls test_q_rhashmap_*.c > _files
while read line; do
  echo "Testing $line"
  gcc $CFLAGS $FILES $line -o a.out
  $VG ./a.out 1> _out 2>_err
  cat _out
  if [ "$VG" != "" ]; then 
    grep "0 errors from 0 contexts" _err 1>/dev/null 2>&1
  fi
done < _files

##--- Generate documentation
ofile_prefix=_q_tests
texfile=${ofile_prefix}.tex
pdffile=${ofile_prefix}.pdf
cp preamble.tex $texfile
echo "\\title{Unit Tests for RhashMap in Q}" >> $texfile
grep '\/\/C'  test_q*.c | \
  sed s'/\/\/C/ /'g | \
  sed s'/^test_q_rha*.*:/ /'g >> $texfile
echo "\\end{document}" >> $texfile
latex2pdf $texfile > $pdffile
cp $pdffile /tmp/

echo "SUCCESS: Q tests for rhashmap succeeded"
