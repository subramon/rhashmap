return  require 'Q/UTILS/lua/code_gen' { 
  declaration = [[
#ifndef __HMAP_TYPE_H
#define __HMAP_TYPE_H

${val_rec}
${in_val_rec}

#define NUM_VALS ${NUM_VALS}

typedef struct {
  uint32_t size;
  uint32_t nitems;
  uint64_t divinfo;
  ${ckeytype}  *keys;  // keys
  ${cvaltype} *vals; // values 
  uint16_t *psls; // probe sequence length 
  uint64_t hashkey;
  uint32_t minsize;

} hmap_t;

#endif
  ]]
}
