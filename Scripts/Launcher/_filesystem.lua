-- FILE:        _filesystem.lua
-- DESCRIPTION: Cross-platform filesystem and IO helpers

local fs = {}

fs.OS_NAME           = fs.OS_NAME or (OS_NAME and OS_NAME or "Windows")
fs.OS_FILE_SEPARATOR = OS_FILE_SEPARATOR

-------------------------------------------------------------------------------
-- Path functions
-------------------------------------------------------------------------------
function fs.PathGetDirectory(full)
    return full:match("^(.*)[/\\][^/\\]+$") or ""
end

function fs.PathJoin(...)
    return table.concat({...}, fs.OS_FILE_SEPARATOR)
end

function fs.PathSplit(fp)
    local dir = fp:match("^(.*)"..fs.OS_FILE_SEPARATOR)
    local file = fp:match("[^"..fs.OS_FILE_SEPARATOR.."]+$")
    return dir or "", file
end

function fs.PathResolveCaseInsensitive(base_dir, relative_path)
    local comps, current = {}, base_dir

    for part in relative_path:gmatch("[^/\\]+") do
        comps[#comps+1] = part
    end

    for i = 1, #comps - 1 do
        local act = fs.DirectoryFindCaseInsensitive(current, comps[i])
        if not act then
            return false
        end
        current = fs.PathJoin(current, act)
    end

    local actFile = fs.DirectoryFindCaseInsensitive(current, comps[#comps])
    return actFile and fs.PathJoin(current, actFile) or false
end

function fs.PathDelete(path)
    local cmd
    if fs.OS_NAME == "Windows" then
        cmd = 'if exist "'..path..'" (rmdir /S /Q "'..path..'" > nul 2>&1 || del /F /Q "'..path..'" > nul 2>&1)'
    else
        cmd = 'rm -rf "'..path..'"'
    end
    return os.execute(cmd) == 0
end


-------------------------------------------------------------------------------
-- Directory functions
-------------------------------------------------------------------------------

function fs.DirectoryList(dir)
    local listing, cmd = {}, ""
    if fs.OS_NAME == "Windows" then
        local target = (dir == "" or dir == nil) and "." or dir
        cmd = ('dir "%s" /b /a-d'):format(target)
    elseif fs.OS_NAME == "Linux" then
        local target = (dir == "" or dir == nil) and "." or dir
        cmd = ('ls -1A "%s"'):format(target)
    else
        io.stderr:write("Unsupported operating system.\n")
        return nil
    end

    local p = io.popen(cmd)
    if not p then
        io.stderr:write("Failed to open directory.\n")
        return nil
    end
    for f in p:lines() do listing[#listing+1] = f end
    p:close()
    return listing
end

function fs.DirectoryFindCaseInsensitive(dir, name)
    for _, file in ipairs(fs.DirectoryList(dir)) do
        if file:lower() == name:lower() then
            return file
        end
    end
    return nil
end

function fs.DirectoryEnsure(path)
    if fs.OS_NAME == "Windows" then
        os.execute('mkdir "'..path..'" >nul 2>nul')
    else
        os.execute('mkdir -p "'..path..'"')
    end
end

-------------------------------------------------------------------------------
-- File operations
-------------------------------------------------------------------------------

function fs.FilesList(dir)
    local files, cmd = {}, ""
    local target = (dir == "" or dir == nil) and "." or dir
    if fs.OS_NAME == "Windows" then
        cmd = ('dir "%s" /b /a-d 2> nul'):format(target)
    else
        cmd = ('find "%s" -maxdepth 1 -mindepth 1 -type f -printf "%%f\\n" 2> /dev/null'):format(target)
    end
    local p = io.popen(cmd); if not p then return files end
    for entry in p:lines() do files[#files+1] = entry end
    p:close()
    return files
end

function fs.FilesCheck(dst, fileList)
    local missing = {}

    for _, rel in ipairs(fileList) do
        if not fs.PathResolveCaseInsensitive(dst, rel) then
            missing[#missing+1] = rel
        end
    end

    if #missing == 0 then
        return true
    end

    print("Failure: The following files are missing in "..dst..":")
    for _,f in ipairs(missing) do
        print("  - "..f)
    end
    return false
end

function fs.FileCopy(srcRoot, dstRoot, rel)
    local src = fs.PathResolveCaseInsensitive(srcRoot, rel)
    if not src then
        print("Error: Source file not found: "..rel)
        return false
    end
    local dest       = fs.PathJoin(dstRoot, src:sub(#srcRoot + 2))
    local destDir, _ = fs.PathSplit(dest)

    fs.DirectoryEnsure(destDir)

    local r = io.open(src,  "rb"); if not r then return false end
    local w = io.open(dest, "wb"); if not w then r:close(); return false end

    w:write(r:read("*all"))
    r:close(); w:close()
    print("Copied: "..rel)
    return true
end

function fs.FilesCopy(srcRoot, dstRoot, list)
    for _, rel in ipairs(list) do
        fs.FileCopy(srcRoot, dstRoot, rel)
    end
    print("File copying completed.")
end

-- Move contents of one directory into another (includes hidden files)
function fs.FilesMove(src_dir, dst_dir)
    
    fs.DirectoryEnsure(dst_dir)
    local cmd
    if fs.OS_NAME == "Windows" then
        cmd = 'move /Y "'..src_dir..'\\*" "'..dst_dir..'" > nul 2>&1'
    else
        cmd = 'find "'..src_dir..'" -mindepth 1 -maxdepth 1 -exec mv -f {} "'..dst_dir..'/" \\; 2> /dev/null'
    end
    return os.execute(cmd) == 0
end

-------------------------------------------------------------------------------
-- Checks
-------------------------------------------------------------------------------

function fs.IsFileExecutable(path)
    if fs.OS_NAME == "Windows" then
        return os.execute('where "'..path..'" >nul 2>&1') == 0
    else
        return os.execute('test -x "'..path..'"') == true
    end
end

function fs.IsFilePresent(path)
    local dirname, filename = path:match("(.*/)(.*)")
    dirname  = dirname or "."
    filename = filename:lower()
    local cmd

    if fs.OS_FILE_SEPARATOR == '\\' then
        cmd = 'dir /b /a "'..dirname..'" 2> nul'
    else
        cmd = 'ls -a "'..dirname..'" 2> /dev/null'
    end

    local p = io.popen(cmd)
    if not p then
        return false
    end

    for f in p:lines() do
        if f:lower() == filename then
            p:close()
            return true
        end
    end
    p:close()
    return false
end

return fs
