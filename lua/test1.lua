local libgen = require 'libgen'
local T = {}
T.keytype = "I8"
local vals = {}
local x = { read_as = "I4", agg_as = "I8", agg_type = "sum" }
local y = { read_as = "I1", agg_type = "min" }
local z = { read_as = "I2", agg_type = "cnt" }
vals[#vals+1] = x
vals[#vals+1] = y
vals[#vals+1] = z
T.vals = vals
libgen(T, "libfoobar.so")
