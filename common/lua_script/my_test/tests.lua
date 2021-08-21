
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
require "my_test.test_storage"
require "my_test.test_tensor"

require "libs.platform"
require "libs.ffi_util"


--require "my_test.test_ffi_stest"

--test ffi we ffmpeg failed by some unknown cause.(blocking in some method)
--require "libs.ffi_ffmpeg"
--require "my_test.test_ffmpeg_decode_video"


require "nn.test"
-- require "test.nn.LinearTHNN"
-- require "test.nn.benchmarks.IndexLinear"

require "my_test.test_nn"
require "optim"
require "sys"


