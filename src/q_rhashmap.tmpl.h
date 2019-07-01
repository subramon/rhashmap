/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _Q_RHASHMAP_H_
#define _Q_RHASHMAP_H_

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "macros.h"
#include "fastdiv.h"
#include "macros.h"

#define	Q_RHM_SET  1
#define	Q_RHM_ADD 2

#define	HASH_INIT_SIZE		(65536)
#define	MAX_GROWTH_STEP		(1024U * 1024)

#define	LOW_WATER_MARK 0.4
#define	HIGH_WATER_MARK 0.85

#include "_q_rhashmap_struct.h"

extern q_rhashmap___KV___t *	
q_rhashmap_create(
    size_t initial_size
    );
extern void		
q_rhashmap_destroy(
    q_rhashmap___KV___t *
    );

extern int
q_rhashmap_get___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ *ptr_val,
    bool *ptr_is_found
    );
extern int
q_rhashmap_put___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ val,
    int update_type,
    __VALTYPE__ *ptr_oldval,
    int *ptr_num_probes
    );
extern int
q_rhashmap_del___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__ key,
    __VALTYPE__ *ptr_oldval,
    bool *ptr_is_found
    );

extern int 
q_rhashmap_getn___KV__(
    q_rhashmap___KV___t *hmap, 
    __KEYTYPE__  *keys, // [nkeys] 
    uint32_t *hashes, // [nkeys] 
    uint32_t *locs, // [nkeys] 
    __VALTYPE__  *vals, // [nkeys] 
    uint32_t nkeys
    );

extern int 
q_rhashmap_mk_hash___KTYPE__(
    __KEYTYPE__ *keys, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint64_t hmap_hashkey, // input 
    uint32_t *hashes// output 
    );
extern int 
q_rhashmap_mk_loc(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t hmap_size, // input 
    uint32_t *locs // [nkeys] 
    );
extern int 
q_rhashmap_mk_tid(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t nT, // input , number of threads
    uint8_t *tids // output [nkeys] 
    );
extern int 
q_rhashmap_putn___KV__(
    q_rhashmap___KV___t *hmap,  // INPUT
    int update_type, // INPUT
    __KEYTYPE__ *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys]
    uint8_t *tids, // INPUT [nkeys]
    int nT,
    __VALTYPE__ *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *is_founds // OUTPUT [nkeys bits] TODO: Change from byte to bit 
    );
#endif
