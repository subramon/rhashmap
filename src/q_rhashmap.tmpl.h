/*
 * Copyright (c) 2019 Ramesh Subramonian <subramonian@gmail.com>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _Q_RHASHMAP___KV___H_
#define _Q_RHASHMAP___KV___H_

#include "q_rhashmap_common.h"
#include "_q_rhashmap_struct___KV__.h"

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
extern q_rhashmap___KV___t *
q_rhashmap_create___KV__(
      size_t size
        );
extern void
q_rhashmap_destroy___KV__(
    q_rhashmap___KV___t *ptr_hmap
    );
#endif
