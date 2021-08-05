
local ffi = require("ffi")

ffi.cdef[[
typedef int* int_ptr;
int stest_parse(int** a, long long val);
]]

local cur_dir = assert ( debug.getinfo ( 1 , "S" ).source:match ( [=[^@(.-[/\]?)[^/\]*$]=] ) , "Current directory unknown" )
local stest = ffi.load(cur_dir.."/../../../test_res/gcc/libstest")
print("load libstest.dll ok.", stest)

local s = ffi.new("int**[1]");
ffi.debug(s)
local AV_NOPTS_VALUE = ffi.new("int64_t[1]")
AV_NOPTS_VALUE[0] = ffi.i64(0x8000000000000000);

print("before: call stest_parse(): s = ", s[0])
local res = stest.stest_parse(s[0], 1);
print("res: ", res)
print("s: ", s[0])