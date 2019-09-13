local libgen = require 'libgen'
local T = {}
T.keytype = "I8"
-- T.cnttype = "I8"
local vals = {}
local x = { valtype = "F8", aggtype = "sum" }
vals[#vals+1] = x
T.vals = vals
libgen(T, "libtest2.so")
