local libgen = require 'libgen'
local T = {}
T.keytype = "I4"
local vals = {}
local x = { read_as = "I4", agg_as = "I8", agg_type = "sum" }
vals[#vals+1] = x
T.vals = vals
libgen(T, "libfoobar.so")
