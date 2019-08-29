#ifndef __TYPES_H
#define __TYPES_H
typedef struct {
	uint32_t size;
	uint32_t nitems;
	uint64_t divinfo;
	void     *buckets;
	uint64_t hashkey;
	uint32_t minsize;
} q_rhashmap_t;

#endif
