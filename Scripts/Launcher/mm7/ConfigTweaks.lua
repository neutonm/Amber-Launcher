-- COMMAND:     ConfigTweaks
-- DESCRIPTION: Tweaks grayface's MM config for best gameplay experience

local _TweakerWizardEntries = {
    {
        tag = "windowmode",
        {},
        {}
    },
    {
        tag = "rendermode",
        {},
        {}
    },
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
    AL.INISet(ini, "Settings", "NoIntro",         "0")
    AL.INISet(ini, "Settings", "MouseLook",       "1")
    AL.INISet(ini, "Settings", "ViewDistanceD3D", "16000")

    local uiResponse = AL.UICall(UIEVENT.MODAL_TWEAKER, AL_TweakerText(_TweakerWizardEntries))
    if not uiResponse or uiResponse.status ~= true then
        AL.INIClose(ini)
        return false
    end

    if uiResponse.windowmode ~= nil then
        AL.SetRegistryKey("startinwindow", uiResponse.windowmode)
    end
    if uiResponse.rendermode ~= nil then
        AL.SetRegistryKey("2dacceloff", uiResponse.rendermode)
        AL.SetRegistryKey("Use D3D", uiResponse.rendermode)
    end

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
                AL_print("Failed to install foliage: "..tostring(err))
                AL.INIClose(ini)
                return false
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

    local saved = AL.INISave(ini, INI_PATH_GAME)

    AL_print("Tweaking done!")
    AL.INIClose(ini)

    sleep(1) -- give that ui time to close

    return saved and true or false
end

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandAdd("ConfigTweaks", _ConfigTweaks, -50)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("ConfigTweaks")
end
