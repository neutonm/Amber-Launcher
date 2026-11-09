-- FILE:        MusicConversion.lua
-- DESCRIPTION: Converts numbered MP3 tracks to looping WAV files.

function AL_ConvertGameMusic(directory)

    AL_print("Converting music files in directory: "..directory)

    if lfs.attributes(directory, "mode") ~= "directory" then
        AL_print("Music directory not found: "..directory)
        return false
    end

    for _, fileName in ipairs(FS.FilesList(directory)) do
        if fileName:match("^%d+%.mp3$") then
            local fullPath = FS.PathJoin(directory, fileName)
            print("Track: "..fullPath)
            if not AL.ConvertMP3ToWAV(fullPath) then
                AL_print("Failed to convert "..fileName)
                return false
            end
        end
    end

    AL_print("Converting done!")
    return true
end
