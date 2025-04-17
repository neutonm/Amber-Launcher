-- COMMAND:     RegistryTweaks
-- DESCRIPTION: Tweaks MM vars in Windows Registry for best gameplay experience

_GameRegistryValues = {

    { key = "2dacceloff",         value = 0 },
    { key = "3DSoundAvailable",   value = 1 },
    { key = "D3DAvailable",       value = 1 },
    { key = "Colored Lights",     value = 0 },
    { key = "Use D3D",            value = 1 },
    { key = "Disable3DSound",     value = 0 },
    { key = "startinwindow",      value = 0 },
}

local function _RegistryTweaks()

    print("Tweaking windows registry...")

    for i = 1, #_GameRegistryValues do
        local entry = _GameRegistryValues[i]
        AL.SetRegistryKey(entry.key, entry.value)
    end

    print("Tweaking done!")
    return true
end

function events.InitLauncher()

    print("Initialize RegistryTweaks.lua")
    AL.CommandAdd("RegistryTweaks", _RegistryTweaks) -- overwrites internal cmd
end
