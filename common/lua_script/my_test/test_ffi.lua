local ffi = require("ffi")
ffi.cdef[[
typedef struct { uint8_t red, green, blue, alpha; } rgba_pixel;
]]

local function image_ramp_green(n)
  local img = ffi.new("rgba_pixel[?]", n)
  local f = 255/(n-1)
  for i=0,n-1 do
    img[i].green = i*f
    img[i].alpha = 255
  end
  print("img: ", img)
  return img
end

local function image_to_grey(img, n)
  for i=0,n-1 do
    local y = 0.3*img[i].red + 0.59*img[i].green + 0.11*img[i].blue
    img[i].red = y; img[i].green = y; img[i].blue = y
  end
end

local N = 10*10
local img = image_ramp_green(N)
for i=1,1000 do
  image_to_grey(img, N)
end

--local ffi = require("ffi")
ffi.cdef[[
int printf(const char *fmt, ...);
]]
ffi.C.printf("Hello %s!", "world\r\n")

print("lua ffi test end.")