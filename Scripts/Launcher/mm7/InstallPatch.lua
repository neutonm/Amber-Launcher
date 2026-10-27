-- COMMAND:     InstallPatch
-- DESCRIPTION: Installs GrayFace for MM7, without the Amber mod archive.

local function _EnsureConfig(path, contents)

    if FS.IsFilePresent(path) then
        return true
    end
    local file, err = io.open(path, "wb")
    if not file then
        AL_print("Failed to create config: "..path..": "..tostring(err))
        return false
    end
    local written = file:write(contents)
    local closed  = file:close()
    return written ~= nil and closed ~= nil
end

local function _InstallPatch()

    local archivesDir = FS.PathJoin("Data", "Launcher", "Archives")
    local patchName   = "_grayfacePatch257.zip"
    local patchPath   = FS.PathResolveCaseInsensitive(archivesDir, patchName)

    if not patchPath then
        AL_print("Failed to find archive: "..FS.PathJoin(archivesDir, patchName))
        return false
    end
    AL_print("Installing GrayFace patch for Might and Magic VII...")
    if not AL.ArchiveExtract(patchPath, GAME_DESTINATION_PATH) then
        AL_print("Failed to extract "..patchName)
        return false
    end

    -- Amber normally supplies these through _mod.zip. Vanilla MM7 needs its own
    -- initial configs; keep existing settings when Autoconfig is run again.
    if not _EnsureConfig(INI_PATH_GAME,
        "[Settings]\nPlayMP3=1\nMusicLoopsCount=0\nDataFiles=1\n") then
        return false
    end
    return _EnsureConfig(INI_PATH_MOD,
        "[Settings]\nCurrentGame=MM7\nCloseOnLaunch=1\n[Localisation]\nCore=English\n")
end

function events.InitLauncher()

    AL.CommandAdd("InstallPatch", _InstallPatch, -90)
end
