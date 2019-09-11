local libgen = require 'libgen'
local T = {}
T.keytype = "I8"
local vals = {}
local x = { read_as = "F4", agg_as = "F8", agg_type = "sum" }
vals[#vals+1] = x
T.vals = vals
libgen(T, "libfoobar.so")
