-- DESCRIPTION: Allows tweaking game options separately

local _LocaleCoreTable = {}
local _LocaleModTable  = {}

local function _GetLocaleNames(which, t)
    assert(which == "core" or which == "mod", "Argument must be 'core' or 'mod'")
    t = t or AL_TLocales

    local result = {}

    for _, locale in ipairs(t) do
        if which == "core" and locale.core then
            table.insert(result, locale.name)
        elseif which == "mod" and locale.mod then
            table.insert(result, locale.name)
        end
    end

    return result
end

local function _GetLocaleNameFromConfig(which)
    assert(which == "core" or which == "mod", "Argument must be 'core' or 'mod'")

    local ini    = AL.INILoad(INI_PATH_MOD)
    local retStr = AL.INIGet(ini, "Localisation", which == "core" and "Core" or "Mod")
    AL.INIClose(ini)

    retStr = (retStr == nil or retStr == 0) and "English" or retStr
    return retStr
end

local function _GetLocaleIDFromConfig(which, customLang)
    assert(which == "core" or which == "mod", "Argument must be 'core' or 'mod'")

    local retStr = (customLang == nil and _GetLocaleNameFromConfig(which) or customLang)

    local names = which == "core" and _LocaleCoreTable or _LocaleModTable
    local englishIndex = 1
    for idx, name in ipairs(names) do
        if name == "English" then
            englishIndex = idx
        end
        if name == retStr then
            return idx - 1
        end
    end

    return englishIndex - 1
end

local function _GetOptionsTable()

    _LocaleCoreTable = _GetLocaleNames("core")
    _LocaleModTable  = _GetLocaleNames("mod")

    return {
        {
            sectionName     = "General",
            {
                title       = "Window Mode:",
                id          = "windowmode",
                type        = UIWIDGET.RADIO,
                optTitle    = {
                    "Fullscreen",
                    "Windowed"
                },
                default     = AL.GetRegistryKey("startinwindow"),
                callback    = function(t)

                    AL.SetRegistryKey("startinwindow", t.value)
                end
            },
            {
                title       = "Rendering mode:",
                id          = "rendermode",
                type        = UIWIDGET.RADIO,
                optTitle    = {
                    "Software",
                    "Hardware"
                },
                default     = AL.GetRegistryKey("2dacceloff"),
                callback    = function(t)

                    local registryValues = {
                        { key = "2dacceloff", val = t.value },
                        { key = "Use D3D",    val = t.value },
                    }

                    for i = 1, #registryValues do
                        local entry = registryValues[i]
                        AL.SetRegistryKey(entry.key, entry.val)
                    end
                end
            },
            {
                title       = "Language (Core Game):",
                id          = "languagecore",
                type        = UIWIDGET.POPUP,
                optTitle    = _LocaleCoreTable,
                default     = _GetLocaleIDFromConfig("core"),
                callback    = function(t)

                    local localeIndex = t.value + 1
                    local matchStr    = _LocaleCoreTable[localeIndex]
                    local localeNew   = nil

                    for _, locale in ipairs(AL_TLocales) do
                        if locale.name == matchStr then
                            if locale.core ~= nil then
                                localeNew = locale
                            end
                            break
                        end
                    end

                    if localeNew == nil then
                        return
                    end

                    AL_LocaliseCoreFreeAll()
                    if localeNew.coreFreeCb then
                        localeNew.coreFreeCb(localeNew)
                    end

                    local coreSuccess = localeNew.coreCb(localeNew)
                    if not coreSuccess then
                        print("Failed to initialize core localisation.")
                        return
                    end

                    AL_WriteConfigLocalisationNames(localeNew.name)
                end
            },
            {
                title       = "Language (Mod):",
                id          = "languagemod",
                type        = UIWIDGET.POPUP,
                optTitle    = _LocaleModTable,
                default     = _GetLocaleIDFromConfig("mod"),
                callback    = function(t)

                    local localeIndex = t.value + 1
                    local matchStr    = _LocaleModTable[localeIndex]
                    local localeNew   = nil

                    for _, locale in ipairs(AL_TLocales) do
                        if locale.name == matchStr then
                            if locale.mod ~= nil then
                                localeNew = locale
                            end
                            break
                        end
                    end

                    if localeNew == nil then
                        return
                    end

                    if localeNew.modFreeCb then
                        localeNew.modFreeCb(localeNew)
                    end

                    local modSuccess = localeNew.modCb(localeNew)
                    if not modSuccess then
                        print("Failed to initialize mod localisation.")
                        return
                    end

                    AL_WriteConfigLocalisationNames(nil, localeNew.name)
                end
            },
        },
        {
            sectionName     = "Mods",
            {
                title       = "Mods:",
                id          = "modslist",
                type        = UIWIDGET.LISTBOX,
                optTitle    = {
                    "David's Stuff",
                },
                default     = 0,
                callback    = function(t)

                    -- @todo
                end
            },
        },
        {
            sectionName     = "Tweaks",
            {
                title       = "View Distance:",
                id          = "tviewdist",
                type        = UIWIDGET.EDIT,
                default     = "16000",
                callback    = function(t)

                    iniPath = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR.."mm7.ini"

                    local ini = AL.INILoad(iniPath)
                    AL.INISet(ini, "Settings", "ViewDistanceD3D", tostring(t.value))
                    AL.INISave(ini, iniPath)
                    AL.INIClose(ini)
                end
            },
        }
    }
end

function ModalShowOptions()

    local optTable   = _GetOptionsTable()
    local uiResponse = AL.UICall(UIEVENT.MODAL_OPTIONS, optTable)
    if uiResponse and uiResponse.status == true then

        for _, section in ipairs(optTable) do
            for _, opt in ipairs(section) do
                local val = uiResponse[opt.id]
                if val ~= nil then
                    opt.value = val
                    --print("Option "..opt.id.." values: "..tostring(opt.value).." (default: "..tostring(opt.default)..")")
                    if (tostring(opt.default) ~= tostring(opt.value)) then
                        --print("Call option!")
                        opt.callback(opt)
                    else
                        --print("Ignoring option...")
                    end
                end
            end
        end
    end
    print(dump(uiResponse))
end
