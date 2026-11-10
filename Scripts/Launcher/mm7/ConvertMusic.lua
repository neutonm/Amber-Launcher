-- FILE:        ConvertMusic.lua

local function _ConvertMusic()

    return AL_ConvertGameMusic(FS.PathJoin(GAME_DESTINATION_PATH, "Music"))
end

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandAdd("ConvertMusic", _ConvertMusic, -70)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("ConvertMusic")
end
