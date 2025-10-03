
function AL_DetectMod(destinationFolder, bSetModVersion)

    local approxCustomDungeonScriptSize = 4
    bSetModVersion = bSetModVersion or false
    
    local modManifestLua   = FS.PathJoin(destinationFolder, "Scripts", "manifest.lua")
    local modManifestTable = {}

    -- Latest iteration
    if FS.IsFilePresent(modManifestLua) then
        local ok, t = pcall(dofile, modManifestLua)
        if ok and type(t) == "table" then
            modManifestTable = t
            print(string.format("Detected Amber Island Mod.\n"..
            "\t• Name: \t%s (%s)\n\t• Version: \t%s\n\t• Last update: \t%s",
            t.name, t.game, t.version, t.updated))
            if bSetModVersion then
                local maj, min, pat  = AL_ParseVersion(t.version)
                GAME_MOD_VERSION_STR = t.version
                GAME_MOD_VERSION     = maj * 2^24 + min * 2^12 + pat
            end
            return true, AL_ParseVersion(t.version)
        end
    end

    -- Original Public release
    local modLuaName = FS.PathJoin("Scripts", "General", "Mod.lua")
    local file, err  = io.open(modLuaName, "rb")
    if not file then
        --print("Couldn't open file, assuming there's no mod installed: "..err)
        return false, 0.0
    end

    local fileSizeInKb = file:seek("end") / 1024
    file:close()

    if fileSizeInKb > approxCustomDungeonScriptSize then
        print("Detected first official release of Amber Island Mod.")
        return true, 1.0
    end

    print("Detected Custom Dungeon Mod.")
    return true, 0.0
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

    return true
end

local function _DetectAndInstallMod()

    print("Detecting mod...")

    if AL_DetectMod(GAME_DESTINATION_PATH) then
        -- return true
		-- autoconfig should reinstall it
    end

    AL_print("Amber Island mod is not detected. Installing it now...")
    if not InstallMod(GAME_DESTINATION_PATH) then
        return false
    end

    AL_print("Done!")

    return true
end

function events.InitLauncher()

    print("Initialize DetectAndInstallMod.lua")
    AL.CommandAdd("DetectAndInstallMod", _DetectAndInstallMod, -90) 

end
