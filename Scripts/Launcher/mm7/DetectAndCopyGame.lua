-- FILE:        DetectAndCopyGame.lua

function AL_DetectGame(searchFolder, bIgnoreWav)

    return AL_DetectGameFiles({
        destination = GAME_DESTINATION_PATH,
        exeName     = MM7_GAME_FILES.exeName,
        searchPaths = searchFolder and {searchFolder} or GAME_EXECUTABLE_FOLDERS,
        files       = bIgnoreWav and MM7_GAME_FILES.copyFiles or MM7_GAME_FILES.detectFiles
    })
end

local function _DetectAndCopyGame()

    return AL_DetectAndCopyGameFiles({
        destination = GAME_DESTINATION_PATH,
        exeName     = MM7_GAME_FILES.exeName,
        searchPaths = GAME_EXECUTABLE_FOLDERS,
        files       = MM7_GAME_FILES.copyFiles
    })
end

function events.GameActivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandAdd("DetectAndCopyGame", _DetectAndCopyGame, -100)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.MM7 then return end
    AL.CommandRemove("DetectAndCopyGame")
end
