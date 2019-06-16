function readFile(fileName)
    local f = assert(io.open(fileName,'r'))
    local content = f:read('*all')
    f:close()
    return content
end
