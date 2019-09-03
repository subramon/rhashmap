#!/bin/bash
INCS=-I../inc/ -I../xgen_inc/
INCDIR=../xgen_inc/


all :  libhmap.so \
	../xgen_inc/_hmap_mk_loc.h   \
	../xgen_inc/_hmap_mk_tid.h   \
	../xgen_inc/_calc_new_size.h   \
	../xgen_inc/_validate_psl_p.h   \
	../xgen_src/_hmap_create_I4.c \
	../xgen_src/_hmap_destroy_I4.c \

hfilegen = ../../Q/UTILS/lua/cli_extract_func_decl.lua

$(INCDIR) :
	mkdir -p ../xgen_inc/

tmpl_files = \
	hmap_create.tmpl.lua  \
	hmap_destroy.tmpl.lua  \
	hmap_mk_hash.tmpl.lua  \
	hmap_type.tmpl.lua \
	hmap_resize.tmpl.lua \
	hmap_insert.tmpl.lua \

../xgen_src/_hmap_create_I4.c : ${tmpl_files}
	luajit generator.lua

../xgen_inc/_hmap_mk_loc.h : $(INCDIR) ${tmpl_files}
	lua ${hfilegen} ../src/hmap_mk_loc.c ${INCDIR}

../xgen_inc/_hmap_mk_tid.h : $(INCDIR) ${tmpl_files}
	lua ${hfilegen} ../src/hmap_mk_tid.c ${INCDIR}

../xgen_inc/_calc_new_size.h : $(INCDIR) ${tmpl_files}
	lua ${hfilegen} ../src/calc_new_size.c ${INCDIR}

../xgen_inc/_validate_psl_p.h : $(INCDIR) ${tmpl_files}
	lua ${hfilegen} ../src/validate_psl_p.c ${INCDIR}

hdr_files = \
	../xgen_inc/_hmap_mk_loc.h   \
	../xgen_inc/_hmap_mk_tid.h   \
	../xgen_inc/_calc_new_size.h   \
	../xgen_inc/_validate_psl_p.h   \

src_files = \
	    ../src/murmurhash.c \
	    ../src/calc_new_size.c \
	    ../src/hmap_mk_tid.c \
	    ../src/hmap_mk_loc.c \

xgen_src_files = \
	../xgen_src/_hmap_create_I4.c  \
	../xgen_src/_hmap_create_I8.c  \
	../xgen_src/_hmap_destroy_I4.c  \
	../xgen_src/_hmap_destroy_I8.c  \
	../xgen_src/_hmap_mk_hash_I4.c \
	../xgen_src/_hmap_mk_hash_I8.c \
	../xgen_src/_hmap_resize_I4_F4.c \
	../xgen_src/_hmap_resize_I4_F8.c \
	../xgen_src/_hmap_resize_I4_I1.c \
	../xgen_src/_hmap_resize_I4_I2.c \
	../xgen_src/_hmap_resize_I4_I4.c \
	../xgen_src/_hmap_resize_I4_I8.c \
	../xgen_src/_hmap_resize_I8_F4.c \
	../xgen_src/_hmap_resize_I8_F8.c \
	../xgen_src/_hmap_resize_I8_I1.c \
	../xgen_src/_hmap_resize_I8_I2.c \
	../xgen_src/_hmap_resize_I8_I4.c \
	../xgen_src/_hmap_resize_I8_I8.c \
	../xgen_src/_hmap_insert_I4_F4.c   \
	../xgen_src/_hmap_insert_I8_F4.c \
	../xgen_src/_hmap_insert_I4_F8.c   \
	../xgen_src/_hmap_insert_I8_F8.c \
	../xgen_src/_hmap_insert_I4_I1.c   \
	../xgen_src/_hmap_insert_I8_I1.c \
	../xgen_src/_hmap_insert_I4_I2.c   \
	../xgen_src/_hmap_insert_I8_I2.c \
	../xgen_src/_hmap_insert_I4_I4.c   \
	../xgen_src/_hmap_insert_I8_I4.c \
	../xgen_src/_hmap_insert_I4_I8.c   \
	../xgen_src/_hmap_insert_I8_I8.c \



../xgen_src/_hmap_mk_hash_I4.c  : hmap_mk_hash.tmpl.lua
	luajit generator.lua


clean :
	rm -f ../xgen_src/*
	rm -f ../xgen_inc/*
	rm -f _*
	rm -f *.so
	rm -f *.o

libhmap.so : ${hdr_files} ${xgen_src_files}
	gcc ${QC_FLAGS} ../xgen_src/*.c ${src_files} ${CFLAGS} ${INCS} -shared -o libs_to_f.so

