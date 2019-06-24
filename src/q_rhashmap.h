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
#include "macros.h"

#define	Q_RHM_SET  1
#define	Q_RHM_ADD 2

#define KEYTYPE uint64_t
#define VALTYPE  int64_t

#include "q_rhashmap_struct.h"

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
    q_rhashmap_t *hmap, 
    KEYTYPE key,
    VALTYPE *ptr_val,
    bool *ptr_is_found
    );
extern int
q_rhashmap_put(
    q_rhashmap_t *hmap, 
    KEYTYPE key,
    VALTYPE val,
    int update_type,
    VALTYPE *ptr_oldval,
    int *ptr_num_probes
    );
extern int
q_rhashmap_del(
    q_rhashmap_t *hmap, 
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
q_rhashmap_mk_hash(
    KEYTYPE *keys, // input  [nkeys] 
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
q_rhashmap_putn(
    q_rhashmap_t *hmap,  // INPUT
    int update_type, // INPUT
    KEYTYPE *keys, // INPUT [nkeys] 
    uint32_t *hashes, // INPUT [nkeys]
    uint32_t *locs, // INPUT [nkeys]
    uint8_t *tids, // INPUT [nkeys]
    int nT,
    VALTYPE *vals, // INPUT [nkeys] 
    uint32_t nkeys, // INPUT
    uint8_t *is_founds // OUTPUT [nkeys bits] TODO: Change from byte to bit 
    );
#endif
