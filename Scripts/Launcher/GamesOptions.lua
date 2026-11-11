-- FILE:        GamesOptions.lua
-- DESCRIPTION: Shared game option definitions.

local function _LocaleNames(context, which)

    local names = {}
    for _, locale in ipairs(context.locales) do
        if locale[which.."Available"] then names[#names + 1] = locale.name end
    end
    return names
end

local function _Language(which, title)

    local key = which == "core" and "Core" or "Mod"
    return {
        id       = "language"..which,
        section  = "General",
        title    = title,
        type     = UIWIDGET.POPUP,
        optTitle = function(context)
            return _LocaleNames(context, which)
        end,
        read     = function(context)
            local name = context:Get("project", "Localisation", "Applied"..key,
                context:Get("project", "Localisation", key, "English"))
            for i, available in ipairs(_LocaleNames(context, which)) do
                if available == name then return i - 1 end
            end
            return 0
        end,
        callback = function(option, context)
            return AL_SetLocalisation(context, which, option.optTitle[option.value + 1])
        end
    }
end

local function _GameINI(id, section, title, widgetType, key, fallback, choices, decode, encode)

    return {
        id       = id,
        section  = section,
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
            return context:Set("game", "Settings", key, value)
        end
    }
end

local function _EnabledByDefault(value)

    return tonumber(value) == 0 and 0 or 1
end

GAME_OPTIONS = {
    windowmode   = {
        id       = "windowmode",
        section  = "General",
        title    = "Window Mode:",
        type     = UIWIDGET.RADIO,
        optTitle = {"Fullscreen", "Windowed"},
        read     = function()
            return tonumber(AL.GetRegistryKey("startinwindow")) == 1 and 1 or 0
        end,
        callback = function(option)
            AL.SetRegistryKey("startinwindow", option.value)
        end
    },
    rendermode   = {
        id       = "rendermode",
        section  = "General",
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
    },
    languagecore = _Language("core", "Language (Core Game):"),
    languagemod  = _Language("mod",  "Language (Mod):"),
    strafe       = _GameINI("strafe", "Gameplay", "Side movement behavior:",
        UIWIDGET.RADIO, "AlwaysStrafe", "1", {"Turn", "Move (strafe)"}, _EnabledByDefault),
    ui           = _GameINI("ui", "Gameplay", "User Interface:",
        UIWIDGET.RADIO, "UILayout", "0", {"Modern (HD)", "Original"},
        function(value) return value == "UI" and 0 or 1 end,
        function(value) return value == 0 and "UI" or "0" end),
    mouselook    = _GameINI("mouselook", "Gameplay", "Mouse Look Mode:",
        UIWIDGET.CHECKBOX, "MouseLook", "1", nil, _EnabledByDefault),
    tviewdist    = _GameINI("tviewdist", "Tweaks", "View Distance:",
        UIWIDGET.EDIT, "ViewDistanceD3D", "16000")
}
