local cwrap = {}

cwrap.types = require '_types'
cwrap.CInterface = require 'cinterface'
cwrap.CInterface.argtypes = cwrap.types

return cwrap
