--[[
ByteTensor -- contains unsigned chars
CharTensor -- contains signed chars
ShortTensor -- contains shorts
IntTensor -- contains ints
LongTensor -- contains longs
FloatTensor -- contains floats
DoubleTensor -- contains doubles

All tensor operations in this class do not make any memory copy
]]--
require "torch"

 --- creation of a 4D-tensor 4x5x6x2
 z = torch.Tensor(4,5,6,2)
 --- for more dimensions, (here a 6D tensor) one can do:
 s = torch.LongStorage(6)
 s[1] = 4; s[2] = 5; s[3] = 6; s[4] = 2; s[5] = 7; s[6] = 3;
 
 x = torch.Tensor(s) -- default double tensor
 --print("s: ", s)
 --print("x: ", x) -- 4 * 5 * 6 * 2 * 7 * 3
 print("x dim:", x:nDimension()) --维数
 --  'x[3][4][5])' equal to 'x:storage()[x:storageOffset() +(3-1)*x:stride(1)+(4-1)*x:stride(2)+(5-1)*x:stride(3)]'
 

--  storage from tensor
x = torch.Tensor(4,5)
s = x:storage()
for i=1,s:size() do -- fill up the Storage
  s[i] = i  -- start from 1
end
--print("storage from tensor:", x) -- 4*5 = 4 rows * 5 columns

x = torch.Tensor(4,5)
i = 0
x:apply(function()
  i = i + 1
  return i
end)

-- print("apply tensor:", x)
-- stride return last dim of storage.
print("apply tensor stride:", x:stride()) -- 5 ,1 [torch.LongStorage of size 2]

-- default tensor type is double. you can set other
torch.setdefaulttensortype('torch.FloatTensor')


x = torch.Tensor(5):zero()
x:narrow(1, 2, 3):fill(1)   -- narrow() returns a Tensor (from index 1 to 3)
                            -- referencing the same Storage as x
-- print("narrow: ",x)

-- copy tensor
y = torch.Tensor(x:size()):copy(x)
y = x:clone()

-- create new tensor with same memory of x. no memory copy.
y = torch.Tensor(x);

-- create 4d rensor. 4*4*3*2
x = torch.Tensor(torch.LongStorage({4,4,3,2}))

-------- 'torch.Tensor(sizes, [strides])' --------------
-- strides >= 0 (must)
-- create four size tensor with sample element addr.
x = torch.Tensor(torch.LongStorage({4}), torch.LongStorage({0})):zero() -- zeroes the tensor
x[1] = 1 -- all elements point to the same address!
-- print("all elements point to the same address: ",x)  -- 4 * 1


a = torch.LongStorage({1,2}) -- We have a torch.LongStorage containing the values 1 and 2
-- General case for TYPE ~= Long, e.g. for TYPE = Float:
b = torch.FloatTensor(a)
-- Creates a new torch.FloatTensor with 2 dimensions, the first of size 1 and the second of size 2
-- print("FloatTensor from LongStorage:", b:size()) -- 1 * 2

-- Special case of torch.LongTensor
c = torch.LongTensor(a)
-- Creates a new torch.LongTensor that uses a as storage and thus contains the values 1 and 2
-- print("LongTensor from LongStorage:", c)

-------------- torch.Tensor(storage, [storageOffset, sizes, [strides]]) -----------------
-- creates a storage with 10 elements
s = torch.Storage(10):fill(1)
s[2] = 0
-- s[0]  error. must start from 1

-- we want to see it as a 2x5 tensor. 2 is the start-storageOffset.( from second)
x = torch.Tensor(s, 2, torch.LongStorage{2,5})
-- print("Tensor from Storage:",x)

--------- torch.Tensor(storage, [storageOffset, sz1 [, st1 ... [, sz4 [, st4]]]]) -------------

--------- torch.Tensor(table) --------------
--print("torch.Tensor(table) :",torch.Tensor({{1,2,3,4}, {5,6,7,8}}))  -- 2 * 4

-------------- clone ()--------------
---- [Tensor] clone()-----
i = 0
x = torch.Tensor(5):apply(function(x)
  i = i + 1
  return i
end)
y = x:clone()

---- [Tensor] contiguous --------
x = torch.Tensor(2,3):fill(1)

-- x is contiguous, so y points to the same thing
y = x:contiguous():fill(2)
print("contiguous() y: ",y)
print("contiguous() x: ",x)

-- x:t() is not contiguous, so z is a clone
z = x:t():contiguous():fill(3.14)


---- [Tensor or string] type(type) ---------
print("Tensor type():",torch.Tensor():type()) -- default 'torch.DoubleTensor'. can be change by 'torch.setdefaulttensortype('torch.DoubleTensor')'

