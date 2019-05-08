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

#define __KEYTYPE__ uint64_t
#define __VALTYPE__  int64_t

typedef struct _rh_bucket_t {
	__KEYTYPE__  key; 
	__VALTYPE__ val;
	uint64_t	hash	: 32;
	uint64_t	psl	: 16;
} rh_bucket_t;

typedef struct _q_rhashmap_t {
	unsigned	size;
	unsigned	nitems;
	unsigned	flags;
	uint64_t	divinfo;
	rh_bucket_t *	buckets;
	uint64_t	hashkey;
	unsigned	minsize;
} q_rhashmap_t;

extern q_rhashmap_t *	
q_rhashmap_create(
    size_t, 
    unsigned
    );
extern void		
q_rhashmap_destroy(
    q_rhashmap_t *
    );

extern __VALTYPE__
q_rhashmap_get(
    q_rhashmap_t *, 
    __KEYTYPE__ key
    );
extern __VALTYPE__
q_rhashmap_put(
    q_rhashmap_t *, 
    __KEYTYPE__ key,
    __VALTYPE__ val
    );
extern __VALTYPE__
q_rhashmap_del(
    q_rhashmap_t *, 
    __KEYTYPE__ key
    );

#endif
