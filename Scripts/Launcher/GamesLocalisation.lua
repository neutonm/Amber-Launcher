-- FILE:        GamesLocalisation.lua

local function _DeleteFiles(root, files)

    for _, path in ipairs(files) do
        local actual = FS.PathResolveCaseInsensitive(root, path)
        if actual and not os.remove(actual) then
            AL_print("Failed to remove localisation file: "..actual)
            return false
        end
    end
    return true
end

local function _FreeText(locale, root)

    return _DeleteFiles(root, {"mm6text.cas", "mm6text.non", "mm7text.dll"})
end

function AL_LocalisationArchive(prefix, code)

    return FS.PathResolveCaseInsensitive(
        FS.PathJoin(LAUNCHER_PATH, "Data", "Launcher", "Archives"), prefix.."-"..code..".zip")
end

local function _BackupRoot(root)

    return FS.PathJoin(root, "Data", "Launcher", "Backups", "Localisation")
end

local function _LocaliseCore(locale, root)

    local archive = AL_LocalisationArchive("core", locale.code)
    if not archive then return false end
    for _, path in ipairs(locale.backupFiles or {}) do
        if not FS.PathResolveCaseInsensitive(_BackupRoot(root), path) and
            not FS.FileCopy(root, _BackupRoot(root), path) then
            return false
        end
    end
    return AL.ArchiveExtract(archive, root)
end

local profiles = {
    MM7 = {
        files   = {"Data/loc.ICONS.LOD", "Data/loc.Events.lod", "SOUNDS/loc.Audio.snd"},
        locales = {
            {name = "English", code = "en_en", coreCb = function() return true end, coreFreeCb = _FreeText},
            {name = "Polish",  code = "pl_pl", coreCb = _LocaliseCore},
            {name = "Russian", code = "ru_ru", coreCb = _LocaliseCore, coreFreeCb = _FreeText},
            {name = "German",  code = "de_de", coreCb = _LocaliseCore},
            -- This archive replaces the base audio file.
            {name = "French",  code = "fr_fr", coreCb = _LocaliseCore, backupFiles = {"SOUNDS/Audio.snd"}}
        }
    }
}

function AL_CreateGameLocales(baseGame)

    local result  = {}
    local profile = profiles[baseGame]
    if not profile then return result end
    for _, source in ipairs(profile.locales) do
        local locale = {}
        for key, value in pairs(source) do locale[key] = value end
        locale.core = FS.PathJoin(LAUNCHER_PATH, "Data", "Launcher", "local-"..locale.code:sub(1, 2).."-core.png")
        locale.coreAvailable = locale.code == "en_en" or not not AL_LocalisationArchive("core", locale.code)
        result[#result + 1]   = locale
    end
    return result
end

function AL_LocaliseCoreFreeAll(root, baseGame)

    local profile = profiles[baseGame]
    if not profile then return false end
    for _, locale in ipairs(profile.locales) do
        for _, path in ipairs(locale.backupFiles or {}) do
            local backup = FS.PathResolveCaseInsensitive(_BackupRoot(root), path)
            if backup then
                if not FS.FileCopy(_BackupRoot(root), root, path) or not os.remove(backup) then
                    return false
                end
            end
        end
    end
    return _DeleteFiles(root, profile.files)
end

local function _FindLocale(locales, which, value)

    for _, locale in ipairs(locales) do
        if locale[which.."Available"] and (locale.name == value or locale.code == value) then
            return locale
        end
    end
end

function AL_SetLocalisation(context, which, value)

    assert(which == "core" or which == "mod", "Unknown localisation scope")
    if context.game ~= GAME_CURRENT or context.locales ~= GAME_LOCALES then return false end
    local locale = _FindLocale(context.locales, which, value)
    if not locale then return false end
    local key = which == "core" and "Core" or "Mod"
    if not context:Set("project", "Localisation", "Applied"..key, "") then return false end

    if which == "core" and not AL_LocaliseCoreFreeAll(context.root, context.game.baseGame) then
        return false
    end
    for _, entry in ipairs(context.locales) do
        local cleanup = entry[which.."FreeCb"]
        if cleanup and cleanup(entry, context.root) == false then return false end
    end
    local apply = locale[which.."Cb"]
    if not apply or not apply(locale, context.root) then
        AL_print("Failed to apply "..which.." localisation: "..locale.name)
        return false
    end
    return context:Set("project", "Localisation", key, locale.name) and
        context:Set("project", "Localisation", "Applied"..key, locale.name)
end

function AL_ConfigureLocalisation()

    local context = AL_ConfigContext()
    local choices = {}
    local core    = context:Get("project", "Localisation", "Core", "English")
    local mod     = context:Get("project", "Localisation", "Mod", "English")
    for _, locale in ipairs(context.locales) do
        local entry = {}
        for key, value in pairs(locale) do entry[key] = value end
        entry.coreSelected = locale.name == core
        entry.modSelected  = locale.name == mod
        choices[#choices + 1] = entry
    end
    context:Close(false)
    local response = AL.UICall(UIEVENT.MODAL_LOCALISATION, choices)
    if not response or response.status ~= true or context.game ~= GAME_CURRENT or
        context.locales ~= GAME_LOCALES then
        return false
    end
    local ok, success = pcall(function()
        if not AL_SetLocalisation(context, "core", response.core) then return false end
        return not response.mod or AL_SetLocalisation(context, "mod", response.mod)
    end)
    local saved = context:Close(true)
    if not ok then AL_print("Failed to configure localisation: "..tostring(success)) end
    return ok and success and saved
end

function AL_SyncLocalisation()

    local context = AL_ConfigContext()
    local ok, success = pcall(function()
        for _, which in ipairs({"core", "mod"}) do
            local key       = which == "core" and "Core" or "Mod"
            local preferred = context:Get("project", "Localisation", key,
                context:Get("launcher", "Settings", "DefaultLanguage", "English"))
            local locale = _FindLocale(context.locales, which, preferred) or
                _FindLocale(context.locales, which, "English")
            if locale and context:Get("project", "Localisation", "Applied"..key) ~= locale.name and
                not AL_SetLocalisation(context, which, locale.code) then
                return false
            end
        end
        return true
    end)
    local saved = context:Close(true)
    if not ok then AL_print("Failed to synchronize localisation: "..tostring(success)) end
    return ok and success and saved
end
