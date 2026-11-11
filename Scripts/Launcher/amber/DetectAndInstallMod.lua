-- FILE:        DetectAndInstallMod.lua

local function _DetectMod(destination)

    local manifest = FS.PathResolveCaseInsensitive(destination, "Scripts/manifest.lua")
    if manifest then
        local ok, info = pcall(dofile, manifest)
        if ok and type(info) == "table" and info.name == "Amber Island" and
            type(info.version) == "string" then
            AL_print("Detected Amber Island "..info.version)
            return true, info.version
        end
    end

    local legacy = FS.PathResolveCaseInsensitive(destination, "Scripts/General/Mod.lua")
    local attr   = legacy and lfs.attributes(legacy)
    if not attr or attr.mode ~= "file" then return false end
    return true, attr.size > 4 * 1024 and "1.0.0" or "0.0.0"
end

local function InstallMod(destinationFolder)

    local archivesDir   = FS.PathJoin("Data", "Launcher", "Archives")
    local patchZipName  = "_grayfacePatch257.zip"
    local modZipName    = "_mod.zip"
    local patchZip      = FS.PathResolveCaseInsensitive(archivesDir, patchZipName)
    local modZip        = FS.PathResolveCaseInsensitive(archivesDir, modZipName)

    if not patchZip then
        AL_print('Failed to find archive: ' .. FS.PathJoin(archivesDir, patchZipName))
        return false
    end
    if not modZip then
        AL_print('Failed to find archive: ' .. FS.PathJoin(archivesDir, modZipName))
        return false
    end

    if not AL.ArchiveExtract(patchZip, destinationFolder) then
        AL_print("Failed to extract _grayfacePatch257")
        return false
    end
    if not AL.ArchiveExtract(modZip, destinationFolder) then
        AL_print("Failed to extract _mod.zip")
        return false
    end
    local obsolete = FS.PathResolveCaseInsensitive(destinationFolder, "mod.ini")
    if obsolete and not os.remove(obsolete) then
        AL_print("Failed to remove obsolete config: "..obsolete)
    end

    return true
end

local function _DetectAndInstallMod()

    AL_print("Installing Amber Island...")
    if not InstallMod(GAME_DESTINATION_PATH) then
        return false
    end
    if not AL_DetectMod(GAME_DESTINATION_PATH, true) then return false end

    AL_print("Done!")

    return true
end

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    GAME_CURRENT.detectMod = _DetectMod
    AL.CommandAdd("DetectAndInstallMod", _DetectAndInstallMod, -90)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandRemove("DetectAndInstallMod")
    GAME_CURRENT.detectMod = nil
end
