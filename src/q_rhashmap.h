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
#define KV I8_I8
#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define NAME(fun) EVALUATOR(fun, KV)

#define bucket_type   rh_bucket_I8_I8_t
#define hashmap_type rhashmap_I8_I8_t

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
NAME(q_rhashmap_create)(
    size_t initial_size
    );
extern void		
NAME(q_rhashmap_destroy)(
    hashmap_type *
    );

extern __VALTYPE__
NAME(q_rhashmap_get)(
    hashmap_type *, 
    __KEYTYPE__ key
    );
extern __VALTYPE__
NAME(q_rhashmap_put)(
    hashmap_type *, 
    __KEYTYPE__ key,
    __VALTYPE__ val
    );
extern __VALTYPE__
NAME(q_rhashmap_del)(
    hashmap_type *, 
    __KEYTYPE__ key
    );

#endif
