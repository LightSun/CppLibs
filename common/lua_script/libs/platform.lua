
local system;
local function getOsName()
	-- for windows. report undefined  cmd of 'uname' and system is nil.
    system = io.popen("uname -s"):read("*l")  
	if(not system) then
		system = "Windows"
		return;
	end
	--[[
	if system == "Darwin" then  
		-- do something darwin related  
	elseif system == "Windows" then
		-- do something Windows related
	elseif system == "Linux" then  
		-- do something Linux related  
	end  ]]--
end

if pcall(getOsName) then
	print("os is ", system)
	return system;
end
print("os is ", system)
return system;
