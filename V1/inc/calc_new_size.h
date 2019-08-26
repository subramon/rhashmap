extern int
calc_new_size(
    uint32_t nitems, 
    uint32_t minsize, 
    uint32_t size, 
    bool decreasing, 
    /* true =>  just added an element and are concerned about sparsity
     * false=> just added an element and are concerned about denseness
    */
    uint32_t *ptr_newsize,
    bool *ptr_resize
    );
