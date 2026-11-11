-- COMMAND:     InstallPatch
-- DESCRIPTION: Installs GrayFace for MM7, without the Amber mod archive.

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

    return AL_EnsureConfig(INI_PATH_GAME,
        "[Settings]\nPlayMP3=1\nMusicLoopsCount=0\nDataFiles=1\n")
end

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandAdd("InstallPatch", _InstallPatch, -90)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("InstallPatch")
end
