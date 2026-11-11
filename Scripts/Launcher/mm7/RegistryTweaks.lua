-- COMMAND:     RegistryTweaks
-- DESCRIPTION: Tweaks MM vars in Windows Registry for best gameplay experience

local _GameRegistryValues = {

    { key = "2dacceloff",         value = 0 },
    { key = "3DSoundAvailable",   value = 1 },
    { key = "D3DAvailable",       value = 1 },
    { key = "Colored Lights",     value = 0 },
    { key = "Use D3D",            value = 1 },
    { key = "Disable3DSound",     value = 0 },
    { key = "startinwindow",      value = 0 },
}

local function _RegistryTweaks()

    AL_print("Tweaking windows registry...")

    for i = 1, #_GameRegistryValues do
        local entry = _GameRegistryValues[i]
        AL.SetRegistryKey(entry.key, entry.value)
    end

    AL_print("Done!")
    return true
end

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandAdd("RegistryTweaks", _RegistryTweaks, -60)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("RegistryTweaks")
end
