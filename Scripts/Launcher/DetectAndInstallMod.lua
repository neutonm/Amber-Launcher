
local function IsAmberIslandMod(destinationFolder)

    local approxCustomDungeonScriptSize = 4

    -- Latest iteration
    local ini = AL.INILoad(INI_PATH_MOD)
    if ini then
        local myStr = AL.INIGet(ini, "Info", "Version")
        if myStr then
            print("Detected Amber Island Mod. Version: "..myStr)
            return true, tonumber(myStr)
        end
        AL.INIClose(ini)
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
        print('Failed to find archive: ' .. FS.PathJoin(archivesDir, patchZipName))
        return false
    end
    if not modZip then
        print('Failed to find archive: ' .. FS.PathJoin(archivesDir, modZipName))
        return false
    end

    if not AL.ArchiveExtract(patchZip, destinationFolder) then
        print("Failed to extract _grayfacePatch257")
        return false
    end
    if not AL.ArchiveExtract(modZip, destinationFolder) then
        print("Failed to extract _mod.zip")
        return false
    end

    return true
end

local function _DetectAndInstallMod()

    print("Detecting mod...")

    if IsAmberIslandMod(GAME_DESTINATION_FOLDER) then
        return true
    end

    print("Amber Island mod is not detected. Installing it now...")
    if not InstallMod(GAME_DESTINATION_FOLDER) then
        return false
    end

    print("Done!")

    return true
end

function events.InitLauncher()

    print("Initialize DetectAndInstallMod.lua")
    AL.CommandAdd("DetectAndInstallMod", _DetectAndInstallMod, -98) 

end
