-- Detect and Copy Game contents into launcher folder

-- List of files to check and copy
local GRAYFACE_FILES = {
    "Data/00 add.icons.lod",
    "Data/00 patch.bitmaps.lod",
    "Data/00 patch.events.lod",
    "Data/00 patch.games.lod",
    "Data/00 patch.icons.lod",
    "Data/01 dragon.games.lod",
    "Data/01 mon pal.bitmaps.lod",
    "Data/01 tunnels.events.lod",
    "Data/01 water.bitmaps.lwd",
    "Data/icons.UI.lod",
    "Data/MouseCursorShooter.bmp",
    "Data/MouseCursorShooterHD.bmp",
    "Data/MouseLookCursor.bmp",
    "Data/MouseLookCursorHD.bmp",
    "MM7Patch ReadMe.TXT",
    "MM7patch.dll",
    "MM7-Rel.exe",
}

local MM7_FILES = {
    "Anims/Magic7.vid",
    "Anims/Might7.vid",
    "Data/BITMAPS.LOD",
    "Data/d3dbitmap.hwl",
    "Data/d3dsprite.hwl",
    "Data/Events.lod",
    "Data/GAMES.LOD",
    "Data/ICONS.LOD",
    "Data/SPRITES.LOD",
    "Music/2.mp3",
    "Music/3.mp3",
    "Music/4.mp3",
    "Music/5.mp3",
    "Music/6.mp3",
    "Music/7.mp3",
    "Music/8.mp3",
    "Music/9.mp3",
    "Music/10.mp3",
    "Music/11.mp3",
    "Music/12.mp3",
    "Music/13.mp3",
    "Music/14.mp3",
    "Music/15.mp3",
    "Music/16.mp3",
    "Music/17.mp3",
    "Music/18.mp3",
    "Music/19.mp3",
    "Music/20.mp3",
    "SOUNDS/Audio.snd",
    "audiere.dll",
    "audio.dll",
    "BINKW32.DLL",
    "MANUAL.PDF",
    "MAP.PDF",
    "mm7.exe",
    "MM7Setup.exe",
    "MSS32.DLL",
    "MSSA3D.M3D",
    "MSSDS3DH.M3D",
    "Mssds3ds.m3d",
    "MSSEAX.M3D",
    "Mssfast.m3d",
    "msvcp90.dll",
    "msvcr90.dll",
    "Readme.txt",
    "Reference_card.PDF",
    "SMACKW32.DLL",
    "VIC32.DLL",
}

-- Utility Functions

-- Function to list directory contents
local function list_directory(dir)
    local listing = {}
    local command
    if OS_NAME == "Windows" then
        -- Windows: Use 'dir' command
        command = 'dir /b /a-d "' .. dir .. '" 2> nul'
    else
        -- Unix-like systems: Use 'ls' command
        command = 'ls -1 "' .. dir .. '" 2> /dev/null'
    end
    local p = io.popen(command)
    if p then
        for file in p:lines() do
            table.insert(listing, file)
        end
        p:close()
    end
    return listing
end

-- Function to join multiple path components
local function join_paths(...)
    local args = {...}
    return table.concat(args, OS_FILE_SEPARATOR)
end

-- Function to find a matching name case-insensitively
local function find_case_insensitive(dir, name)
    local files = list_directory(dir)
    for _, file in ipairs(files) do
        if file:lower() == name:lower() then
            return file
        end
    end
    return nil
end

