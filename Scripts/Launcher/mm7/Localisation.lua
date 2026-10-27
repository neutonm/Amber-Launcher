-- COMMAND:     Localisation
-- DESCRIPTION: Provides localisation api + defines behavior for localisation
--              modal window during autoconfig

local archivesDir      = FS.PathJoin("Data", "Launcher", "Archives")

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

    return _DeleteList(GAME_DESTINATION_PATH, files)
end

local function _LocaliseCore(t)

    AL_print("Core game localisation: "..(t.code).."\n")

    local archivePath = _FindArchive("core", t.code)
    if not AL.ArchiveExtract(archivePath, GAME_DESTINATION_PATH) then
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
        coreCb      = function(t)
                        return true
                    end,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        _DeleteList(GAME_DESTINATION_PATH, files)

                        return true
                    end,
    },
    {
        name        = "Polish",
        code        = "pl_pl",
        core        = FS.PathJoin("Data", "Launcher", "local-pl-core.png"),
        coreCb      = _LocaliseCore,
    },
    {
        name        = "Russian",
        code        = "ru_ru",
        core        = FS.PathJoin("Data", "Launcher", "local-ru-core.png"),
        coreCb      = _LocaliseCore,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        _DeleteList(GAME_DESTINATION_PATH, files)

                        return true
                    end,
    },
    {
        name        = "German",
        code        = "de_de",
        core        = FS.PathJoin("Data", "Launcher", "local-de-core.png"),
        coreCb      = _LocaliseCore,
    },
    {
        name        = "French",
        code        = "fr_fr",
        core        = FS.PathJoin("Data", "Launcher", "local-fr-core.png"),
        coreCb      = _LocaliseCore,
    },
}

function AL_WriteConfigLocalisationNames(langCore)

    if langCore == nil then
        return
    end

    local ini     = AL.INILoad(INI_PATH_MOD)
    if not ini then
        AL_print("[Error]: failed to open localisation settings: "..INI_PATH_MOD)
        return false
    end
    if (langCore ~= nil) then
        AL.INISet(ini, "Localisation", "Core", langCore)
    end
    local saved = AL.INISave(ini, INI_PATH_MOD)
    AL.INIClose(ini)
    if not saved then
        AL_print("[Error]: failed to save localisation settings: "..INI_PATH_MOD)
        return false
    end
    return true
end

local function _Localisation()

    local langCore = nil

    AL_print("Setting up localisation for the game...")

    local uiResponse = AL.UICall(UIEVENT.MODAL_LOCALISATION, AL_TLocales)
    if not uiResponse or uiResponse.status ~= true then
        return false
    end

    -- cleanup core first
    if type(AL_LocaliseCoreFreeAll) == "function" then
        AL_LocaliseCoreFreeAll()
    end
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

    end

    if not langCore or AL_WriteConfigLocalisationNames(langCore) == false then
        return false
    end
    AL_print("Localisation adjusted!")

    return true
end

function events.InitLauncher()

    print("Initialize Localisation.lua")
    AL.CommandAdd("Localisation", _Localisation, -40) -- overwrites internal cmd

end
