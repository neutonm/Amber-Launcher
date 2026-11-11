-- FILE:        amber/Localisation.lua

local function _LocaliseMod(locale, root)

    local directory = FS.PathJoin(root, "Scripts", "Localization")
    if locale.code == "en_en" then
        for _, name in ipairs({"Quests.lua", "Common.lua", "Scripts.lua"}) do
            local path = FS.PathResolveCaseInsensitive(directory, name)
            if path and not os.remove(path) then return false end
        end
        return true
    end
    local archive = AL_LocalisationArchive("mod", locale.code)
    return archive and FS.DirectoryEnsure(directory) and AL.ArchiveExtract(archive, directory) or false
end

function events.GameActivated(gameID)

    if gameID ~= MODID.AMBER then return end
    GAME_LOCALES = AL_CreateGameLocales(GAME_CURRENT.baseGame)
    for _, locale in ipairs(GAME_LOCALES) do
        if locale.code == "en_en" or AL_LocalisationArchive("mod", locale.code) then
            locale.modCb        = _LocaliseMod
            locale.modAvailable = true
            locale.mod          = ""
            if locale.code == "en_en" then
                locale.mod = FS.PathJoin(LAUNCHER_PATH, "Data", "Launcher", "local-en-mod.png")
            end
        end
    end
    AL.CommandAdd("Localisation", AL_ConfigureLocalisation, -40)
end

function events.GameDeactivated(gameID)

    if gameID ~= MODID.AMBER then return end
    AL.CommandRemove("Localisation")
    GAME_LOCALES = {}
end
