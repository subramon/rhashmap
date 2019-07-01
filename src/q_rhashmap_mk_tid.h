extern int 
q_rhashmap_mk_tid(
    uint32_t *hashes, // input  [nkeys] 
    uint32_t nkeys, // input 
    uint32_t nT, // input , number of threads
    uint8_t *tids // output [nkeys] 
    );
