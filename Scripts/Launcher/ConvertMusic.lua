-- COMMAND:     ConvertMusic
-- DESCRIPTION: Converts MP3 from specified folder into WAV in order to allow looping

local function _ConvertMusic()

    musicDir = FS.PathJoin(GAME_DESTINATION_FOLDER, "Music")
    print("Converting music files in directory: "..musicDir)

    local files = FS.DirectoryList(musicDir)
    if not files then
        print("Converting failed!!!")
        return false
    end

    for _, fileName in ipairs(files) do
        if #fileName > 4 and fileName:sub(-4):lower() == ".mp3" then
            local fullPath = FS.PathJoin(musicDir, fileName)
            print("Track: " .. fullPath)
            if not AL.ConvertMP3ToWAV(fullPath) then
                print("Failed to convert " .. fileName)
                return false
            end
        end
    end

    print("Converting done!")
    return true
end

function events.InitLauncher()

    print("Initialize ConvertMusic.lua")
    AL.CommandAdd("ConvertMusic", _ConvertMusic)

end
