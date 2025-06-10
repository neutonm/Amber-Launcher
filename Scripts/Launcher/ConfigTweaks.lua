-- COMMAND:     ConfigTweaks
-- DESCRIPTION: Tweaks grayface's MM config for best gameplay experience

local _TweakerWizardEntries = {
    {
        info = "Toggle to change how the Left and Right buttons function in-game:\n * Strafe: Buttons move your character sideways (left/right) without changing orientation.\n * Turn: Buttons rotate your character's viewpoint to the left or right.",
        tag = "strafe",
        { optionName = "Strafe",    imagePath = "Data/Launcher/tweak-strafe-a.gif" },
        { optionName = "Turn",      imagePath = "Data/Launcher/tweak-strafe-b.gif" },
    },
    {
        info = "Decide how GUI would look like.",
        tag = "gui",
        { optionName = "Modern",    imagePath = "Data/Launcher/tweak-gui-a.jpg" },
        { optionName = "Original",  imagePath = "Data/Launcher/tweak-gui-b.jpg" }
    },
    {
        info = "David's sprites",
        tag = "david",
        { optionName = "HD",        imagePath = "Data/Launcher/tweak-gui-a.jpg" }, -- temp
        { optionName = "Original",  imagePath = "Data/Launcher/tweak-gui-b.jpg" }  -- temp
    }
}

local function _ConfigTweaks()

    print("Tweaking mm7 config...")
    iniPath = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR.."mm7.ini"

    local ini = AL.INILoad(iniPath)
    if ini then
        local myStr = AL.INIGet(ini, "Settings", "ViewDistanceD3D")
        if myStr then
            print(myStr)
        end
    else
        print("Failed to load INI file: "..iniPath)
        return false
    end

    local uiResponse = AL.UICall(UIEVENT.MODAL_TWEAKER, _TweakerWizardEntries)
    if uiResponse and uiResponse.status == true then

        print(dump(uiResponse))

        if uiResponse.strafe then
            local myStr = AL.INIGet(ini, "Settings", "AlwaysStrafe")
            print("uiResponse.strafe before: "..myStr)
            AL.INISet(ini, "Settings", "AlwaysStrafe", tostring(1 - uiResponse.strafe))
            myStr = AL.INIGet(ini, "Settings", "AlwaysStrafe")
            print("uiResponse.strafe after: "..myStr)
        end

        if uiResponse.gui then
            local myStr = AL.INIGet(ini, "Settings", "UILayout")
            print("uiResponse.gui before: "..myStr)
            AL.INISet(ini, "Settings", "UILayout", uiResponse.gui == 0 and "UI" or "0")
            myStr = AL.INIGet(ini, "Settings", "UILayout")
            print("uiResponse.gui after: "..myStr)
        end

        if uiResponse.david then
            -- special non-ini case...
        end

        AL.INISave(ini, iniPath)
    end

    print("Tweaking done!")
    AL.INIClose(ini)

    return true
end

function events.InitLauncher()

    print("Initialize ConfigTweaks.lua")
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks) -- overwrites internal cmd

end
