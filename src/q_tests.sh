#!/bin/bash
set -e 
rm -f _*
CFLAGS=" -std=gnu99 -Wall -Wextra -Werror -fopenmp -g -D_GNU_SOURCE -D_DEFAULT_SOURCE -Wno-unknown-warning-option  -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings -Wold-style-definition -Wsuggest-attribute=noreturn -Wduplicated-cond -Wmisleading-indentation -Wnull-dereference -Wduplicated-branches -Wrestrict "

FILES=" q_rhashmap.c murmurhash.c "
gcc $CFLAGS $FILES test_q_rhashmap_1.c -o a.out
./a.out

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

echo "Q tests for rhashmap succeeded"
