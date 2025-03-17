-- COMMAND:     ConvertMusic
-- DESCRIPTION: Converts MP3 from specified folder into WAV in order to allow looping

local function _GetListOfDirectories(directory)
    local command = nil
    if OS_NAME == "Windows" then
        if directory == "" then
            command = 'dir /b /a-d'
        else
            command = 'dir "' .. directory .. '" /b /a-d'
        end
    elseif OS_NAME == "Linux" then
        if directory == "" then
            command = 'ls -1A'
        else
            command = 'ls -1A "' .. directory .. '"'
        end
    else
        io.stderr:write("Unsupported operating system.\n")
        return nil
    end

    local p = io.popen(command)
    if not p then
        io.stderr:write("Failed to open directory.\n")
        return nil
    end

    local files = {}
    for fileName in p:lines() do
        table.insert(files, fileName)
    end
    p:close()
    return files
end

local function _ConvertMusic()

    directory = "Music"
    print("Converting music files in directory: "..directory)

    local pathSeparator = package.config:sub(1,1) -- Get the directory separator

    local files = _GetListOfDirectories(directory)
    if files then
        for _, fileName in ipairs(files) do
            if fileName ~= "." and fileName ~= ".." then
                if #fileName > 4 and fileName:sub(-4):lower() == ".mp3" then
                    
                    local fullPath
                    if directory == "" then
                        fullPath = fileName
                    else
                        -- Ensure no double separators or trailing separators
                        local dir = directory
                        if dir:sub(-1) == pathSeparator then
                            dir = dir:sub(1, -2)
                        end
                        fullPath = dir .. pathSeparator .. fileName
                    end
                    print("Track: "..fullPath)
                    AL.ConvertMP3ToWAV(fullPath)
                end
            end
        end
    else
        print("Converting failed!!!")
        return
    end

    print("Converting done!")
end

function events.InitLauncher()

    print("Initialize ConvertMusic.lua")

    --[[ AL.CommandAdd("ConvertMusic", _ConvertMusic) ]]

end
