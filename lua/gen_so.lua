local libgen = require 'libgen'
local config = assert(arg[1])
local T      = require (config)
assert(type(T) == "table")
libgen(T)

