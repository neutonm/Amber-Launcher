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
        info = "Choose between the classic UI or a new HD widescreen interface that gives you more view space.",
        tag = "gui",
        { optionName = "Modern",    imagePath = "Data/Launcher/tweak-gui-a.jpg" },
        { optionName = "Original",  imagePath = "Data/Launcher/tweak-gui-b.jpg" }
    },
    {
        info = "Mod: Uses DaveHer’s HD foliage sprites, replacing the vanilla foliage.",
        tag = "david",
        { optionName = "HD",        imagePath = "Data/Launcher/tweak-daveher-a.jpg" },
        { optionName = "Original",  imagePath = "Data/Launcher/tweak-daveher-b.jpg" }
    },
    {
        info = "Mod: Replaces the default map texture with a version that marks shops, teachers, and dungeons for easier navigation.",
        tag = "mapicons",
        { optionName = "Extra Icons",imagePath = "Data/Launcher/tweak-mapicons-a.jpg" },
        { optionName = "Original",   imagePath = "Data/Launcher/tweak-mapicons-b.jpg" }
    }
}

local function _INISet(ini, )
    local myStr = AL.INIGet(ini, "Settings", "AlwaysStrafe")
    AL.INISet(ini, "Settings", "AlwaysStrafe", tostring(1 - uiResponse.strafe))
    myStr = AL.INIGet(ini, "Settings", "AlwaysStrafe")
end

local function _ConfigTweaks()

    AL_print("Tweaking mm7 config...")

    local ini = AL.INILoad(INI_PATH_MM7)
    if not ini then
        AL_print("Failed to load INI file: "..iniPath)
        return false
    end

    -- Necessary changes
    AL.INISet(ini, "Settings", "NoIntro",   "0")
    AL.INISet(ini, "Settings", "MouseLook", "1")
    AL.INISet(ini, "Settings", "tviewdist", "1600")

    local uiResponse = AL.UICall(UIEVENT.MODAL_TWEAKER, _TweakerWizardEntries)
    if uiResponse and uiResponse.status == true then

        if uiResponse.strafe then
            AL.INISet(ini, "Settings", "AlwaysStrafe", tostring(1 - uiResponse.strafe))
        end

        if uiResponse.gui then
            AL.INISet(ini, "Settings", "UILayout", uiResponse.gui == 0 and "UI" or "0")
        end

        if uiResponse.david then
            AL_UninstallMod("davidsprites")

            if uiResponse.david == 0 then
                local ok, err = AL_InstallMod("davidsprites")
                if not ok then
                    print("Failed:", err)
                end
            end
        end

        if uiResponse.mapicons then
            AL_UninstallMod("extramapicons")

            if uiResponse.mapicons == 0 then
                local ok, err = AL_InstallMod("extramapicons")
                if not ok then
                    print("Failed:", err)
                end
            end
        end
    end

    AL.INISave(ini, INI_PATH_MM7)

    AL_print("Tweaking done!")
    AL.INIClose(ini)

    sleep(1) -- give that ui time to close

    return true
end

function events.InitLauncher()

    print("Initialize ConfigTweaks.lua")
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks, -50) -- overwrites internal cmd

end
