
local ffi = require("ffi")

ffi.cdef[[
typedef int* int_ptr;
int stest_parse(int** a, long long val);
]]

local cur_dir = assert ( debug.getinfo ( 1 , "S" ).source:match ( [=[^@(.-[/\]?)[^/\]*$]=] ) , "Current directory unknown" )
local stest = ffi.load(cur_dir.."/../../../test_res/gcc/libstest")
print("load libstest.dll ok.", stest)

local s = ffi.new("int*[1]");
local ptr = ffi.new("int[1]")
ptr[0] = 123
s[0] = ptr

local AV_NOPTS_VALUE = ffi.new("int64_t", 0x8000000000000000)
--AV_NOPTS_VALUE[0] = ffi.i64(0x8000000000000000);
--print("AV_NOPTS_VALUE: ",ffi.string(AV_NOPTS_VALUE[0]))

print("before: call stest_parse(): s = ", s)
local res = stest.stest_parse(s, AV_NOPTS_VALUE);
print("res: ", res)
print("s: ", s, s[0], s[0][0])