-- FILE:        MergeAndRename.lua

local function _MergeAndRename()

    if not AL_NormalizeGameFiles({
        destination = GAME_DESTINATION_PATH,
        folderNames = MM7_GAME_FILES.folderNames,
        fileNames   = MM7_GAME_FILES.fileNames
    }) then
        return false
    end

    return AL_VerifyGameFiles(GAME_DESTINATION_PATH, MM7_GAME_FILES.copyFiles)
end

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandAdd("MergeAndRename", _MergeAndRename, -80)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandRemove("MergeAndRename")
end
