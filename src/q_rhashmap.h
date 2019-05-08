/*
 * Copyright (c) 2017 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _RHASHMAP_H_
#define _RHASHMAP_H_

#define	RHM_NOCOPY		0x01
#define	RHM_NONCRYPTO		0x02

#define concat2(x, y) x ## y 
#define concat3(x, y, z) x ## y ## z
#define __KEYTYPE__ uint64_t
#define __VALTYPE__  int64_t
#define KV concat2(__KEYTYPE__, __VALTYPE__)

#define bucket_type concat3(q_rh_bucket_, KV, _t)
#define hashmap_type concat3(q_rhashmap__, KV, _t)

typedef struct {
	__KEYTYPE__  key; 
	__VALTYPE__ val;
	uint64_t	hash	: 32;
	uint64_t	psl	: 16;
} bucket_type;

typedef struct {
	unsigned	size;
	unsigned	nitems;
	unsigned	flags;
	uint64_t	divinfo;
	bucket_type *	buckets;
	uint64_t	hashkey;
	unsigned	minsize;
} hashmap_type;

extern hashmap_type *	
q_rhashmap_create(
    size_t, 
    unsigned
    );
extern void		
q_rhashmap_destroy(
    hashmap_type *
    );

extern __VALTYPE__
q_rhashmap_get(
    hashmap_type *, 
    __KEYTYPE__ key
    );
extern __VALTYPE__
q_rhashmap_put(
    hashmap_type *, 
    __KEYTYPE__ key,
    __VALTYPE__ val
    );
extern __VALTYPE__
q_rhashmap_del(
    hashmap_type *, 
    __KEYTYPE__ key
    );

#endif
