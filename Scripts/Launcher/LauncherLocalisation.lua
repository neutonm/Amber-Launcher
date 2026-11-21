-- FILE:        LauncherLocalisation.lua

local _Language   = "en_US"
local _Catalogues = {}
local _Root       = FS.PathJoin("Scripts", "Launcher")

local function _Catalogue(root, language)
    local path = FS.PathJoin(root, "Locales", language..".lua")
    if not _Catalogues[path] then
        local ok, texts = false, nil
        if FS.IsFilePresent(path) then ok, texts = pcall(dofile, path) end
        _Catalogues[path] = ok and type(texts) == "table" and texts or {}
    end
    return _Catalogues[path]
end

function AL_UILanguage()
    return _Language
end

function AL_GetUILanguages()
    local languages = {}
    for _, file in ipairs(FS.FilesList(FS.PathJoin(_Root, "Locales"))) do
        local code = file:match("^(%l%l_%u%u)%.lua$")
        if code then
            languages[#languages + 1] = {code = code, name = _Catalogue(_Root, code).name or code}
        end
    end
    table.sort(languages, function(a, b) return a.code < b.code end)
    return languages
end

function AL_InitUILanguage()
    local ini = AL.INILoad(INI_PATH_LAUNCHER)
    local code = ini and AL.INIGet(ini, "Settings", "UILanguage")
    if ini then AL.INIClose(ini) end
    _Language = "en_US"
    for _, language in ipairs(AL_GetUILanguages()) do
        if language.code == code then _Language = code; break end
    end
end

function AL_Text(scope, key, fallback)
    local directory = scope == "launcher" and _Root or
        scope == "game" and GAME_CURRENT.scriptsPath or scope
    local text = directory and (_Catalogue(directory, _Language)[key] or _Catalogue(directory, "en_US")[key])
    if text then return text end
    if scope == "game" then return AL_Text("launcher", key, fallback) end
    return fallback or key
end

function AL_RefreshUI(invalidate)
    if invalidate then _Catalogues = {} end
    local texts = {}
    for _, key in ipairs({"mainIntro", "configIntro", "configSuccess", "gameNotFound"}) do
        texts[key] = AL_Text("game", key)
    end
    local response = AL.UICall(UIEVENT.LANGUAGE, _Language, texts)
    return response and response.status == true
end

function AL_TweakerText(entries)
    local result = {}
    for _, entry in ipairs(entries) do
        local item = {tag = entry.tag, info = AL_Text("game", "wizard."..entry.tag..".info", entry.info)}
        for i, option in ipairs(entry) do
            item[i] = {imagePath = option.imagePath,
                optionName = AL_Text("game", "wizard."..entry.tag..".choice."..i, option.optionName)}
        end
        result[#result + 1] = item
    end
    return result
end
