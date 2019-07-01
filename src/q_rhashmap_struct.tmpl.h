#ifndef _Q_RHASHMAP_STRUCT___KV___H_
#define _Q_RHASHMAP_STRUCT___KV___H_
typedef struct {
	__KEYTYPE__  key; 
	__VALTYPE__ val;
        // TODO P4: Think through whether hash should be 64 bit
	uint32_t hash;
	uint16_t psl; // TODO P4: Confirm this is enough
} q_rh_bucket___KV___t;

typedef struct {
	uint32_t	size;
	uint32_t	nitems;
	uint64_t	divinfo;
	q_rh_bucket___KV___t *	buckets;
	uint64_t	hashkey;
	uint32_t	minsize;
} q_rhashmap___KV___t;

#endif
