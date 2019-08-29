return  require 'Q/UTILS/lua/code_gen' { 
  declaration = [[
#ifndef __CELL_TYPE_${qkeytype}_H
#define __CELL_TYPE_${qkeytype}_H
    typedef struct {
      ${ckeytype}  key; 
#ifdef DEBUG
      uint32_t hash;
#endif
      uint16_t psl; // TODO P4: Confirm this is enough
    } q_rh_bucket_${qkeytype}_t;;


#endif
  ]]
}
