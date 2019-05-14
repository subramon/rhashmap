/*
 * Copyright (c) 2017 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _RHASHMAP_H_
#define _RHASHMAP_H_

#define	Q_RHM_SET  1
#define	Q_RHM_INCR 2

#define KEYTYPE uint64_t
#define VALTYPE  int64_t

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"

typedef struct {
	KEYTYPE  key; 
	VALTYPE val;
	uint64_t	hash	: 32;
	uint64_t	psl	: 16;
} q_rh_bucket_t;

typedef struct {
	unsigned	size;
	unsigned	nitems;
	unsigned	flags;
	uint64_t	divinfo;
	q_rh_bucket_t *	buckets;
	uint64_t	hashkey;
	unsigned	minsize;
} q_rhashmap_t;

extern q_rhashmap_t *	
q_rhashmap_create(
    size_t initial_size
    );
extern void		
q_rhashmap_destroy(
    q_rhashmap_t *
    );

extern VALTYPE
q_rhashmap_get(
    q_rhashmap_t *, 
    KEYTYPE key
    );
extern int
q_rhashmap_put(
    q_rhashmap_t *, 
    KEYTYPE key,
    VALTYPE val,
    int update_type,
    VALTYPE *ptr_oldval
    );
extern int
q_rhashmap_del(
    q_rhashmap_t *, 
    KEYTYPE key,
    bool *ptr_key_exists,
    VALTYPE *ptr_oldval
    );

#endif
