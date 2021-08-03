-- creates an array which contains twice the same tensor
local array = {}
x = torch.Tensor(1)
table.insert(array, x)
table.insert(array, x)

-- array[1] and array[2] refer to the same address
-- x[1] == array[1][1] == array[2][1] == 3.14
array[1][1] = 3.14
assert(array[1][1] == array[2][1])

-- write the array on disk
file = torch.DiskFile('foo.asc', 'w')
file:writeObject(array)
file:close() -- make sure the data is written

-- reload the array
file = torch.DiskFile('foo.asc', 'r')
arrayNew = file:readObject()

-- arrayNew[1] and arrayNew[2] refer to the same address!
-- arrayNew[1][1] == arrayNew[2][1] == 3.14
-- so if we do now:
arrayNew[1][1] = 2.72
-- arrayNew[1][1] == arrayNew[2][1] == 2.72 !