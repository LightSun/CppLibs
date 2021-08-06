
local function loadfile2(f_str)
	local s, msg = loadfile(f_str)
	if(msg) then 
		print("load file failed. ", f_str, msg);
	end
	return s;
end

require "test.longSize"
require "test.test_aliasMultinomial"
--require "my_test.test_ffi"
require "my_test.test_file"
require "libs.platform"
require "libs.ffi_util"
require "libs.ffi_ffmpeg"
--require "my_test.test_ffi_stest"
require "my_test.test_ffmpeg_decode_video"



