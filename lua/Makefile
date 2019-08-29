#!/bin/bash
INCS=-I../inc/ -I../gen_inc/
INCDIR=../gen_inc/


all : ${hdr_files} 

hfilegen = ../../Q/UTILS/lua/cli_extract_func_decl.lua

$(INCDIR) :
	mkdir -p ../gen_inc/

../gen_inc/_q_rhashmap_mk_loc.h : $(INCDIR)
	lua ${hfilegen} ../src/q_rhashmap_mk_loc.c ${INCDIR}

hdr_files = \
	../gen_inc/_q_rhashmap_mk_loc.h   \

#--------
src_files =  \
	bits_to_bytes.c \
	bytes_to_bits.c \
	B1_to_txt.c \
	file_exists.c \
	generate_bin.c \
	get_bit_u64.c  \
	get_file_size.c \
	get_time_usec.c \
	is_valid_chars_for_num.c \
	isfile.c \
	isdir.c \
	q_omp_get_num_procs.c  \
	rand_file_name.c \
	rdtsc.c \
	rs_mmap.c \
	SC_to_txt.c \
	set_bit_u64.c  \
	spooky_hash.c \
	trim.c \
	txt_to_B1.c \
	txt_to_SC.c \
	../gen_src/_I1_to_txt.c  \
	../gen_src/_I2_to_txt.c  \
	../gen_src/_I4_to_txt.c  \
	../gen_src/_I8_to_txt.c  \
	../gen_src/_F4_to_txt.c  \
	../gen_src/_F8_to_txt.c \
	../gen_src/_txt_to_I1.c \
	../gen_src/_txt_to_I2.c \
	../gen_src/_txt_to_I4.c \
	../gen_src/_txt_to_I8.c \
	../gen_src/_txt_to_F4.c \
	../gen_src/_txt_to_F8.c \
#----------------------------------

clean :
	rm -f ../gen_src/*
	rm -f ../gen_inc/*
	rm -f _*
	rm -f *.so
	rm -f *.o

