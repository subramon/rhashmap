/*
 * Copyright (c) 2017 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _RHASHMAP_H_
#define _RHASHMAP_H_

#define	Q_RHM_SET  1
#define	Q_RHM_ADD 2

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
	uint32_t	size;
	uint32_t	nitems;
	uint32_t	flags;
	uint64_t	divinfo;
	q_rh_bucket_t *	buckets;
	uint64_t	hashkey;
	uint32_t	minsize;
} q_rhashmap_t;

extern q_rhashmap_t *	
q_rhashmap_create(
    size_t initial_size
    );
extern void		
q_rhashmap_destroy(
    q_rhashmap_t *
    );

extern int
q_rhashmap_get(
    q_rhashmap_t *, 
    KEYTYPE key,
    VALTYPE *ptr_val,
    bool *ptr_is_found
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
    VALTYPE *ptr_oldval,
    bool *ptr_is_found
    );

extern int 
q_rhashmap_getn(
    q_rhashmap_t *hmap, 
    KEYTYPE  *keys, // [nkeys] 
    uint32_t *hashes, // [nkeys] 
    uint32_t *locs, // [nkeys] 
    VALTYPE  *vals, // [nkeys] 
    uint32_t nkeys
    );

extern int 
q_rhashmap_murmurhash(
    KEYTYPE *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    );
extern int 
q_rhashmap_get_loc(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t hmap_size, // input 
    uint64_t hmap_divinfo, // input 
    uint32_t *locs // [nkeys] 
    );
extern int 
q_rhashmap_setn(
    q_rhashmap_t *hmap, 
    int update_type,
    KEYTYPE *keys, // [nkeys] 
    uint32_t *hashes, // [nkeys]
    VALTYPE *vals, // [nkeys] 
    uint32_t nkeys,
    uint8_t *is_founds // [nkeys bits]
    );
#endif
