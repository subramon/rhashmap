local libgen = require 'libgen'
local T = {}
T.keytype = "I8"
T.cnttype = "I4" -- range needed to count keys
local vals = {}
local x = { valtype = "F4", aggtype = "sum" }
local y = { valtype = "I1", aggtype = "min" }
local z = { valtype = "I2", aggtype = "max" }
local w = { valtype = "I4", aggtype = "set" }
vals[#vals+1] = x
vals[#vals+1] = y
vals[#vals+1] = z
vals[#vals+1] = w
T.vals = vals
libgen(T, "libtest1.so")
