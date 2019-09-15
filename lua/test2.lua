local libgen = require 'libgen'
local T = {}
T.keytype = "I8"
-- T.cnttype = "I8"
local vals = {}
local x = { valtype = "F8", aggtype = "sum" }
vals[#vals+1] = x
T.vals = vals
T.so = "libtest2.so"
T.lbl = "test2"
return T
-- libgen(T)
