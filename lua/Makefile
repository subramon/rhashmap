#!/bin/bash
INCS=-I../inc/ -I../gen_inc/
INCDIR=../gen_inc/


all : ${hdr_files} 

hfilegen = ../../Q/UTILS/lua/cli_extract_func_decl.lua

$(INCDIR) :
	mkdir -p ../gen_inc/

../gen_inc/_hmap_mk_loc.h : $(INCDIR)
	lua ${hfilegen} ../src/hmap_mk_loc.c ${INCDIR}

../gen_inc/_hmap_mk_tid.h : $(INCDIR)
	lua ${hfilegen} ../src/hmap_mk_tid.c ${INCDIR}

../gen_inc/_calc_new_size.h : $(INCDIR)
	lua ${hfilegen} ../src/calc_new_size.c ${INCDIR}

hdr_files = \
	../gen_inc/_hmap_mk_loc.h   \
	../gen_inc/_hmap_mk_tid.h   \
	../gen_inc/_calc_new_size.h   \

clean :
	rm -f ../gen_src/*
	rm -f ../gen_inc/*
	rm -f _*
	rm -f *.so
	rm -f *.o

