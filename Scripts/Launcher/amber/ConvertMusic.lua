-- FILE:        ConvertMusic.lua

local function _ConvertMusic()

    return AL_ConvertGameMusic(FS.PathJoin(GAME_DESTINATION_PATH, "Music"))
end

function events.InitLauncher()

    AL.CommandAdd("ConvertMusic", _ConvertMusic, -70)
end
