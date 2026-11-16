-- FILE:        ConvertMusic.lua

local _MusicHashes

local function _ConvertMusic()

    return AL_ConvertGameMusic(FS.PathJoin(GAME_DESTINATION_PATH, "Music"))
end

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandAdd("ConvertMusic", _ConvertMusic, -70)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandRemove("ConvertMusic")
    _MusicHashes = nil
end

function events.BeforeUpdate()

    if GAME_CURRENT.key ~= "AMBER" then return end
    _MusicHashes = AL_SnapshotGameMusic(FS.PathJoin(GAME_DESTINATION_PATH, "Music"))
end

function events.AfterUpdate()

    if GAME_CURRENT.key ~= "AMBER" or not _MusicHashes then return end
    AL_ConvertUpdatedGameMusic(FS.PathJoin(GAME_DESTINATION_PATH, "Music"), _MusicHashes)
    _MusicHashes = nil
end
