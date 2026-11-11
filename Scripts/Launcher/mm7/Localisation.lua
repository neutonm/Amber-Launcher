-- FILE:        mm7/Localisation.lua

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    GAME_LOCALES = AL_CreateGameLocales(GAME_CURRENT.baseGame)
    AL.CommandAdd("Localisation", AL_ConfigureLocalisation, -40)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("Localisation")
    GAME_LOCALES = {}
end
