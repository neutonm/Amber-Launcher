-- COMMAND:     ConfigTweaks
-- DESCRIPTION: Tweaks grayface's MM config for best gameplay experience

local function _ConfigTweaks()

    print("Tweaking mm7 config...")

    local ini = AL.INILoad("mm7.ini")
    if ini then
        local myStr = AL.INIGet(ini, "Settings", "ViewDistanceD3D")
        if myStr then
            print(myStr)
        end
        AL.INIClose(ini)
    else
        print("Failed to load INI file.")
    end

    print("Tweaking done!")

end

function events.InitLauncher()

    print("Initialize ConfigTweaks.lua")
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks) -- overwrites internal cmd

end
