-- FILE:        _filesystem.lua
-- DESCRIPTION: Cross-platform filesystem and IO helpers

local fs = {}

fs.OS_NAME           = fs.OS_NAME or (OS_NAME and OS_NAME or "Windows")
fs.OS_FILE_SEPARATOR = OS_FILE_SEPARATOR

-------------------------------------------------------------------------------
-- Static Helper Functions (replaced shell calls with Lua/lfs)
-------------------------------------------------------------------------------

local lfs = lfs  -- lfs is already available globally

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
    if not relative_path or relative_path == "" then
        return false
    end

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

-- Recursive delete (file or directory)
function fs.PathDelete(path)
    local attr = lfs.attributes(path)
    if not attr then return false end

    if attr.mode == "file" then
        return os.remove(path)
    elseif attr.mode == "directory" then
        for entry in lfs.dir(path) do
            if entry ~= "." and entry ~= ".." then
                local sub = fs.PathJoin(path, entry)
                fs.PathDelete(sub)
            end
        end
        return lfs.rmdir(path)
    end
    return false
end

function fs.PathNormalize(path)
    if fs.OS_NAME == "Windows" then
        return path:gsub("/", "\\")
    else
        return path
    end
end

function fs.PathClean(path)
    local sep  = fs.OS_FILE_SEPARATOR
    local parts = {}
    for part in path:gmatch("[^"..sep.."]+") do
        if part == ".." then
            table.remove(parts)
        elseif part ~= "." then
            table.insert(parts, part)
        end
    end
    local root = path:sub(1,1) == sep and sep or ""
    return root .. table.concat(parts, sep)
end

-------------------------------------------------------------------------------
-- Directory functions
-------------------------------------------------------------------------------

function fs.CurrentDir()
    return lfs.currentdir() or ""
end

function fs.DirectoryList(dir)
    local listing = {}
    local target = (dir == "" or dir == nil) and "." or dir
    for entry in lfs.dir(target) do
        if entry ~= "." and entry ~= ".." then
            listing[#listing+1] = entry
        end
    end
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

-- Recursively create directories
function fs.DirectoryEnsure(path)
    if path == "" or path == nil then return end
    local sep = fs.OS_FILE_SEPARATOR
    local prefix = ""

    -- Handle drive letter on Windows
    if fs.OS_NAME == "Windows" and path:match("^%a:[/\\]") then
        prefix = path:sub(1,2)
        path = path:sub(3)
    end

    local current = prefix
    for part in path:gmatch("[^"..sep.."]+") do
        current = (current == "" and part) or fs.PathJoin(current, part)
        lfs.mkdir(current)
    end
end

-------------------------------------------------------------------------------
-- File operations
-------------------------------------------------------------------------------

function fs.FilesList(dir)
    local files = {}
    local target = (dir == "" or dir == nil) and "." or dir
    for entry in lfs.dir(target) do
        if entry ~= "." and entry ~= ".." then
            local full = fs.PathJoin(target, entry)
            local attr = lfs.attributes(full)
            if attr and attr.mode == "file" then
                files[#files+1] = entry
            end
        end
    end
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
    for entry in lfs.dir(src_dir) do
        if entry ~= "." and entry ~= ".." then
            local from = fs.PathJoin(src_dir, entry)
            local to   = fs.PathJoin(dst_dir, entry)
            local ok, err = os.rename(from, to)
            if not ok then
                io.stderr:write("Move failed: "..tostring(err).."\n")
                return false
            end
        end
    end
    return true
end

-------------------------------------------------------------------------------
-- Checks
-------------------------------------------------------------------------------

function fs.IsFileExecutable(path)
    local attr = lfs.attributes(path)
    if not attr or attr.mode ~= "file" then return false end

    if fs.OS_NAME == "Windows" then
        return true  -- executability checks are not straightforward on Windows
    else
        return attr.permissions:match("x") ~= nil
    end
end

function fs.IsFilePresent(path)
    local attr = lfs.attributes(path)
    return attr ~= nil
end

-------------------------------------------------------------------------------
-- Misc
-------------------------------------------------------------------------------

--- Create a desktop link to an executable without using system calls.
-- On Windows, generates a .url shortcut; on Linux, a .desktop file.
function fs.CreateDesktopLink(name, target, icon)
    assert(type(name)   == "string" and name  ~= "",  "name required")
    assert(type(target) == "string" and target ~= "", "target required")

    local desktop
    if fs.OS_NAME == "Windows" then
        desktop = os.getenv("USERPROFILE") .. "\\Desktop"
    else
        desktop = os.getenv("HOME") .. "/Desktop"
    end
    fs.DirectoryEnsure(desktop)

    if fs.OS_NAME == "Windows" then
        local file = fs.PathJoin(desktop, name .. ".url")
        icon = icon or target
        local f = io.open(file, "w")
        if not f then return false end
        f:write("[InternetShortcut]\n")
        f:write("URL=\"", target, "\"\n")
        f:write("IconFile=\"", icon, "\"\n")
        f:write("IconIndex=0\n")
        f:close()
        return true
    elseif fs.OS_NAME == "Linux" then
        local file = fs.PathJoin(desktop, name .. ".desktop")
        local f = io.open(file, "w")
        if not f then return false end
        icon = icon or target
        f:write("[Desktop Entry]\n")
        f:write("Type=Application\n")
        f:write("Name=", name, "\n")
        f:write("Exec=\"", target, "\"\n")
        f:write("Icon=", icon, "\n")
        f:write("Terminal=false\n")
        f:write("Categories=Game;\n")
        f:close()
        return true
    end

    io.stderr:write("CreateDesktopLink: Unsupported OS: "..tostring(fs.OS_NAME).."\n")
    return false
end

return fs
