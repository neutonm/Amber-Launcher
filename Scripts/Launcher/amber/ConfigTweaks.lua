-- COMMAND:     ConfigTweaks
-- DESCRIPTION: Tweaks grayface's MM config for best gameplay experience

local _TweakerWizardEntries = {
    {
        tag = "strafe",
        { imagePath = "Data/Launcher/tweak-strafe-a.gif" },
        { imagePath = "Data/Launcher/tweak-strafe-b.gif" },
    },
    {
        tag = "gui",
        { imagePath = "Data/Launcher/tweak-gui-a.jpg" },
        { imagePath = "Data/Launcher/tweak-gui-b.jpg" }
    },
    {
        tag = "david",
        { imagePath = "Data/Launcher/tweak-daveher-a.jpg" },
        { imagePath = "Data/Launcher/tweak-daveher-b.jpg" }
    },
    {
        tag = "mapicons",
        { imagePath = "Data/Launcher/tweak-mapicons-a.jpg" },
        { imagePath = "Data/Launcher/tweak-mapicons-b.jpg" }
    },
    {
        tag = "desktoplink",
        {},
        {}
    }
}

local function _ConfigTweaks()

    AL_print("Tweaking mm7 config...")

    local ini = AL.INILoad(INI_PATH_GAME)
    if not ini then
        AL_print("Failed to load INI file: "..INI_PATH_GAME)
        return false
    end

    -- Necessary changes
    AL.INISet(ini, "Settings", "NoIntro",   "0")
    AL.INISet(ini, "Settings", "MouseLook", "1")
    AL.INISet(ini, "Settings", "tviewdist", "1600")

    local uiResponse = AL.UICall(UIEVENT.MODAL_TWEAKER, AL_TweakerText(_TweakerWizardEntries))
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

        if uiResponse.desktoplink then

            if uiResponse.desktoplink == 0 then
                local exe    = FS.PathJoin(LAUNCHER_PATH, 
                    (FS.OS_NAME=="Windows") and (LAUNCHER_NAME..".exe") or LAUNCHER_NAME)
                FS.CreateDesktopLink(GAME_MOD_ID_TITLE,exe,FS.PathJoin(LAUNCHER_PATH, "icon.ico"))
            end
        end
    end

    AL.INISave(ini, INI_PATH_GAME)

    AL_print("Tweaking done!")
    AL.INIClose(ini)

    sleep(1) -- give that ui time to close

    return true
end

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks, -50)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandRemove("ConfigTweaks")
end
