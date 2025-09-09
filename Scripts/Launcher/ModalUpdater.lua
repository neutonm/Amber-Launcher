-- FILE:        ModalUpdater.lua
-- DESCRIPTION: Launcher / Mod Updater script

local function _parseVersion(v)
    -- "major.minor.patch" or "major.minor"
    local maj, min, pat = v:match("^(%d+)%.(%d+)%.?(%d*)$")
    return tonumber(maj) or 0,
           tonumber(min) or 0,
           tonumber(pat) or 0
end

local function _isVersionNewer(vNew, vOld)
    local a1,a2,a3 = _parseVersion(vNew)
    local b1,b2,b3 = _parseVersion(vOld)
    if a1 ~= b1 then return a1 >  b1 end
    if a2 ~= b2 then return a2 >  b2 end
    return a3 >  b3
end

function ModalShowUpdater()

    local oldMods = AL_ScanMods()

    local uiResponse = AL.UICall(UIEVENT.MODAL_UPDATER)
    if uiResponse and uiResponse.status == true then
        print(dump(uiResponse))
    end

    -- Reinstall updated Mods
    local newMods = AL_ScanMods()
    local oldById = {}

    for _, m in ipairs(oldMods) do
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
