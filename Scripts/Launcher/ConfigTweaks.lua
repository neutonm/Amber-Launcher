-- COMMAND:     ConfigTweaks
-- DESCRIPTION: Tweaks grayface's MM config for best gameplay experience

local function _ConfigTweaks()

    print("Tweaking mm7 config...")
    iniPath = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR.."mm7.ini"

    local ini = AL.INILoad(iniPath)
    if ini then
        local myStr = AL.INIGet(ini, "Settings", "ViewDistanceD3D")
        if myStr then
            print(myStr)
        end
        AL.INIClose(ini)
    else
        print("Failed to load INI file: "..iniPath)
        return false
    end

    print("Tweaking done!")

    return true
end

function events.InitLauncher()

    print("Initialize ConfigTweaks.lua")
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks) -- overwrites internal cmd

end
