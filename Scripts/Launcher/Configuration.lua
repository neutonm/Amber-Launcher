-- FILE:        Configuration.lua

function AL_EnsureConfig(path, contents)

    if FS.IsFilePresent(path) then return true end
    local file, err = io.open(path, "wb")
    if not file then
        AL_print("Failed to create config: "..path..": "..tostring(err))
        return false
    end
    local written = file:write(contents or "[Settings]\n")
    local closed  = file:close()
    return written ~= nil and closed ~= nil
end

local function _Value(ini, section, key)

    local value = ini and AL.INIGet(ini, section, key)
    return value and value ~= "" and value or nil
end

local function _FillConfig(path, values)

    if not AL_EnsureConfig(path) then return false end
    local ini = AL.INILoad(path)
    if not ini then return false end
    local dirty = false
    for _, entry in ipairs(values) do
        if entry[3] ~= nil and not _Value(ini, entry[1], entry[2]) then
            if not AL.INISet(ini, entry[1], entry[2], entry[3]) then
                AL.INIClose(ini)
                AL_print("Failed to set config key: "..path..": "..entry[2])
                return false
            end
            dirty = true
        end
    end
    local saved = not dirty or AL.INISave(ini, path)
    AL.INIClose(ini)
    return saved and true or false
end

function AL_InitLauncherConfig()

    return _FillConfig(INI_PATH_LAUNCHER, {
        {"Settings", "CloseOnLaunch",   "1"},
        {"Settings", "DefaultLanguage", "English"}
    })
end

function AL_InitProjectConfig()

    local launcher = AL.INILoad(INI_PATH_LAUNCHER)
    local language = _Value(launcher, "Settings", "DefaultLanguage") or "English"
    if launcher then AL.INIClose(launcher) end
    local values = {
        {"Settings", "LaunchCommand", GAME_EXECUTABLE_NAME},
        {"Localisation", "Core", language}
    }
    if GAME_CURRENT.requiresMod then
        values[#values + 1] = {"Localisation", "Mod", language}
    end
    local saved = _FillConfig(INI_PATH_PROJECT, values)
    if saved then
        local context = AL_ConfigContext()
        AL.SetLaunchCommand(context:Get("project", "Settings", "LaunchCommand", GAME_EXECUTABLE_NAME))
        context:Close(false)
    end
    return saved
end

function AL_ConfigContext()

    local files       = {}
    local paths       = {launcher = INI_PATH_LAUNCHER, project = INI_PATH_PROJECT,
                         game = INI_PATH_GAME, mod = INI_PATH_MOD}
    local context     = {game = GAME_CURRENT, root = GAME_DESTINATION_PATH, locales = GAME_LOCALES}
    local writeFailed = false

    local function open(scope)
        assert(scope == "launcher" or scope == "project" or scope == "game" or scope == "mod",
            "Unknown config scope")
        local path = paths[scope]
        if not path then return nil end
        if files[path] == nil then
            local handle = AL.INILoad(path)
            files[path]  = {handle = handle, dirty = false}
            if not handle then AL_print("Failed to open settings: "..path) end
        end
        return files[path]
    end

    function context:Get(scope, section, key, fallback)
        local file = open(scope)
        return _Value(file and file.handle, section, key) or fallback
    end

    function context:Set(scope, section, key, value)
        local file = open(scope)
        if not file or not file.handle then
            writeFailed = true
            return false
        end
        if not AL.INISet(file.handle, section, key, tostring(value)) then
            writeFailed = true
            AL_print("Failed to set config key: "..key)
            return false
        end
        file.dirty = true
        return true
    end

    function context:Close(save)
        local success = not writeFailed
        for path, file in pairs(files) do
            if file.handle then
                if save and file.dirty then
                    local ok, result = pcall(AL.INISave, file.handle, path)
                    if not ok or not result then
                        AL_print("Failed to save settings: "..path)
                        success = false
                    end
                end
                AL.INIClose(file.handle)
            end
        end
        files       = {}
        writeFailed = false
        return success
    end

    return context
end
