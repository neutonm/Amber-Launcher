-- FILE:        MusicConversion.lua
-- DESCRIPTION: Converts numbered MP3 tracks to looping WAV files.

function AL_ConvertGameMusic(directory, tracks)

    AL_print("Converting music files in directory: "..directory)

    if lfs.attributes(directory, "mode") ~= "directory" then
        AL_print("Music directory not found: "..directory)
        return false
    end

    for _, fileName in ipairs(FS.FilesList(directory)) do
        if fileName:match("^%d+%.mp3$") and (not tracks or tracks[fileName]) then
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

function AL_SnapshotGameMusic(directory)

    local hashes = {}
    if lfs.attributes(directory, "mode") ~= "directory" then return hashes end

    -- Backups contain the source used for the current WAV.
    for _, fileName in ipairs(FS.FilesList(directory)) do
        local track = fileName:match("^(%d+)%.bak%.mp3$")
        if track then
            hashes[track..".mp3"] = AL.SHA256HashFile(FS.PathJoin(directory, fileName))
        end
    end
    return hashes
end

function AL_ConvertUpdatedGameMusic(directory, hashes)

    if lfs.attributes(directory, "mode") ~= "directory" then return true end

    local tracks = {}
    for _, fileName in ipairs(FS.FilesList(directory)) do
        local track = fileName:match("^(%d+)%.mp3$")
        if track then
            local hash = AL.SHA256HashFile(FS.PathJoin(directory, fileName))
            if not hash then
                AL_print("Failed to hash music file: "..fileName)
                return false
            end
            if hash ~= hashes[fileName] or not FS.IsFilePresent(FS.PathJoin(directory, track..".wav")) then
                tracks[fileName] = true
            end
        end
    end
    if next(tracks) then
        return AL_ConvertGameMusic(directory, tracks)
    end
    return true
end
