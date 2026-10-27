-- FILE:        mm7/Options.lua
-- DESCRIPTION: MM7's game settings. Language preferences remain in mod.ini.

local function _LocaleNames(which)

    local names = {}
    for _, locale in ipairs(AL_TLocales or {}) do
        if locale[which] then
            names[#names + 1] = locale.name
        end
    end
    return names
end

local function _RegisterLanguage(which, title)

    local configKey = which == "core" and "Core" or "Mod"
    AL_RegisterOption({
        id       = "language"..which,
        section  = "General",
        games    = {"MM7"},
        title    = title,
        type     = UIWIDGET.POPUP,
        optTitle = function()
            return _LocaleNames(which)
        end,
        read     = function(context)
            local name     = context:Get("mod", "Localisation", configKey, "English")
            local fallback = 0
            for i, available in ipairs(_LocaleNames(which)) do
                if available == name then
                    return i - 1
                end
                if available == "English" then
                    fallback = i - 1
                end
            end
            return fallback
        end,
        callback = function(option, context)
            local name = option.optTitle[option.value + 1]
            for _, locale in ipairs(AL_TLocales or {}) do
                if locale.name == name and locale[which] then
                    -- The common cleanup hook is optional; locale-specific hooks
                    -- retain their existing behavior, including no-op callbacks.
                    if which == "core" and type(AL_LocaliseCoreFreeAll) == "function" then
                        AL_LocaliseCoreFreeAll()
                    end
                    local cleanup = locale[which.."FreeCb"]
                    local apply   = locale[which.."Cb"]
                    if cleanup then
                        cleanup(locale)
                    end
                    if apply and apply(locale) then
                        context:Set("mod", "Localisation", configKey, locale.name)
                    else
                        AL_print("Failed to initialize "..which.." localisation.")
                        return false
                    end
                    return
                end
            end
        end
    })
end

local function _RegisterGameINI(id, section, title, widgetType, key, fallback, choices, decode, encode)

    AL_RegisterOption({
        id       = id,
        section  = section,
        games    = {"MM7"},
        title    = title,
        type     = widgetType,
        optTitle = choices,
        read     = function(context)
            local value = context:Get("game", "Settings", key, fallback)
            if decode then
                return decode(value)
            end
            return value
        end,
        callback = function(option, context)
            local value = encode and encode(option.value) or option.value
            context:Set("game", "Settings", key, value)
        end
    })
end

function events.InitLauncher()

    AL_RegisterOption({
        id       = "windowmode",
        section  = "General",
        games    = {"MM7"},
        title    = "Window Mode:",
        type     = UIWIDGET.RADIO,
        optTitle = {"Fullscreen", "Windowed"},
        read     = function()
            return tonumber(AL.GetRegistryKey("startinwindow")) == 1 and 1 or 0
        end,
        callback = function(option)
            AL.SetRegistryKey("startinwindow", option.value)
        end
    })
    AL_RegisterOption({
        id       = "rendermode",
        section  = "General",
        games    = {"MM7"},
        title    = "Rendering mode:",
        type     = UIWIDGET.RADIO,
        optTitle = {"Software", "Hardware"},
        read     = function()
            return tonumber(AL.GetRegistryKey("2dacceloff")) == 1 and 1 or 0
        end,
        callback = function(option)
            AL.SetRegistryKey("2dacceloff", option.value)
            AL.SetRegistryKey("Use D3D", option.value)
        end
    })

    _RegisterLanguage("core", "Language (Core Game):")

    local function enabledByDefault(value)
        return tonumber(value) == 0 and 0 or 1
    end

    _RegisterGameINI("strafe", "Gameplay", "Side movement behavior:",
        UIWIDGET.RADIO, "AlwaysStrafe", "1", {"Turn", "Move (strafe)"}, enabledByDefault)
    _RegisterGameINI("ui", "Gameplay", "User Interface:",
        UIWIDGET.RADIO, "UILayout", "0", {"Modern (HD)", "Original"},
        function(value) return value == "UI" and 0 or 1 end,
        function(value) return value == 0 and "UI" or "0" end)
    _RegisterGameINI("mouselook", "Gameplay", "Mouse Look Mode:",
        UIWIDGET.CHECKBOX, "MouseLook", "1", nil, enabledByDefault)
    _RegisterGameINI("tviewdist", "Tweaks", "View Distance:",
        UIWIDGET.EDIT, "ViewDistanceD3D", "16000")
end