-- if same type no memory copy
y = x:type('torch.DoubleTensor')



torch.isTensor(torch.randn(3,4)) -- true
torch.isTensor(torch.randn(3,4)[1]) -- true
torch.isTensor(torch.randn(3,4)[1][2]) -- false (just a number)


------- [Tensor] byte(), char(), short(), int(), long(), float(), double()------
-- cast tensor type.
--- 'x:type('torch.IntTensor')'  = 'x:int()'

--------- [number] nDimension() 维数 -------
--------- [number] dim() 维数 ---------

------- [number] size(dim) -------
-- 返回第几维的 数量----
x = torch.Tensor(4,5):zero() -- 4 * 5
x:size(2) -- 5

--- [LongStorage] size() ----- 
x:size()   --- 4  5  [torch.LongStorage of size 2]

---- [LongStorage] #self ---
-- 同 '[LongStorage] size()'

------ [number] stride(dim) -------
x = torch.Tensor(4,5):zero()
-- elements in a row are contiguous in memory
x:stride(2) -- 1

-- to go from one element to the next one in a column
-- we need here to jump the size of the row
x:stride(1) -- 5
x:stride() -- 返回每个维度的stride 组成 LongStorage 


---------- [Storage] storage() ---------
-- return the storage which used to store the all elements.

--------- [boolean] isContiguous() ------------ 是否是连续的


---------- [boolean] isSize(storage) ---------
x = torch.Tensor(4,5)
y = torch.LongStorage({4,5})
z = torch.LongStorage({5,4,1})
x:isSize(y) -- true;
x:isSize(z) -- false

----------- [boolean] isSameSizeAs(tensor) ----------
x = torch.Tensor(4,5)
y = torch.Tensor(4,5)
x:isSameSizeAs(y)     -- true
y = torch.Tensor(4,6) -- false

---------- [number] nElement() -------------
--Returns the total number of elements of a tensor.
x = torch.Tensor(4,5) -- 4 * 5 = 20

--------- [number] storageOffset() --------
-- starting from 1

---------- Querying elements ----------
x = torch.Tensor(3,3)
i = 0; x:apply(function() i = i + 1; return i end)

-- x[2] -- returns row 2 (LongStorage)
--print(x[2][3]) -- returns row 2, column 3
--print(x[{2,3}]) -- another way to return row 2, column 3
--print(x[torch.LongStorage{2,3}]) -- yet another way to return row 2, column 3


-- 取tensor 的 低5个组成 新tensor
print("torch.le: ",x[torch.le(x,5)]) -- torch.le returns a ByteTensor that acts as a mask


-------------- Referencing a tensor to an existing tensor or chunk of memory --------------
y = torch.Storage(10)
x = torch.Tensor()
x:set(y, 1, 10)
-- same result
y = torch.Storage(10)
x = torch.Tensor(y, 1, 10)

------------ [self] set(tensor) ------------
x = torch.Tensor(2,5):fill(3.14)
y = torch.Tensor():set(x)  --no memory copy


------------ [boolean] isSetTo(tensor) --------------
--Returns true iff the Tensor is set to the argument Tensor. 
--Note: this is only true if the tensors are the same size, have the same strides and share the same storage and offset.
x = torch.Tensor(2,5)
y = torch.Tensor()
--y:isSetTo(x) -- false

y:set(x)
-- y:isSetTo(x) -- true
-- print("y:t():isSetTo(x): ", y:t():isSetTo(x)) -- false -- x and y have different strides


----------- [self] set(storage, [storageOffset, sizes, [strides]]) --------
----------- [self] set(storage, [storageOffset, sz1 [, st1 ... [, sz4 [, st4]]]]) ------------

------- [self] copy(tensor) ------
------- [self] fill(value) -------
------- [self] zero() -----------
------- [self] resizeAs(tensor) --------
------- [self] resize(sizes) -------
------- [self] resize(sz1 [,sz2 [,sz3 [,sz4]]]]) ---------

-- ================== Extracting sub-tensors ======================
-------- [self] narrow(dim, index, size) ---------
-- Returns a new Tensor which is a narrowed version of the current one: t
-- he dimension dim is narrowed from index to index+size-1.
x = torch.Tensor(5, 6):zero()
--[[
> x
0 0 0 0 0 0
0 0 0 0 0 0
0 0 0 0 0 0
0 0 0 0 0 0
0 0 0 0 0 0
[torch.DoubleTensor of dimension 5x6]
 ]]--

