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
  local subs = {}
  if ( not plpath.isdir(srcdir) ) then plpath.mkdir(srcdir) end
  if ( not plpath.isdir(incdir) ) then plpath.mkdir(incdir) end
  --=====================================
  assert(type(T) == "table")
  local keytype = assert(T.keytype)
  assert( ( keytype == "I4" ) or ( keytype == "I8" ) ) 
  subs.ckeytype = "u" .. assert(qconsts.qtypes[keytype].ctype)
  --=====================================
  local vals = assert(T.vals)
  assert(type(vals) == "table")
  local num_vals = #vals
  -- 4 is just an arbitrary but hopefully reasonable limit
  assert( ( num_vals >= 1 ) and ( num_vals <= 4 ) ) 
  --=====================================
  local cnttype = "I4"
  if ( T.cnttype ) then 
    assert( (cnttype == "I4" ) or (cnttype == "I8" ) )
  end
  subs.ccnttype = "u" .. assert(qconsts.qtypes[cnttype].ctype)
  --=====================================
  local aggstype  = {}
  local valstype = {}
  for i, v in pairs(vals) do
    local valtype  = assert(v.valtype)
    assert( ( valtype == "I1" ) or ( valtype == "I2" )  or
            ( valtype == "I4" ) or ( valtype == "I8" )  or
            ( valtype == "F4" ) or ( valtype == "F8" ) ) 
    local aggtype    = assert(v.aggtype)
    --====================
    valstype[i]   = valtype
    aggstype[i]   = aggtype
    --====================
  end
  subs.code_for_update  = "XXXXXX" 
  subs.code_for_promote = "YYYYYY" 
  -- START generating code
  -- 1) hmap_type.h
  -- set common stuff
  -- notice that key is unsigned
  --================================
  local X = {}
  X[#X+1] = "// start update "
  for i = 1, num_vals do 
    local si = tostring(i)
    local dst = "bkts[probe_loc].val.val_" .. si 
    local src = " val.val_" ..  si 
    if ( aggstype[i] == "set" ) then 
      X[#X+1] = dst .. " = " .. src 
    elseif ( aggstype[i] == "sum" ) then 
      X[#X+1] = dst .. " += " .. src 
    elseif ( aggstype[i] == "cnt" ) then 
      X[#X+1] = dst .. " += 1 " 
    elseif ( aggstype[i] == "min" ) then 
      X[#X+1] = dst .. " = min(" .. dst .. "," .. src .. ")"
    elseif ( aggstype[i] == "max" ) then 
      X[#X+1] = dst .. " = max(" .. dst .. "," .. src .. ")"
    else
      assert(nil, "Unknown aggtype = ", aggstype[1])
    end
  end
  X[#X+1] = "// stop update "
  subs.code_for_update = table.concat(X, ";\n");
  --=============================
  local X = {}
  for i = 1, num_vals do 
    local cvaltype = assert(qconsts.qtypes[valstype[i]].ctype)
    X[#X+1] = cvaltype .. " val_" .. tostring(i) .. ";"
  end
  subs.spec_for_vals = table.concat(X, "\n");
  --=============================
  local X = {}
  for i = 1, num_vals do 
    local si = tostring(i)
    X[#X+1] = "if ( v1.val_" .. si .. " != v2.val_" .. si .. " ) { return false; } "
  end
  subs.val_cmp_spec = table.concat(X, "\n");
  --=============================
  -- 0) hmap_eq.[c,h]
  subs.tmpl = "hmap_eq.tmpl.lua"
  subs.fn = "hmap_eq"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 1) hmap_type.h
  subs.tmpl = "hmap_type.tmpl.lua"
  subs.fn = "hmap_types"
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
  -- 5) hmap_insert.[c, h]
  subs.fn = "hmap_insert"
  subs.tmpl = "hmap_insert.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 6) hmap_get.[c, h]
  subs.fn = "hmap_get"
  subs.tmpl = "hmap_get.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 7) hmap_put.[c, h]
  subs.fn = "hmap_put"
  subs.tmpl = "hmap_put.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 8) hmap_del.[c, h]
  subs.fn = "hmap_del"
  subs.tmpl = "hmap_del.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
  -- 9) hmap_chk_no_holes.[c, h]
  subs.fn = "hmap_chk_no_holes"
  subs.tmpl = "hmap_chk_no_holes.tmpl.lua"
  gen_code.doth(subs, incdir); gen_code.dotc(subs, srcdir)
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
  X[#X+1] = srcdir .. "_hmap_eq.c" 
  X[#X+1] = srcdir .. "_hmap_mk_hash.c" 
  X[#X+1] = srcdir .. "_hmap_create.c" 
  X[#X+1] = srcdir .. "_hmap_resize.c" 
  X[#X+1] = srcdir .. "_hmap_chk_no_holes.c" 
  X[#X+1] = srcdir .. "_hmap_insert.c" 
  X[#X+1] = srcdir .. "_hmap_put.c" 
  X[#X+1] = srcdir .. "_hmap_get.c" 
  X[#X+1] = srcdir .. "_hmap_del.c" 
  create_dot_o(X)
  X[#X+1] = " "
  local command = "gcc -shared *.o  -o " .. libname 
  os.execute(command)
  assert(status == 0)

end
return libgen
