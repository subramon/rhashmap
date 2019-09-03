local gen_code = require "Q/UTILS/lua/gen_code"
local qconsts  = require 'Q/UTILS/lua/q_consts'
local plpath   = require "pl.path"
local srcdir   = "../xgen_src/"
local incdir   = "../xgen_inc/"

local function libgen(
  T,
  libname
  )
  if ( not plpath.isdir(srcdir) ) then plpath.mkdir(srcdir) end
  if ( not plpath.isdir(incdir) ) then plpath.mkdir(incdir) end
  assert(type(T) == "table")
  local keytype = assert(T.keytype)
  assert( ( keytype == "I4" ) or ( keytype == "I8" ) ) 

  local tgt = assert(T.tgt)
  assert(type(tgt) == "table")
  local num_vals = #tgt
  invalstype  = {}
  aggvalstype = {}
  aggstype    = {}
  assert( ( num_vals >= 1 ) and ( num_vals <= 4 ) ) 
  -- 4 is just an arbitrary but hopefully reasonable limit

  for i, v in pairs(tgt) do
    local invaltype  = assert(v.invaltype)
    assert( ( invaltype == "I1" ) or ( invaltype == "I2" )  or
            ( invaltype == "I4" ) or ( invaltype == "I8" )  or
            ( invaltype == "F4" ) or ( invaltype == "F8" ) ) 

    local aggtype    = assert(v.aggtype)
    local aggvaltype = v.aggvaltype -- note this is optional
    if ( aggvaltype ) then 
      assert( ( aggvaltype == "I1" ) or ( aggvaltype == "I2" )  or
            ( aggvaltype == "I4" ) or ( aggvaltype == "I8" )  or
            ( aggvaltype == "F4" ) or ( aggvaltype == "F8" ) ) 
    end
    --====================
    if ( ( aggtype == "set" ) or ( aggtype == "min" ) or
         ( aggtype == "max" ) ) then 
      assert(not aggvaltype) -- should not be specified
      aggvaltype = invaltype 
    elseif ( aggtype == "cnt" ) then 
      assert( ( aggvaltype == "I4" ) or ( aggvaltype == "I8" ) ) 
    elseif ( aggtype == "sum" ) then 
      -- all is well 
    else
      assert(nil, "invalid aggtype")
    end
    if ( not aggvaltype ) then aggvaltype = invaltype end
    --====================
    invalstype[i]  = valtype 
    aggvalstype[i] = aggvaltype
    aggstype[i]    = aggtype
    --====================
  end
  -- START generating code
  -- 1) hmap_type.h
  local subs = {}
  subs.tmpl = "hmap_type.tmpl.lua"
  -- notice that key is unsigned
  subs.ckeytype = "u" .. assert(qconsts.qtypes[keytype].ctype)
  subs.NUM_VALS = num_vals
  subs.fn = "hmap_types"
  if ( num_vals == 1 ) then 
    cvaltype = assert(qconsts.qtypes[aggvalstype[1]].ctype)
    subs.val_spec = cvaltype .. " *vals; "
    subs.val_rec = " ";
  else 
    local X = {}
    X[#X+1] = "typedef struct _val_rec_type { ";
    for i = 1, num_vals do 
      local cvaltype = assert(qconsts.qtypes[aggvalstype[i]].ctype)
      X[#X+1] = "  " .. cvaltype .. " val_" .. tostring(i) .. ";" ;
    end
    X[#X+1] = " } VAL_REC_TYPE ; ";
    subs.val_rec = table.concat(X, "\n");
    subs.val_spec = "VAL_REC_TYPE *vals; "
  end
  gen_code.doth(subs, incdir)
  -- 2) hmap_mk_hash.h
  local subs = {}
  subs.fn = "hmap_mk_hash"
  subs.tmpl = "hmap_mk_hash.tmpl.lua"
  -- notice that key is unsigned
  subs.ckeytype = "u" .. assert(qconsts.qtypes[keytype].ctype)
  gen_code.doth(subs, incdir)
  gen_code.dotc(subs, srcdir)
  -- compile code 
  local incs = "-I../inc/ -I../xgen_inc/"
  local X = {}
  X[#X+1] = " "
  X[#X+1] = "../src/murmurhash.c"
  X[#X+1] = srcdir .. "_hmap_mk_hash.c" 
  X[#X+1] = " "
  local command = "gcc -shared " .. qconsts.QC_FLAGS .. incs .. 
    " -o " .. libname 
    .. table.concat(X, " ") 
  print(command)
  os.execute(command)

end
return libgen