-- Function to check if a file exists with case-insensitive path
local function case_insensitive_file_exists(base_dir, relative_path)
    -- Split the relative path into components
    local components = {}
    for part in relative_path:gmatch("[^/\\]+") do
        table.insert(components, part)
    end

    local current_dir = base_dir
    for i = 1, #components - 1 do
        local component = components[i]
        local actual_dir = find_case_insensitive(current_dir, component)
        if not actual_dir then
            return false
        end
        current_dir = join_paths(current_dir, actual_dir)
    end

    -- Now check the file
    local file_component = components[#components]
    local actual_file = find_case_insensitive(current_dir, file_component)
    if actual_file then
        return join_paths(current_dir, actual_file)
    else
        return false
    end
end

-- Function to ensure a directory exists; creates it if it doesn't
local function ensure_directory(path)
    -- Use system-specific mkdir command
    local command
    if OS_NAME == "Windows" then
        command = 'mkdir "' .. path .. '" >nul 2>nul'
    else
        command = 'mkdir -p "' .. path .. '"'
    end
    os.execute(command)
end

-- Function to split a file path into directory and filename
local function split_path(filepath)
    -- Find the last occurrence of '/' or '\'
    local pattern = "^(.*)" .. OS_FILE_SEPARATOR
    local dir = filepath:match(pattern)
    local file = filepath:match("[^" .. OS_FILE_SEPARATOR .. "]+$")

    if dir then
        return dir, file
    else
        return "", filepath
    end
end

-- Function to check if a file is present (case-insensitive)
local function CheckFiles(dst, fileList)
    local missingFiles = {}

    for _, relativePath in ipairs(fileList) do
        -- Check if the file exists case-insensitively
        local foundPath = case_insensitive_file_exists(dst, relativePath)

        if not foundPath then
            print("Missing: " .. join_paths(dst, relativePath))
            table.insert(missingFiles, relativePath)
        end
    end

    if #missingFiles == 0 then
        print("✅ Success: All files are present in the destination folder.")
        return true
    else
        print("❌ Failure: The following files are missing in the destination folder:")
        for _, file in ipairs(missingFiles) do
            print(" - " .. file)
        end
        return false
    end
end

-- Function to copy a single file from source to destination
-- Function to copy a single file from source to destination with case-insensitive handling
local function CopyFile(source_folder, destination_folder, relative_path)
    -- Find the actual source path with correct case
    local actual_source_path = case_insensitive_file_exists(source_folder, relative_path)
    if not actual_source_path then
        print("❌ Error: Source file not found (case-insensitive search failed): " .. join_paths(source_folder, relative_path))
        return false
    end

    -- Determine the corresponding destination path
    local relative_dir = split_path(relative_path)
    local actual_relative_path = actual_source_path:sub(#source_folder + 2) -- +2 to account for the path separator

    local dest_path = join_paths(destination_folder, actual_relative_path)

    -- Ensure the destination directory exists
    local dest_dir = GetDirectory(dest_path)
    if dest_dir ~= "" then
        ensure_directory(dest_dir)
    end

    -- Open source file
    local src_file = io.open(actual_source_path, "rb")
    if not src_file then
        print("❌ Error: Unable to open source file: " .. actual_source_path)
        return false
    end

    -- Read the entire content
    local content = src_file:read("*all")
    src_file:close()

    -- Open destination file
    local dest_file = io.open(dest_path, "wb")
    if not dest_file then
        print("❌ Error: Unable to create destination file: " .. dest_path)
        return false
    end

    -- Write the content
    dest_file:write(content)
    dest_file:close()

    print("✅ Copied: " .. actual_relative_path)
    return true
end


-- Function to copy all files from source to destination
local function CopyFiles(source_folder, destination_folder, file_list)
    for _, relative_path in ipairs(file_list) do
        CopyFile(source_folder, destination_folder, relative_path)
    end
    print("📁 File copying completed.")
end

-- Additional functions (e.g., IsAmberIslandMod, _DetectAndCopyGame, etc.)
-- Ensure these functions use the updated CheckFiles and CopyFiles accordingly
-- For brevity, they're omitted here but should be integrated as per your existing script

-- Example usage:
-- CheckFiles("C:/Games/MM7/Destination", MM7_FILES)
-- CopyFiles("C:/Games/MM7/Source", "C:/Games/MM7/Destination", MM7_FILES)

local function IsFileExecutable(path)
    -- Cross-platform method
    if OS_NAME == "Windows" then
        local command = "where \"" .. path .. "\" >nul 2>&1"
        return os.execute(command) == 0
    else
        local command = "test -x \"" .. path.."\""
        return os.execute(command) == true
    end
end

local function IsFilePresent(path)
    local dirname, filename = path:match("(.*/)(.*)")
    dirname     = dirname or "."
    filename    = filename:lower()

    -- Determine the OS-specific directory listing command
    local command
    if OS_FILE_SEPARATOR == '\\' then
        -- Windows
        command = 'dir /b /a "' .. dirname .. '" 2> nul'
    else
        -- Unix-like systems
        command = 'ls -a "' .. dirname .. '" 2> /dev/null'
    end

    -- Execute the directory listing command
    local p = io.popen(command)
    if p == nil then
        return nil
    end

    -- Iterate over each file in the directory
    for file in p:lines() do
        if file:lower() == filename then
            p:close()
            return dirname .. file
        end
    end
    p:close()
    return nil
end

function GetDirectory(full_path)
    -- Use Lua's pattern matching to capture everything before the last '/' or '\'
    local dir = full_path:match("^(.*)[/\\][^/\\]+$")
    if dir then
        return dir
    else
        -- If no separator is found, return an empty string or handle as needed
        return ""
    end
end

local function _DetectAndCopyGame(dst)

    print("Detecting game...")

    local gameFolders = GAME_EXECUTABLE_FOLDERS
    if dst then
        gameFolders = {
            dst
        }
    end
    -- check if game is already installed in the same folder
    local foundGamePath = nil
    if IsFilePresent(GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR..GAME_EXECUTABLE_NAME) then
    
        print("Game is located in same folder as launcher is.")

        -- Check if all files are installed
        if not CheckFiles(GAME_DESTINATION_FOLDER, MM7_FILES) then
            print("Some game files are missing.")
            return
        end

        print("No need for copying game files.")
        
        return
    end

    -- [[FIND]]
    -- Find missing game
    for _, file in ipairs(gameFolders) do
        local foundFile = IsFilePresent(file)
        if foundFile then
            if IsFileExecutable(foundFile) then
                foundGamePath = foundFile
                break
            end
        end
    end

    if not foundGamePath then
        -- @todo Tell player that game wasn't found and offer to find it manually
        -- abort on failure
        print("Failed to find the game!")

        newPath = AL.UICall(UIEVENT.MODAL_GAMENOTFOUND)
        if newPath and newPath ~= "" then
            local f = io.open(newPath, "rb") 
            if f then
                f:close()
                _DetectAndCopyGame(newPath)
            else
                print("Invalid or nonexistent file path: " .. newPath)
            end
        end
        
        return
    end

    print("Found game at: "..foundGamePath)

    local foundDirPath = GetDirectory(foundGamePath)
    if not CheckFiles(foundDirPath, MM7_FILES) then

        print("It looks like found folder has a broken MM7 installation (missing content files)")
        return
    end

    print("Game successfuly detected and verified.")
    print("Copying the game...")
    -- [[ COPYING SRC FILES ]]

    ensure_directory(GAME_DESTINATION_FOLDER)
    CopyFiles(foundDirPath, GAME_DESTINATION_FOLDER, MM7_FILES)

    if not CheckFiles(GAME_DESTINATION_FOLDER, MM7_FILES) then

        print("Couldn't verify files after copying from source folder.")
        return
    end

    print("Copying operation is succesful!")
end

function events.InitLauncher()

    print("Initialize DetectAndCopyGame.lua")
    AL.CommandAdd("DetectAndCopyGame", _DetectAndCopyGame, -100) 

end
