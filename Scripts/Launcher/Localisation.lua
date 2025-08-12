-- COMMAND:     Localisation
-- DESCRIPTION: Provides localisation api + defines behavior for localisation 
--              modal window during autoconfig

local archivesDir      = FS.PathJoin("Data", "Launcher", "Archives")
local localizationDir  = FS.PathJoin(GAME_DESTINATION_FOLDER, "Scripts", "Localization")

-------------------------------------------------------------------------------
---
local function _DeleteResolved(base, rel)
    local actual = FS.PathResolveCaseInsensitive(base, rel)
    if not actual then return false end
    local ok = os.remove(actual)
    if ok then print("Deleted: " .. actual) end
    return ok
end

local function _DeleteList(base, files)

    local ok = true

    for _, f in ipairs(files) do
        local deleteSuccess
        deleteSuccess = _DeleteResolved(base, f)
        if deleteSuccess == false then
            ok = false
        end
    end

    return ok
end

local function _FindArchive(prefix, code)
    local name = prefix .. "-" .. code .. ".zip"
    return FS.PathResolveCaseInsensitive(archivesDir, name) or FS.PathJoin(archivesDir, name)
end

local function _LocaliseMod(t)

    local files             = {
        "Quests.lua",
        "Common.lua",
        "Scripts.lua"
    }

    AL_print("Mod localisation: "..(t.code).."\n")

    -- Reset localisation if it's english
    if t.code == "en_en" then
        _DeleteList(localizationDir, files)
        return true
    end

    local archivePath = _FindArchive("mod", t.code)
    FS.DirectoryEnsure(localizationDir)

    if not AL.ArchiveExtract(archivePath, localizationDir) then
        AL_print("Failed to extract mod-"..(t.code)..".zip")
        return false
    end

    return true
end

--- Clear all previously installed **core** localisation assets.
-- Removes icon/event/audio localisation files from the game destination folder.
-- Missing files are ignored; successful deletions are logged.
-- @return bool success
function AL_LocaliseCoreFreeAll()

    local files = {
        FS.PathJoin("Data",   "loc.ICONS.LOD"),
        FS.PathJoin("Data",   "loc.Events.lod"),
        FS.PathJoin("SOUNDS", "loc.Audio.snd"),
    }

    return _DeleteList(GAME_DESTINATION_FOLDER, files)
end

local function _LocaliseCore(t)

    AL_print("Core game localisation: "..(t.code).."\n")

    local archivePath = _FindArchive("core", t.code)
    if not AL.ArchiveExtract(archivePath, GAME_DESTINATION_FOLDER) then
        AL_print("Failed to extract core-"..(t.code)..".zip")
        return false
    end

    return true
end

-------------------------------------------------------------------------------
AL_TLocales = {
    {
        name        = "English",
        code        = "en_en",
        core        = FS.PathJoin("Data", "Launcher", "local-en-core.png"),
        mod         = FS.PathJoin("Data", "Launcher", "local-en-mod.png"),
        coreCb      = function(t)
                        return true
                    end,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        _DeleteList(GAME_DESTINATION_FOLDER, files)
                        
                        return true
                    end,
        modCb       = _LocaliseMod,
    },
    {
        name        = "Polish",
        code        = "pl_pl",
        core        = FS.PathJoin("Data", "Launcher", "local-pl-core.png"),
        mod         = FS.PathJoin("Data", "Launcher", "local-pl-mod.png"),
        coreCb      = _LocaliseCore,
        modCb       = _LocaliseMod,
    },
    {
        name        = "Russian",
        code        = "ru_ru",
        core        = FS.PathJoin("Data", "Launcher", "local-ru-core.png"),
        mod         = FS.PathJoin("Data", "Launcher", "local-ru-mod.png"),
        coreCb      = _LocaliseCore,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        _DeleteList(GAME_DESTINATION_FOLDER, files)

                        return true
                    end,
        modCb       = _LocaliseMod,
    }
}

function AL_WriteConfigLocalisationNames(langCore, langMod)

    if langCore == nil and langMod == nil then
        return
    end

    local ini     = AL.INILoad(INI_PATH_MOD)
    if (langCore ~= nil) then
        AL.INISet(ini, "Localisation", "Core", langCore)
    end
    if (langMod ~= nil) then
        AL.INISet(ini, "Localisation", "Mod", langMod)
    end
    AL.INISave(ini, INI_PATH_MOD)
    AL.INIClose(ini)
end

local function _Localisation()

    local langCore = nil
    local langMod  = nil

    AL_print("Setting up localisation for the game...")

    FS.DirectoryEnsure(localizationDir)

    local uiResponse = AL.UICall(UIEVENT.MODAL_LOCALISATION, AL_TLocales)
    if uiResponse and uiResponse.status == true then

        -- cleanup core first
        AL_LocaliseCoreFreeAll()
        for _, locale in ipairs(AL_TLocales) do
            if locale.coreFreeCb then locale.coreFreeCb(locale) end
        end

        -- install localisations
        for _, locale in ipairs(AL_TLocales) do
            if uiResponse.core == locale.code then
                if locale.coreCb then
                    local coreSuccess = locale.coreCb(locale)
                    if not coreSuccess then
                        AL_print("Failed to initialize core localisation.")
                        return false
                    end
                    langCore = locale.name
                end
            end
            if uiResponse.mod == locale.code then
                local modSuccess = locale.modCb(locale)
                if not modSuccess then
                    AL_print("Failed to initialize mod localisation.")
                    return false
                end
                langMod = locale.name
            end
        end
    end

    AL_WriteConfigLocalisationNames(langCore, langMod)
    AL_print("Localisation adjusted!")

    return true
end

function events.InitLauncher()

    print("Initialize Localisation.lua")
    AL.CommandAdd("Localisation", _Localisation) -- overwrites internal cmd

end
