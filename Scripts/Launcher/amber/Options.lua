-- FILE:        amber/Options.lua

local options = {
    "windowmode",
    "rendermode",
    "languagecore",
    "languagemod",
    "strafe",
    "ui",
    "mouselook",
    "tviewdist"
}

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    for _, id in ipairs(options) do
        AL_RegisterOption(GAME_OPTIONS[id])
    end
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    for _, id in ipairs(options) do
        AL_UnregisterOption(id)
    end
end
