-- FILE:        ModalUpdater.lua
-- DESCRIPTION: Launcher / Mod Updater script

local _oldMods = {}

local function _isVersionNewer(vNew, vOld)
    local a1,a2,a3 = AL_ParseVersion(vNew)
    local b1,b2,b3 = AL_ParseVersion(vOld)
    if a1 ~= b1 then return a1 >  b1 end
    if a2 ~= b2 then return a2 >  b2 end
    return a3 >  b3
end

local function _UpdateMods()

    -- Reinstall updated Mods
    local newMods = AL_ScanMods()
    local oldById = {}

    for _, m in ipairs(_oldMods) do
        oldById[m.id] = m
    end

    for _, m in ipairs(newMods) do
        local prev = oldById[m.id]
        if prev and prev.version and m.version and
            _isVersionNewer(m.version, prev.version) then
            AL_print(string.format(
                "[Updater] %s upgraded %s → %s - reinstalling...",
                m.id, tostring(prev.version), tostring(m.version)
            ))
            AL_ReinstallMod(m.id)
        end
    end
end

function ModalShowUpdater()

    local uiResponse = AL.UICall(UIEVENT.MODAL_UPDATER)
    if uiResponse and uiResponse.status == true then

        AL.UICall(
            UIEVENT.MODAL_MESSAGE,
            "Update successful!\n\nLauncher will restart now..."
        )

        if OS_NAME == "Windows" then
            os.execute(LAUNCHER_NAME..".exe")
        else
            os.execute("./"..LAUNCHER_NAME)
        end

        AL.UICall(UIEVENT.EXITAPP)
    end
end

function events.BeforeUpdate()

    _oldMods = AL_ScanMods()
end

function events.AfterUpdate()

    AL_print("[Updater] Checking mods...")
    _UpdateMods()
    AL_print("[Updater] Done!")
end
