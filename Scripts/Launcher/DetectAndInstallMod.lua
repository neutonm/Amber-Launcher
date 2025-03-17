
local function IsAmberIslandMod(destinationFolder)

    local approxCustomDungeonScriptSize = 4
    local checkTable = {

        registry = true,
    }
    -- @todo @temp
    -- Couple of things to consider:
    -- - Check for mod's existence
    -- - check for mod's up to date state, if not the latest - reconfigure

    -- shit to check
    -- Mod scripts
    --- Custom mod [outdated]
    --- Pre "difficulty" version [outdated]
    --- Todays
    -- Registry 

    -- Latest iteration
    local ini = AL.INILoad(destinationFolder..OS_FILE_SEPARATOR.."mod.ini")
    if ini then
        local myStr = AL.INIGet(ini, "Info", "Version")
        if myStr then
            print("Detected Amber Island Mod. Version: "..myStr)
            return true, tonumber(myStr)
        end
        AL.INIClose(ini)
    end

    -- Original Public release
    local file, err = io.open(destinationFolder..OS_FILE_SEPARATOR.."Scripts"..OS_FILE_SEPARATOR.."General"..OS_FILE_SEPARATOR.."Mod.lua", "rb")
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

    -- Check if registry is tweaked
    -- for _, entry in ipairs(_GameRegistryValues) do
    --     local value = AL.GetRegistryKey(entry.key)
    --     if entry.value ~= value then 
    --         print("Registry key \""..entry.key.."\" doesn't match to configuration setting. Adding RegistryTweak requirement for configuration")
    --         checkTable.registry = false
    --         break 
    --     end
    -- end

    -- if checkTable.registry == true then print("Registry shit is OK") end

    -- return true
end

local function InstallMod(destinationFolder)

    local dataFolder = "Data"..OS_FILE_SEPARATOR.."Launcher"..OS_FILE_SEPARATOR

    AL.ArchiveExtract(dataFolder.."_grayfacePatch257.zip", destinationFolder)
    AL.ArchiveExtract(dataFolder.."_mod.zip", destinationFolder)
end

local function _DetectAndInstallMod()

    print("Detecting mod...")

    if IsAmberIslandMod(GAME_DESTINATION_FOLDER) then
        return true
    end

    print("Amber Island mod is not detected. Installing it now...")
    InstallMod(GAME_DESTINATION_FOLDER)

    print("Done!")
end

function events.InitLauncher()

    print("Initialize DetectAndInstallMod.lua")
    AL.CommandAdd("DetectAndInstallMod", _DetectAndInstallMod, -98) 

end
