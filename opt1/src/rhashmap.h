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

typedef struct _rh_bucket_t {
	void *		key;
	void *		val;
	uint64_t	hash	: 32;
	uint64_t	psl	: 16;
	uint64_t	len	: 16;
} rh_bucket_t;

typedef struct _rhashmap_t {
	unsigned	size;
	unsigned	nitems;
	unsigned	flags;
	uint64_t	divinfo;
	rh_bucket_t *	buckets;
	uint64_t	hashkey;
	unsigned	minsize;
} rhashmap_t;

extern rhashmap_t *	
rhashmap_create(
    size_t, 
    unsigned
    );
extern void		
rhashmap_destroy(
    rhashmap_t *
    );

extern void *		
rhashmap_get(
    rhashmap_t *, 
    const void *, 
    size_t
    );
extern void *		
rhashmap_put(
    rhashmap_t *, 
    const void *, 
    size_t, 
    void *
    );
extern void *		
rhashmap_del(
    rhashmap_t *, 
    const void *, 
    size_t
    );

#endif