y = x:narrow(1, 2, 3) -- narrow dimension 1 from index 2 to index 2+3-1
y:fill(1) -- fill with 1
--[[
> y
 1  1  1  1  1  1
 1  1  1  1  1  1
 1  1  1  1  1  1
[torch.DoubleTensor of dimension 3x6]
 ]]--

--[[
> x -- memory in x has been modified!
 0  0  0  0  0  0
 1  1  1  1  1  1
 1  1  1  1  1  1
 1  1  1  1  1  1
 0  0  0  0  0  0
[torch.DoubleTensor of dimension 5x6]
 ]]--
 
 
 ------ [Tensor] sub(dim1s, dim1e ... [, dim4s [, dim4e]]) ----------
 --[[
 This method is equivalent to do a series of narrow up to the first 4 dimensions. 
 It returns a new Tensor which is a sub-tensor going from index dimis to dimie in the i-th dimension. 
 Negative values are interpreted index starting from the end: 
	-1 is the last index, -2 is the index before the last index, ...
 ]]--
 
 x = torch.Tensor(5, 6):zero()
--[[
> x
 0 0 0 0 0 0
 0 0 0 0 0 0
 0 0 0 0 0 0
 0 0 0 0 0 0
 0 0 0 0 0 0
[torch.DoubleTensor of dimension 5x6]
 ]]--

y = x:sub(2,4):fill(1) -- y is sub-tensor of x:
 --[[
> y                    -- dimension 1 starts at index 2, ends at index 4
 1  1  1  1  1  1
 1  1  1  1  1  1
 1  1  1  1  1  1
[torch.DoubleTensor of dimension 3x6]
]]--

 --[[
> x                    -- x has been modified!
 0  0  0  0  0  0
 1  1  1  1  1  1
 1  1  1  1  1  1
 1  1  1  1  1  1
 0  0  0  0  0  0
[torch.DoubleTensor of dimension 5x6]
]]--

z = x:sub(2,4,3,4):fill(2) -- we now take a new sub-tensor
 --[[
> z                        -- dimension 1 starts at index 2, ends at index 4
                           -- dimension 2 starts at index 3, ends at index 4
 2  2
 2  2
 2  2
[torch.DoubleTensor of dimension 3x2]
]]--

 --[[
> x                        -- x has been modified
 0  0  0  0  0  0
 1  1  2  2  1  1
 1  1  2  2  1  1
 1  1  2  2  1  1
 0  0  0  0  0  0
[torch.DoubleTensor of dimension 5x6]
]]--

 --[[
> y                        -- y has been modified
 1  1  2  2  1  1
 1  1  2  2  1  1
 1  1  2  2  1  1
[torch.DoubleTensor of dimension 3x6]
]]--

 --[[
> y:sub(-1, -1, 3, 4)      -- negative values = bounds
 2  2
[torch.DoubleTensor of dimension 1x2]
]]--

local val = 0
x = x:apply(function(x)
val = val + 1;
return val
end
)

--[[
print(x)
 1   2   3   4   5   6
 7   8   9  10  11  12
 13  14  15  16  17  18
 19  20  21  22  23  24
 25  26  27  28  29  30
 ]]--

y = x:sub(2, 4) -- no memory copy
--[[
print(y)
 7   8   9  10  11  12
 13  14  15  16  17  18
 19  20  21  22  23  24
[torch.FloatTensor of size 3x6]
]]--

z = x:sub(2,4,3,4) -- y:sub(3,4)

print("x:sub(2,4,3,4): \n", z)
--[[
9  10
15  16
21  22
 ]]--
 
 ----- [Tensor] select(dim, index) --------------

y = x:select(1, 2) -- 选择第1维(这里是行)的 第2. 
--[[
print("select: y = \n",y)
  7
  8
  9
 10
 11
 12
[torch.FloatTensor of size 6]
]]--

y = x:select(2, 2) -- 选择第2维(这里是列)的 第2. 
--[[
print("select: y = ",y)
 2
  8
 14
 20
 26
[torch.FloatTensor of size 5]
]]--

--=============== [Tensor] [{ dim1,dim2,... }] or [{ {dim1s,dim1e}, {dim2s,dim2e} }] =====================
--The indexing operator [] can be used to combine narrow/sub and select in a concise and efficient way. It can also be used to copy, and fill (sub) tensors.
print("--------- start indexing operator ---------")
--print("x[{ 1,3 }]: ",x[{ 1,3 }])  -- 第1行第3列
--print("x[{ 2,{2,4} }]: ", x[{ 2,{2,4} }]) -- 第2行，第2~4列(均包含)
--print("x[{ {},4 }]: ", x[{ {},4 }])  -- 第4列
-- x[{ {},2 }] = torch.range(1,5)

