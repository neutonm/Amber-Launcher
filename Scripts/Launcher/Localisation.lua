-- COMMAND:     Localisation
-- DESCRIPTION: Provides localisation api + defines behavior for localisation modal window during autoconfig

local sep = OS_FILE_SEPARATOR
local archiveFolder     =   "Data" ..
                            sep ..
                            "Launcher" ..
                            sep ..
                            "Archives" ..
                            sep

local destinationFolder =   GAME_DESTINATION_FOLDER ..
                            sep ..
                            "Scripts" ..
                            sep ..
                            "Localization"

local function _LocaliseMod(t)

    local archivePath       = archiveFolder.."mod-"..(t.code)..".zip"
    local files             = {
        "Quests.lua",
        "Common.lua",
        "Scripts.lua"
    }

    print("Mod localisation: "..(t.code).."\n")

    -- Reset localisation if it's english
    if t.code == "en_en" then
        for _, filename in ipairs(files) do
            local path = destinationFolder..filename
            local success, err = os.remove(path)
            if success then
                print("Deleted: "..path)
            end
        end

        return true
    end

    if not AL.ArchiveExtract(archivePath, destinationFolder) then
        print("Failed to extract mod-"..(t.code)..".zip")
        return false
    end

    return true
end

local function _LocaliseCoreFreeAll()

    local files = {
        "Data"..sep.."loc.ICONS.LOD",
        "Data"..sep.."loc.Events.lod",
        "SOUNDS"..sep.."loc.Audio.snd",
    }

    for _, filename in ipairs(files) do
        local path = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR..filename
        local success, err = os.remove(path)
        if success then
            print("Deleted: "..path)
        end
    end

    return true
end

local function _LocaliseCore(t)

    local archivePath = archiveFolder.."core-"..(t.code)..".zip"

    print("Core game localisation: "..(t.code).."\n")

    if not AL.ArchiveExtract(archivePath, GAME_DESTINATION_FOLDER) then
        print("Failed to extract core-"..(t.code)..".zip")
        return false
    end

    return true
end

local _Locales = {
    {
        code        = "en_en",
        core        = "Data"..sep.."Launcher"..sep.."local-en-core.png",
        mod         = "Data"..sep.."Launcher"..sep.."local-en-mod.png",
        coreCb      = _LocaliseCore,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        for _, filename in ipairs(files) do
                            local path = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR..filename
                            local success, err = os.remove(path)
                            if success then
                                print("Deleted: "..path)
                            end
                        end

                        return true
                    end,
        modCb       = _LocaliseMod,
    },
    {
        code        = "pl_pl",
        core        = "Data"..sep.."Launcher"..sep.."local-pl-core.png",
        mod         = "Data"..sep.."Launcher"..sep.."local-pl-mod.png",
        coreCb      = _LocaliseCore,
        modCb       = _LocaliseMod,
    },
    {
        code        = "ru_ru",
        core        = "Data"..sep.."Launcher"..sep.."local-ru-core.png",
        mod         = "Data"..sep.."Launcher"..sep.."local-ru-mod.png",
        coreCb      = _LocaliseCore,
        coreFreeCb  = function(t)

                        local files = {
                            "mm6text.cas",
                            "mm6text.non",
                            "mm7text.dll"
                        }

                        for _, filename in ipairs(files) do
                            local path = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR..filename
                            local success, err = os.remove(path)
                            if success then
                                print("Deleted: "..path)
                            end
                        end

                        return true
                    end,
        modCb       = _LocaliseMod,
    }
}

local function _Localisation()

    print("Setting up localisation for the game...")

    local uiResponse = AL.UICall(UIEVENT.MODAL_LOCALISATION, _Locales)
    if uiResponse and uiResponse.status == true then

        print(dump(uiResponse))

        -- cleanup core first
        _LocaliseCoreFreeAll()
        for _, locale in ipairs(_Locales) do
            if locale.coreFreeCb then locale.coreFreeCb(locale) end
        end

        -- install localisations
        for _, locale in ipairs(_Locales) do
            if uiResponse.core == locale.code then
                if locale.coreCb then
                    local coreSuccess = locale.coreCb(locale)
                    if not coreSuccess then
                        print("Failed to initialize core localisation.")
                        return false
                    end
                end
            end
            if uiResponse.mod == locale.code then
                local modSuccess = locale.modCb(locale)
                if not modSuccess then
                    print("Failed to initialize mod localisation.")
                    return false
                end
            end
        end
    end

    print("Localisation adjusted!")

    return true
end

function events.InitLauncher()

    print("Initialize Localisation.lua")
    AL.CommandAdd("Localisation", _Localisation) -- overwrites internal cmd

end
