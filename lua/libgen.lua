local gen_code = require "Q/UTILS/lua/gen_code"
local qconsts  = require 'Q/UTILS/lua/q_consts'
local extract_func_decl  = require 'Q/UTILS/lua/extract_func_decl'
local plpath   = require "pl.path"
local srcdir   = "../xgen_src/"
local incdir   = "../xgen_inc/"

local function create_dot_o(
  X
  )
  local incs = " -I../inc/ -I../xgen_inc/ "
  assert(type(X) == "table")
  for k, v in pairs(X) do 
    local command = "gcc -c "  .. qconsts.QC_FLAGS .. incs .. v
    status = os.execute(command)
    if ( status ~= 0 ) then print(command) end 
    assert(status == 0)
  end
end


local function libgen(
  T,
  libname
  )
  if ( not plpath.isdir(srcdir) ) then plpath.mkdir(srcdir) end
  if ( not plpath.isdir(incdir) ) then plpath.mkdir(incdir) end
  assert(type(T) == "table")
  local keytype = assert(T.keytype)
  assert( ( keytype == "I4" ) or ( keytype == "I8" ) ) 

  local vals = assert(T.vals)
  assert(type(vals) == "table")
  local num_vals = #vals
  invalstype  = {}
  aggvalstype = {}
  aggstype    = {}
  assert( ( num_vals >= 1 ) and ( num_vals <= 4 ) ) 
  -- 4 is just an arbitrary but hopefully reasonable limit

  for i, v in pairs(vals) do
    local invaltype  = assert(v.read_as)
    assert( ( invaltype == "I1" ) or ( invaltype == "I2" )  or
            ( invaltype == "I4" ) or ( invaltype == "I8" )  or
            ( invaltype == "F4" ) or ( invaltype == "F8" ) ) 
    local aggtype    = assert(v.agg_type)
    local aggvaltype = v.agg_as -- note this is optional
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
      if ( not aggvaltype ) then aggvaltype = invaltype end
      print(invaltype, aggvaltype)
      assert( ( aggvaltype == "I4" ) or ( aggvaltype == "I8" ) ) 
    elseif ( aggtype == "sum" ) then 
      -- all is well 
    else
      assert(nil, "invalid aggtype")
    end
    if ( not aggvaltype ) then aggvaltype = invaltype end
    --====================
    invalstype[i]  = invaltype 
    aggvalstype[i] = aggvaltype
    aggstype[i]    = aggtype
    --====================
  end
  -- START generating code
  -- 1) hmap_type.h
  local subs = {}
  -- set common stuff
  -- notice that key is unsigned
  subs.ckeytype = "u" .. assert(qconsts.qtypes[keytype].ctype)
  if ( num_vals == 1 ) then 
    subs.cinvaltype  = assert(qconsts.qtypes[invalstype[1]].ctype) 
    subs.caggvaltype = assert(qconsts.qtypes[aggvalstype[1]].ctype)

    subs.agg_val_rec = " ";
    subs.in_val_rec = " ";

  else
    subs.caggvaltype = "AGG_VAL_REC_TYPE";
    subs.cinvaltype  = "IN_VAL_REC_TYPE";

    local X = {}
    X[#X+1] = "typedef struct _in_val_rec_type { ";
    for i = 1, num_vals do 
      local cvaltype = assert(qconsts.qtypes[invalstype[i]].ctype)
      X[#X+1] = "  " .. cvaltype .. " val_" .. tostring(i) .. ";" ;
    end
    X[#X+1] = " } IN_VAL_REC_TYPE ; ";
    subs.in_val_rec = table.concat(X, "\n");

    local X = {}
    X[#X+1] = "typedef struct _agg_val_rec_type { ";
    for i = 1, num_vals do 
      local cvaltype = assert(qconsts.qtypes[aggvalstype[i]].ctype)
      X[#X+1] = "  " .. cvaltype .. " val_" .. tostring(i) .. ";" ;
    end
    X[#X+1] = " } AGG_VAL_REC_TYPE ; ";
    subs.agg_val_rec = table.concat(X, "\n");

  end
  subs.tmpl = "hmap_type.tmpl.lua"
  subs.fn = "hmap_types"
  subs.NUM_VALS = num_vals
  gen_code.doth(subs, incdir)
  -- 2) hmap_mk_hash.[c, h]
  subs.tmpl = "hmap_mk_hash.tmpl.lua"
  subs.fn = "hmap_mk_hash"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 3) hmap_create.[c, h]
  subs.fn = "hmap_create"
  subs.tmpl = "hmap_create.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 4) hmap_resize.[c, h]
  subs.fn = "hmap_resize"
  subs.tmpl = "hmap_resize.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 5) hmap_find_loc.[c, h]
  subs.fn = "hmap_find_loc"
  subs.tmpl = "hmap_find_loc.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  --[[
  -- 6) hmap_get.[c, h]
  subs.fn = "hmap_get"
  subs.tmpl = "hmap_get.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 7) hmap_get.[c, h]
  subs.fn = "hmap_put"
  subs.tmpl = "hmap_put.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  --]]
  -- identify files from src/ folder and generate .h files for them
  local S = {}
  S[#S+1] = "../src/hmap_mk_loc.c"
  S[#S+1] = "../src/hmap_mk_tid.c"
  S[#S+1] = "../src/validate_psl_p.c"
  S[#S+1] = "../src/calc_new_size.c"
  S[#S+1] = "../src/murmurhash.c"
  S[#S+1] = "../src/hmap_destroy.c"
  for k, v in ipairs(S) do 
    extract_func_decl(v, incdir)
  end

  -- compile code 
  local X = {}
  for k, v in ipairs(S) do 
    X[#X+1] = S[k]
  end
  X[#X+1] = srcdir .. "_hmap_mk_hash.c" 
  X[#X+1] = srcdir .. "_hmap_create.c" 
  X[#X+1] = srcdir .. "_hmap_resize.c" 
  X[#X+1] = srcdir .. "_hmap_find_loc.c" 
  -- X[#X+1] = srcdir .. "_hmap_get.c" 
  -- X[#X+1] = srcdir .. "_hmap_put.c" 
--  X[#X+1] = srcdir .. "_hmap_del.c" 
  create_dot_o(X)
  X[#X+1] = " "
  local command = "gcc -shared *.o  -o " .. libname 
  os.execute(command)
  assert(status == 0)

end
return libgen
