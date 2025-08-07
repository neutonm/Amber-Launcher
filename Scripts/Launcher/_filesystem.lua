-- FILE:        _filesystem.lua
-- DESCRIPTION: Cross‑platform filesystem and IO helpers

local fs = {}

fs.OS_NAME           = fs.OS_NAME or (OS_NAME and OS_NAME or "Windows")
fs.OS_FILE_SEPARATOR = OS_FILE_SEPARATOR

function fs.list_directory(dir)

    local listing, cmd = {}, ""
    if fs.OS_NAME == "Windows" then
        cmd = 'dir /b /a-d "'..dir..'" 2> nul'
    else
        cmd = 'ls -1 "'..dir..'" 2> /dev/null'
    end
    local p = io.popen(cmd)
    if p then for f in p:lines() do
        listing[#listing+1] = f end p:close()
    end
    return listing
end

function fs.join_paths(...)

    return table.concat({...}, fs.OS_FILE_SEPARATOR)
end

function fs.find_case_insensitive(dir, name)

    for _, file in ipairs(fs.list_directory(dir)) do
        if file:lower() == name:lower() then
            return file
        end
    end

    return nil
end

function fs.case_insensitive_file_exists(base_dir, relative_path)

    local comps, current = {}, base_dir

    for part in relative_path:gmatch("[^/\\]+") do
        comps[#comps+1] = part
    end

    for i = 1, #comps - 1 do
        local act = fs.find_case_insensitive(current, comps[i])
        if not act then
            return false
        end
        current = fs.join_paths(current, act)
    end

    local actFile = fs.find_case_insensitive(current, comps[#comps])
    return actFile and fs.join_paths(current, actFile) or false
end

function fs.ensure_directory(path)

    if fs.OS_NAME == "Windows" then
        os.execute('mkdir "'..path..'" >nul 2>nul')
    else
        os.execute('mkdir -p "'..path..'"')
    end
end

function fs.split_path(fp)
    local dir = fp:match("^(.*)"..fs.OS_FILE_SEPARATOR)
    local file = fp:match("[^"..fs.OS_FILE_SEPARATOR.."]+$")
    return dir or "", file
end

-------------------------------------------------------------------------------
function fs.CheckFiles(dst, fileList)

    local missing = {}

    for _, rel in ipairs(fileList) do
        if not fs.case_insensitive_file_exists(dst, rel) then
            missing[#missing+1] = rel
        end
    end

    if #missing == 0 then
        --print("Success: All files are present in "..dst)
        return true
    end

    print("Failure: The following files are missing in "..dst..":")
    for _,f in ipairs(missing) do
        print("  - "..f)
    end
    return false
end

function fs.CopyFile(srcRoot, dstRoot, rel)

    local src = fs.case_insensitive_file_exists(srcRoot, rel)
    if not src then
        print("Error: Source file not found: "..rel)
        return false
    end
    local dest       = fs.join_paths(dstRoot, src:sub(#srcRoot + 2))
    local destDir, _ = fs.split_path(dest)

    fs.ensure_directory(destDir)

    local r = io.open(src,  "rb");
    if not r then
        return false
    end

    local w = io.open(dest, "wb");
    if not w then
        r:close()
        return false
    end

    w:write(r:read("*all"))
    r:close()
    w:close()
    print("Copied: "..rel)

    return true
end

function fs.CopyFiles(srcRoot, dstRoot, list)
    for _, rel in ipairs(list) do
        fs.CopyFile(srcRoot, dstRoot, rel)
    end
    print("File copying completed.")
end

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

    local p = io.popen(cmd); if not p then
        return nil
    end

    for f in p:lines() do
        if f:lower() == filename then
            p:close();
            return dirname .. f
        end
    end
    p:close()

    return nil
end

function fs.GetDirectory(full)
    return full:match("^(.*)[/\\][^/\\]+$") or ""
end

return fs
