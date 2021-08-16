
--[[
ByteStorage, CharStorage, ShortStorage, IntStorage, LongStorage, FloatStorage, DoubleStorage.
]]--

require 'torch'

x = torch.IntStorage(10):fill(1)
--print("IntStorage default:",torch.IntStorage(10)) --random value
print("IntStorage:",x)
print(x[5])

-- convert int to double
y = torch.DoubleStorage(10):copy(x)
print("DoubleStorage",y)


x = torch.IntStorage({1,2,3,4});
print("IntStorage with table:", x)

-- create with size 10
x = torch.DoubleStorage(10);
-- create storage from x , and start offset(index = 2) = 3, size = 5. change y will effect x
y = torch.DoubleStorage(x, 3, 5)
y = y:fill(1)
print("with offset x: ", x)
print("with offset y: ", y)

-- create storage with share memory or not
x = torch.CharStorage('hello.txt')
print("hello:",x)
print("hello:",x:string())

-- create storage of shared memeory with size = 10.
x = torch.CharStorage('hello2.txt', true, 10)
x:fill(42);
print("hello2:", x)

--[[
func:
#self
self[index]
[self] copy(storage)
[self] fill(value)
[self] resize(size)
[number] size()
[self] string(str)
		* This function is available only on ByteStorage and CharStorage.*
		This method resizes the storage to the length of the provided string str, 
		and copy the contents of str into the storage. The NULL terminating character is not copied, 
		but str might contain NULL characters. The method returns the Storage.	
[string] string()
		* This function is available only on ByteStorage and CharStorage.*
		
retain() 
		increare reference
free()
		decrease reference. free if 0.
]]--