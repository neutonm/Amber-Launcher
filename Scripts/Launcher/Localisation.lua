-- COMMAND:     Localisation
-- DESCRIPTION: Provides localisation api + defines behavior for localisation modal window during autoconfig

local sep               =   OS_FILE_SEPARATOR
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

function AL_LocaliseCoreFreeAll()

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

AL_TLocales = {
    {
        name        = "English",
        code        = "en_en",
        core        = "Data"..sep.."Launcher"..sep.."local-en-core.png",
        mod         = "Data"..sep.."Launcher"..sep.."local-en-mod.png",
        coreCb      = function(t)
                        return true
                    end,
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
        name        = "Polish",
        code        = "pl_pl",
        core        = "Data"..sep.."Launcher"..sep.."local-pl-core.png",
        mod         = "Data"..sep.."Launcher"..sep.."local-pl-mod.png",
        coreCb      = _LocaliseCore,
        modCb       = _LocaliseMod,
    },
    {
        name        = "Russian",
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

    print("Setting up localisation for the game...")

    local uiResponse = AL.UICall(UIEVENT.MODAL_LOCALISATION, AL_TLocales)
    if uiResponse and uiResponse.status == true then

        print(dump(uiResponse))

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
                        print("Failed to initialize core localisation.")
                        return false
                    end
                    langCore = locale.name
                end
            end
            if uiResponse.mod == locale.code then
                local modSuccess = locale.modCb(locale)
                if not modSuccess then
                    print("Failed to initialize mod localisation.")
                    return false
                end
                langMod = locale.name
            end
        end
    end

    AL_WriteConfigLocalisationNames(langCore, langMod)
    print("Localisation adjusted!")

    return true
end

function events.InitLauncher()

    print("Initialize Localisation.lua")
    AL.CommandAdd("Localisation", _Localisation) -- overwrites internal cmd

end
