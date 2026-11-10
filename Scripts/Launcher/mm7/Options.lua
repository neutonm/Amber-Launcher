-- FILE:        mm7/Options.lua

local options = {
    "windowmode",
    "rendermode",
    "languagecore",
    "strafe",
    "ui",
    "mouselook",
    "tviewdist"
}

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    for _, id in ipairs(options) do
        AL_RegisterOption(GAME_OPTIONS[id])
    end
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    for _, id in ipairs(options) do
        AL_UnregisterOption(id)
    end
end