-- x[torch.lt(x,0)] = -2  -- 将所有小于0的数设置为-2.

--------- [Tensor] index(dim, index) -----
-- index函数会使用内存拷贝--底层则是storage。

print("========== start : [Tensor] index(dim, index) =========")

y = x:index(1,torch.LongTensor{3,1}) -- 表示取第1维的第3行 和 第1行，组成新的tensor. 
--[[
print(y)
 13  14  15  16  17  18
  1   2   3   4   5   6
[torch.FloatTensor of size 2x6]
]]--
y = torch.Tensor()
y:index(x,1,torch.LongTensor{3,1})
--[[
print(y)
等同于 y = x:index(1,torch.LongTensor{3,1})
]]--

--- ============ [Tensor] indexCopy(dim, index, tensor) =============
print("=========== start [Tensor] indexCopy(dim, index, tensor) ==========")
z=torch.Tensor(5,2)
z:select(2,1):fill(-1)
z:select(2,2):fill(-2)

-- z中的元素到 y的第2维（列），第5列和第一列。note: shape the shape must be the same. or else error.
y = x:clone();
y:indexCopy(2,torch.LongTensor{5,1},z);
--print(y)
--[[
 -2   2   3   4  -1   6
 -2   8   9  10  -1  12
 -2  14  15  16  -1  18
 -2  20  21  22  -1  24
 -2  26  27  28  -1  30
[torch.FloatTensor of size 5x6]
]]--

---=========== [Tensor] indexAdd(dim, index, tensor) =============
print("============ [Tensor] indexAdd(dim, index, tensor) ======== ")
z = torch.Tensor(5, 2)
z:select(2,1):fill(-1)
z:select(2,2):fill(-2)

y = x:clone();
y:indexAdd(2,torch.LongTensor{5,1},z) -- 将第2维的，第5列所有元素值+(-1). 第1列所有元素值+(-2)
--print(y)
--[[
 -1   2   3   4   4   6
  5   8   9  10  10  12
 11  14  15  16  16  18
 17  20  21  22  22  24
 23  26  27  28  28  30
[torch.FloatTensor of size 5x6]
]]--

---- =========== [Tensor] indexFill(dim, index, val) ==============
-- 类似 [Tensor] indexAdd(dim, index, tensor), 只不过是设置, not add

--- ============ [Tensor] gather(dim, index) =================
-- gather: 
print("======== [Tensor] gather(dim, index) ==========")
x = torch.IntTensor(5, 5)
local val = 0
x = x:apply(function(x)
val = val + 1;
return val
end
)
--[[
print(x)
 1   2   3   4   5
  6   7   8   9  10
 11  12  13  14  15
 16  17  18  19  20
 21  22  23  24  25
[torch.IntTensor of size 5x5]
]]--

-- dim = 1, index = 行.
-- 这里 torch.LongTensor{{1, 2, 3, 4, 5}, {2, 3, 4, 5, 1}} 表示生成2行5列
y = x:gather(1, torch.LongTensor{{1, 2, 3, 4, 5}, {2, 3, 4, 5, 1}})
--print(y)
-- {1, 2, 3, 4, 5} -> (1,1) (2,2) (3,3) (4,4) (5,5)
-- {2, 3, 4, 5, 1} -> (2,1) (3,2) (4,3) (5,4) (1,5) 
-- ...
-- {a1, a2, a3, a4, a5} -> {a1,1},{a2,2},{a3,3},{a4,4},{a5,5}

--[[  1   7  13  19  25
  6  12  18  24   5
[torch.IntTensor of size 2x5] ]]--


-- dim = 2, index = 列
-- torch.LongTensor{{1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 1}} 表示
-- 1, 将会生成5行2列的数据。
--[[
第1行: {1, 2} -> (1,1),(1,2)
第2行: {2, 3} -> (2,2),(2,3)
第3行: {3, 4} -> (3,3),(3,4)
...
第n行: {k1, k2} -> (n,k1),(n,k2)
]]--
z = x:gather(2, torch.LongTensor{{1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 1}})
--print(z)
--[[
 1   2
  7   8
 13  14
 19  20
 25  21
[torch.IntTensor of size 5x2]
]]--


--=================== [Tensor] scatter(dim, index, src|val) ============
print("============ [Tensor] scatter(dim, index, src|val) ===========")
y = torch.zeros(3, 5):int():scatter(1, torch.LongTensor{{1, 2, 3, 1, 1}, {3, 1, 1, 2, 3}}, x)
--[[

]]--
print(y)
--[[
 1   7   8   4   5
  0   2   0   9   0
  6   0   3   0  10
[torch.IntTensor of size 3x5]
]]--
