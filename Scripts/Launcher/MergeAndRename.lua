-- List of proper folder names
local correct_folder_names = {
    "Anims",
    "Data",
    "SOUNDS",
    "Saves",
}

-- List of proper file names
local correct_file_names = {
    "MM7.exe",
    "MM7Setup.exe",
}

-- Function to join paths
local function join_paths(...)
    local path_separator = package.config:sub(1,1)
    return table.concat({...}, path_separator)
end

-- Function to list directories in the base directory
local function list_directories(dir)
    local dirs = {}
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'dir "' .. dir .. '" /b /ad 2> nul'
    else
        -- Unix-like systems
        command = 'find "' .. dir .. '" -maxdepth 1 -type d -printf "%f\\n" 2> /dev/null'
    end
    local p = io.popen(command)
    if p then
        for entry in p:lines() do
            if entry ~= "." and entry ~= ".." then
                table.insert(dirs, entry)
            end
        end
        p:close()
    end
    return dirs
end

-- Function to list files in the base directory
local function list_files(dir)
    local files = {}
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'dir "' .. dir .. '" /b /a-d 2> nul'
    else
        -- Unix-like systems
        command = 'find "' .. dir .. '" -maxdepth 1 -type f -printf "%f\\n" 2> /dev/null'
    end
    local p = io.popen(command)
    if p then
        for entry in p:lines() do
            table.insert(files, entry)
        end
        p:close()
    end
    return files
end

-- Function to ensure a directory exists; creates it if it doesn't
local function ensure_directory(path)
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'if not exist "' .. path .. '" mkdir "' .. path .. '"'
    else
        -- Unix-like systems
        command = 'mkdir -p "' .. path .. '"'
    end
    os.execute(command)
end

-- Function to move contents from src_dir to dest_dir
local function move_contents(src_dir, dest_dir)
    -- Ensure dest_dir exists
    ensure_directory(dest_dir)

    -- List contents of src_dir
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'move /Y "' .. src_dir .. '\\*.*" "' .. dest_dir .. '" > nul'
    else
        -- Unix-like systems
        command = 'mv -f "' .. src_dir .. '"/. "' .. dest_dir .. '" 2> /dev/null'
    end
    os.execute(command)
end

-- Function to delete a directory or file
local function delete_path(path)
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'rmdir /S /Q "' .. path .. '" > nul 2>&1 || del /F /Q "' .. path .. '" > nul 2>&1'
    else
        -- Unix-like systems
        command = 'rm -rf "' .. path .. '"'
    end
    os.execute(command)
end

-- Function to copy a file
local function copy_file(src_file, dest_file)
    local command
    if package.config:sub(1,1) == '\\' then
        -- Windows
        command = 'copy /Y "' .. src_file .. '" "' .. dest_file .. '" > nul'
    else
        -- Unix-like systems
        command = 'cp -f "' .. src_file .. '" "' .. dest_file .. '"'
    end
    os.execute(command)
end

-- Main function to process folders
local function process_folders(base_dir, correct_folder_names)
    -- Get all directories in the base directory
    local entries = list_directories(base_dir)

    -- Create a mapping of lowercased folder names to actual folder names
    local entries_lower = {}
    for _, entry in ipairs(entries) do
        local lower_entry = entry:lower()
        if entries_lower[lower_entry] == nil then
            entries_lower[lower_entry] = {}
        end
        table.insert(entries_lower[lower_entry], entry)
    end

    -- Process each correct folder name
    for _, correct_name in ipairs(correct_folder_names) do
        local correct_name_lower = correct_name:lower()
        local correct_path = join_paths(base_dir, correct_name)

        local matching_entries = entries_lower[correct_name_lower] or {}
        local correct_exists = false
        local improper_folders = {}

        for _, actual_name in ipairs(matching_entries) do
            if actual_name == correct_name then
                correct_exists = true
            else
                table.insert(improper_folders, actual_name)
            end
        end

        if not correct_exists then
            if #improper_folders > 0 then
                -- Rename the first improperly named folder to the correct name
                local improper_path = join_paths(base_dir, improper_folders[1])
                local success, err = os.rename(improper_path, correct_path)
                if not success then
                    print("Error renaming folder: " .. err)
                else
                    print("Renamed folder '" .. improper_folders[1] .. "' to '" .. correct_name .. "'")
                end
                -- Remove the renamed folder from the list
                table.remove(improper_folders, 1)
            else
                -- No folder exists, create the correct folder
                ensure_directory(correct_path)
                print("Created missing folder '" .. correct_name .. "'")
            end
        end

        -- Now, move contents from any remaining improperly named folders
        for _, improper_name in ipairs(improper_folders) do
            local improper_path = join_paths(base_dir, improper_name)
            print("Moving contents from '" .. improper_name .. "' to '" .. correct_name .. "'")
            move_contents(improper_path, correct_path)
            -- Remove the empty improperly named folder
            delete_path(improper_path)
            print("Removed folder '" .. improper_name .. "'")
        end
    end
end

-- Function to process files
local function process_files(base_dir, correct_file_names)
    -- Get all files in the base directory
    local entries = list_files(base_dir)

    -- Create a mapping of lowercased file names to actual file names
    local entries_lower = {}
    for _, entry in ipairs(entries) do
        local lower_entry = entry:lower()
        if entries_lower[lower_entry] == nil then
            entries_lower[lower_entry] = {}
        end
        table.insert(entries_lower[lower_entry], entry)
    end

    -- Process each correct file name
    for _, correct_name in ipairs(correct_file_names) do
        local correct_name_lower = correct_name:lower()
        local correct_path = join_paths(base_dir, correct_name)

        local matching_entries = entries_lower[correct_name_lower] or {}
        local correct_exists = false
        local improper_files = {}

        for _, actual_name in ipairs(matching_entries) do
            if actual_name == correct_name then
                correct_exists = true
            else
                table.insert(improper_files, actual_name)
            end
        end

        if not correct_exists then
            if #improper_files > 0 then
                -- Choose one of the improperly named files to rename
                local improper_path = join_paths(base_dir, improper_files[1])
                local success, err = os.rename(improper_path, correct_path)
                if not success then
                    print("Error renaming file: " .. err)
                else
                    print("Renamed file '" .. improper_files[1] .. "' to '" .. correct_name .. "'")
                end
                -- Remove the renamed file from the list
                table.remove(improper_files, 1)
            else
                -- No file exists; you might want to handle missing files here
                print("File '" .. correct_name .. "' is missing.")
            end
        end

        -- Handle any remaining improperly named files
        for _, improper_name in ipairs(improper_files) do
            local improper_path = join_paths(base_dir, improper_name)
            print("Overwriting '" .. correct_name .. "' with '" .. improper_name .. "'")
            -- Overwrite the correct file with the improper one
            copy_file(improper_path, correct_path)
            -- Delete the improperly named file
            delete_path(improper_path)
            print("Removed file '" .. improper_name .. "'")
        end
    end
end

local function _MergeAndRename()

    print("Merging/Renaming files...")

    -- Run the processing function
    process_folders(GAME_DESTINATION_FOLDER, correct_folder_names)
    process_files(GAME_DESTINATION_FOLDER, correct_file_names)

    print("Done!")

    return true
end

function events.InitLauncher()

    print("Initialize MergeAndRename.lua")
    AL.CommandAdd("MergeAndRename", _MergeAndRename, -99) 

end
