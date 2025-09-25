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

function fs.PathDelete(path)
    local cmd
    if fs.OS_NAME == "Windows" then
        cmd = 'if exist "'..path..'" (rmdir /S /Q "'..path..'" > nul 2>&1 || del /F /Q "'..path..'" > nul 2>&1)'
    else
        cmd = 'rm -rf "'..path..'"'
    end
    return os.execute(cmd) == 0
end

function fs.PathNormalize(path)
    if fs.OS_NAME == "Windows" then
        return path:gsub("/", "\\")
    else
        return path
    end
end

function fs.PathClean(path)
    local sep 	= fs.OS_FILE_SEPARATOR
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
    local p = (fs.OS_NAME == "Windows") and io.popen("cd") or io.popen("pwd")
    if not p then
        return ""
    end
    local line = p:read("*l")
    p:close()
    return line
end

function fs.DirectoryList(dir)
    local listing, cmd = {}, ""
    if fs.OS_NAME == "Windows" then
        local target = (dir == "" or dir == nil) and "." or dir
        cmd = ('dir "%s" /b'):format(target)
    elseif fs.OS_NAME == "Linux" then
        local target = (dir == "" or dir == nil) and "." or dir
        cmd = ('ls -1A "%s"'):format(target)
    else
        io.stderr:write("Unsupported operating system.\n")
        return {}
    end

    local p   = io.popen(cmd)
    if not p then return {} end
    local out = p:read('*a')
    p:close()
    local listing = {}
    for line in out:gmatch("[^\r\n]+") do
        listing[#listing+1] = line
    end
    return listing
end

function fs.DirectoryFindCaseInsensitive(dir, name)
    local dirList = {}
    dirList = fs.DirectoryList(dir)
    for _, file in ipairs(dirList) do
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

local function os_ok(cmd)
    local ok, why, code = os.execute(cmd)
    if type(ok) == "number" then
        -- lua 5.1
        return ok == 0
    else
        -- lua 5.2+
        return ok and code == 0
    end
end

function fs.IsFileExecutable(path)
    return fs.OS_NAME == "Windows"
           and os_ok(('where "%s" >nul 2>&1'):format(path))
           or  os_ok(('test -x "%s"'):format(path))
end

function fs.IsFilePresent(path)
    local sep 		= OS_FILE_SEPARATOR
    local pattern 	= "^(.*" .. sep .. ")([^" .. sep .. "]+)$"
    local dirname, filename = path:match(pattern)

    if not filename then
        dirname = "."
        filename = path
    end
	
    filename = filename:lower()
	
	----------------
    local cmd

    if OS_NAME == "Windows" then
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

-------------------------------------------------------------------------------
-- Misc
-------------------------------------------------------------------------------

--- Create a desktop link to an executable.
-- @tparam string name       Friendly title (e.g. "Amber Launcher")
-- @tparam string target     Absolute path to the EXE / ELF you want to launch
-- @tparam[opt] string icon  Absolute path to an .ico / .png (falls back to target)
-- @treturn boolean          true on success, false on failure
function fs.CreateDesktopLink(name, target, icon)

    assert(type(name)   == "string" and name  ~= "",  "name required")
    assert(type(target) == "string" and target ~= "", "target required")

    -- Resolve desktop folder
    local desktop
    if fs.OS_NAME == "Windows" then
        desktop = os.getenv("USERPROFILE") .. "\\Desktop"
    else  -- Linux (XDG-desktop fallback)
        desktop = os.getenv("HOME") .. "/Desktop"
    end
    fs.DirectoryEnsure(desktop)

    -- Windows: generate <name>.lnk with PowerShell & WSH
    if fs.OS_NAME == "Windows" then
        local lnkPath = fs.PathJoin(desktop, name .. ".lnk")
        icon = icon or target  -- .lnk accepts “file,iconindex”

        -- Escape single quotes in paths for embedding in PowerShell string
        local function psEscape(s)
            -- For single-quoted PowerShell string: double any single quotes
            return s:gsub("'", "''")
        end

        local tPathEsc = psEscape(target)
        local workingDir = psEscape(fs.PathGetDirectory(target))
        local iconEsc = psEscape(icon)
        local lnkEsc = psEscape(lnkPath)

        -- Using single-quoted PowerShell Here-String may simplify quoting
        local psCmd = ([[
    $s=(New-Object -ComObject WScript.Shell).CreateShortcut('%s');$s.TargetPath='%s';$s.WorkingDirectory='%s';$s.IconLocation='%s,0';$s.Save()
]]):format(lnkEsc, tPathEsc, workingDir, iconEsc)

        -- Wrap with -Command parameter, ensure escaping
        local fullCmd = ("powershell -NoProfile -ExecutionPolicy Bypass -Command \"%s\""):format(psCmd)

        -- Optionally, log the command for debugging
        print("[DEBUG] CreateDesktopLink PS cmd:", fullCmd)

        local ok = os.execute(fullCmd)
		print("OK: "..tostring(ok))
        return ok == 0

    -- Linux: generate <name>.desktop
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
        os.execute('chmod +x "'..file..'"')
        return true
    end

    io.stderr:write("CreateDesktopLink: Unsupported OS: "..tostring(fs.OS_NAME).."\n")
    return false
end

return fs
