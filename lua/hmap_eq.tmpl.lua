return  require 'Q/UTILS/lua/code_gen' { 
declaration = [[ 
#include "hmap_common.h"
#include "_hmap_types.h"
extern bool 
${fn}(
    val_t v1,
    val_t v2
    );
    ]],
definition = [[
#include "_${fn}.h"
bool 
${fn}(
    val_t v1,
    val_t v2
    )
{
  ${val_cmp_spec}
  return true;
}
]],
}

