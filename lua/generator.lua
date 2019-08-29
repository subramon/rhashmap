local gen_code = require "Q/UTILS/lua/gen_code"
local qconsts  = require 'Q/UTILS/lua/q_consts'
local plpath   = require "pl.path"
local srcdir   = "../gen_src/"
local incdir   = "../gen_inc/"
if ( not plpath.isdir(srcdir) ) then plpath.mkdir(srcdir) end
if ( not plpath.isdir(incdir) ) then plpath.mkdir(incdir) end

local keytypes = { 'I4', 'I8', }

local num_produced = 0
for _, qkeytype in ipairs(keytypes) do 
  local subs = {}
  subs.fn = "cell_type_" .. qkeytype
  subs.tmpl = "cell_type.tmpl.lua"
  -- notice that key is unsigned
  subs.ckeytype = "u" .. assert(qconsts.qtypes[qkeytype].ctype)
  subs.qkeytype = qkeytype
  gen_code.doth(subs, incdir)
  num_produced = num_produced + 1
end
assert(num_produced > 0)
