
-- based on https://github.com/torch/torch7/wiki/Torch-for-Numpy-users#ones-and-zeros
require “torch”

local np = {}

function np.empty(...)
	return torch.Tensor(...)
end

function np.empty_like(x)
	return x.new(x:size())
end

function np.eye(...)
	return torch.eye(...)
end

function np.identity(...)
	return torch.eye(...)
end
function np.ones()
	return torch.ones()
end

function np.ones_like(x)
	return torch.ones(x:size())
end
function np.zeros()
	return torch.zeros()
end

function np.zeros_like(x)
	return torch.zeros(x:size())
end

function np.array(...)
	return torch.Tensor(...)
end

function np.ascontiguousarray(x)
	return x:contiguous()
end

function np.copy(x)
	return x:clone()
end

function np.fromfile(file)
	return torch.Tensor(torch.Storage(file))
end
-- np.frombuffer/fromfunction/fromiter/fromstring/loadtxt not support

function np.concatenate(...)
	return torch.cat(...)
end

function np.multiply(...)
	return torch.cmul(...)
end

function np.arange(...)
    local args = {...}
	if #args == 3 then
	    -- np.arange(2, 3, 0.1)	torch.linspace(2, 2.9, 10)
		return torch.linspace(args[1], args[2] - args[3], (args[2] - args[1])/args[3])
	else if #args == 1
		return torch.range(0, args[1] - 1)  		
	end
	end
	error("np.arange: argument error. expected 'np.arange(2, 3, 0.1)'/ np.arange(10)")
end

--like np.linspace(1, 4, 6)
function np.linspace(...)
	return torch.linspace(...)
end

function np.logspace(...)
	return torch.logspace(...)
end

function np.diag(...)
	return torch.diag(...);
end

function np.tril(...)
	return torch.tril(...);
end

function np.triu(...)
	return torch.triu(...);
end

-- attributes
function np.shape(x)
	return x:size()
end

function np.strides(x)
	return x:stride()
end

function np.ndim(x)
	return x:dim()
end

function np.data(x)
	return x:data()
end

function np.size(x)
	return x:nElement()
end

--x.size == y.size ============	x:isSameSizeAs(y)

function np.dtype(x)
	return x:type()
end

return self;