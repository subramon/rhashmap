return  require 'Q/UTILS/lua/code_gen' { 
  declaration = [[
#ifndef __HMAP_TYPE_H
#define __HMAP_TYPE_H

typedef ${ckeytype} keytype;
typedef ${ccnttype} cnttype;

typedef struct _val_t { 
  ${spec_for_vals}
} val_t;

typedef struct _bkt_t { 
  ${ckeytype} key; // keys
  uint16_t psl; // probe sequence length 
  ${ccnttype} cnt; // count number of times key was inserted
  val_t val;
} bkt_t;

typedef struct _hmap_t {
  uint32_t size;
  uint32_t nitems;
  uint64_t divinfo;
  bkt_t  *bkts;  
  uint64_t hashkey;
  uint32_t minsize;
} hmap_t;

#endif
  ]]
}
