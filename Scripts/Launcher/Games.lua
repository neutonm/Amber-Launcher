-- FILE:        Games.lua
-- DESCRIPTION: Games & global mod management
------------------------------------------------------------------------

local launcherScriptsRoot = FS.PathJoin("Scripts", "Launcher")

local function _RegisterGame(def)

    local idx = #GAMEID + 1

    GAMEID[idx]     = def
    GAMEID[def.key] = def
    def.id          = idx

    return idx
end

local function _ValidateGameDef(def, sourcePath)

    if type(def) ~= "table" then
        return false, "return value is not a table"
    end

    local requiredStrings = {
        "key",
        "name",
        "title",
        "exeName",
        "iniName",
        "manifestName"
    }

    for _, k in ipairs(requiredStrings) do
        if type(def[k]) ~= "string" or def[k] == "" then
            return false, ("field '%s' must be a non-empty string"):format(k)
        end
    end

    if not def.name:match("^[%w_-]+$") then
        return false, "name must be a single directory name"
    end

    if type(def.folderNames) ~= "function" then
        return false, "folderNames must be a function"
    end

    if type(def.basePaths) ~= "function" then
        return false, "basePaths must be a function"
    end

    -- test functions
    local ok1, folders = pcall(def.folderNames)
    if not ok1 or type(folders) ~= "table" then
        return false, "folderNames() must return a table"
    end

    local ok2, bases = pcall(def.basePaths)
    if not ok2 or type(bases) ~= "table" then
        return false, "basePaths() must return a table"
    end

    return true
end

local function _LoadGameDef(filePath)

    local chunk, err = loadfile(filePath)
    if not chunk then
        return nil, "loadfile failed: " .. tostring(err)
    end

    if setfenv then
        local env = setmetatable({}, { __index = _G })
        setfenv(chunk, env)
    end

    local ok, result = pcall(chunk)
    if not ok then
        return nil, "runtime error: " .. tostring(result)
    end

    return result
end

-- PUBLIC
-- Compatibility lists use game keys, e.g. {"AMBER", "MM7"}, or {"*"} for all games.
function AL_IsGameCompatible(games, game)

    game = game or GAME_CURRENT
    if type(games) ~= "table" or type(game) ~= "table" or
        game == GAME_NULL or game.key == nil or game.key == GAME_NULL.key then
        return false
    end

    local compatible = false
    local count      = 0
    for index, key in pairs(games) do
        if type(index) ~= "number" or index < 1 or index % 1 ~= 0 then
            return false
        end
        local modID = MODID[key]
        if key ~= "*" and (type(key) ~= "string" or modID == nil or
            modID <= MODID.NULL or modID >= MODID.MAX) then
            return false
        end
        compatible = compatible or key == "*" or key == game.key
        count      = count + 1
    end

    for index = 1, count do
        if games[index] == nil then
            return false
        end
    end

    return compatible
end

-- Select a discovered game using [Settings] CurrentGame in mod.ini.
function AL_SelectCurrentGame()

    GAME_CURRENT = GAME_NULL

    -- An installed game beside the launcher takes precedence over a saved choice.
    -- Specific expansions are checked before their base games in MODID order.
    for id = MODID.NULL + 1, MODID.MAX - 1 do
        for key, modID in pairs(MODID) do
            local game = GAMEID[key]
            if modID == id and game and type(game.detectInstalled) == "function" and
                game.detectInstalled(LAUNCHER_PATH) then
                GAME_CURRENT = game
                GAME_LAYOUT  = "local"
                print(("Selected local game: %s (%s)"):format(game.title, game.key))
                return true
            end
        end
    end
    GAME_LAYOUT = "managed"

    local configuredKey
    local ini = AL.INILoad(INI_PATH_MOD)
    if ini then
        configuredKey = AL.INIGet(ini, "Settings", "CurrentGame")
        AL.INIClose(ini)
    end

    local gamesByModID = {}
    for key, id in pairs(MODID) do
        if id > MODID.NULL and id < MODID.MAX then
            local game = GAMEID[key]
            if game and game ~= GAME_NULL then
                gamesByModID[id] = game
                if key == configuredKey then
                    GAME_CURRENT = game
                    print(("Selected game: %s (%s)"):format(GAME_CURRENT.title, GAME_CURRENT.key))
                    return true
                end
            end
        end
    end

    local defaultGame = gamesByModID[MODID[GAME_DEFAULT_KEY]]
    if defaultGame then
        GAME_CURRENT = defaultGame
        print(("Selected game: %s (%s)"):format(GAME_CURRENT.title, GAME_CURRENT.key))
        return true
    end

    for id = MODID.NULL + 1, MODID.MAX - 1 do
        if gamesByModID[id] then
            GAME_CURRENT = gamesByModID[id]
            print(("Selected game: %s (%s)"):format(GAME_CURRENT.title, GAME_CURRENT.key))
            return true
        end
    end

    AL.EventCall("GamesNotFound")
    return false
end

function AL_ApplyCurrentGame()

    local game  = GAME_CURRENT
    assert(game ~= GAME_NULL, "No game selected")

    GAME_DESTINATION_FOLDER = GAME_LAYOUT == "local" and "" or FS.PathJoin("Games", game.name)
    GAME_DESTINATION_PATH   = GAME_LAYOUT == "local" and LAUNCHER_PATH or
        FS.PathJoin(LAUNCHER_PATH, GAME_DESTINATION_FOLDER)
    GAME_SETTINGS_SECTION   = GAME_LAYOUT == "local" and "Settings" or "Game."..game.key

    GAME_MOD_ID             = MODID[game.key]
    GAME_MOD_ID_NAME        = game.name
    GAME_MOD_ID_TITLE       = game.title
    GAME_MOD_VERSION        = 0
    GAME_MOD_VERSION_STR    = "0.0.0"
    GAME_EXECUTABLE_NAME    = game.exeName
    GAME_FOLDER_NAME        = game.folderNames()
    GAME_BASE_PATHS         = game.basePaths()
    GAME_EXECUTABLE_FOLDERS = {}

    for _, basePath in ipairs(GAME_BASE_PATHS) do
        for _, folderName in ipairs(GAME_FOLDER_NAME) do
            local root = basePath:gsub("[/\\]+$", "")
            local path = FS.PathJoin(root, folderName, GAME_EXECUTABLE_NAME)
            table.insert(GAME_EXECUTABLE_FOLDERS, FS.PathNormalize(path))
        end
    end

    INI_PATH_GAME               = FS.PathJoin(GAME_DESTINATION_PATH, game.iniName)
    INI_PATH_MM7                = INI_PATH_GAME -- Compatibility with existing options and tweaks.
    MOD_ACTIVE_PATH            = FS.PathJoin(GAME_DESTINATION_PATH, "Data", "Launcher", "Mods", "_active.lua")
    URL_UPDATER_MOD_MANIFEST    = URL_UPDATER_ROOT..game.manifestName

    AL.SetLaunchCommand(game.exeName)
end

-- Root preferences are shared; managed launch overrides live in Game.<KEY> sections.
function AL_SaveCurrentGame()

    if not FS.IsFilePresent(INI_PATH_MOD) then
        local file, err = io.open(INI_PATH_MOD, "wb")
        if not file then
            AL_print("Failed to create launcher config: "..tostring(err))
            return false
        end
        local written = file:write("[Settings]\nCloseOnLaunch=1\n[Localisation]\nCore=English\nMod=English\n")
        local closed  = file:close()
        if not written or not closed then
            return false
        end
    end
    local ini = AL.INILoad(INI_PATH_MOD)
    if not ini then
        return false
    end
    AL.INISet(ini, "Settings", "CurrentGame", GAME_CURRENT.key)
    local saved = AL.INISave(ini, INI_PATH_MOD)
    AL.INIClose(ini)
    return saved and true or false
end

function AL_LoadGameScripts(folder)

    local files = FS.FilesList(folder)
    table.sort(files)

    for _, file in ipairs(files) do
        if file:lower():match("%.lua$") and file ~= "_game.lua" then
            local full = FS.PathJoin(folder, file)

            local chunk, err = loadfile(full)
            if not chunk then
                print("Failed to load "..full..": "..tostring(err))
                return false
            else
                local ok, runtimeErr = pcall(chunk)
                if not ok then
                    print("Error running "..full..": "..tostring(runtimeErr))
                    return false
                else
                    print("Loaded script: "..full)
                end
            end
        end
    end

    return true
end

function AL_DiscoverGames()

    local dirs = FS.DirectoryList(launcherScriptsRoot) or {}

    for _, dirName in ipairs(dirs) do

        local fullDir = FS.PathJoin(launcherScriptsRoot, dirName)

        local attr = lfs.attributes(fullDir)
        if attr and attr.mode == "directory" then
            local gameFile = FS.PathJoin(fullDir, "_game.lua")

            if FS.IsFilePresent(gameFile) == false then
                print("Skipping "..fullDir..": missing _game.lua")
            else
                local def, loadErr = _LoadGameDef(gameFile)
                if not def then
                    print(("Skipping %s: %s"):format(gameFile, loadErr))
                else
                    local ok, why = _ValidateGameDef(def, gameFile)
                    if not ok then
                        print(("Skipping %s: invalid _game.lua (%s)"):format(gameFile, why))
                    elseif GAMEID[def.key] ~= nil then
                        print(("Skipping %s: duplicate key '%s'"):format(gameFile, def.key))
                    else
                        def.scriptsPath = fullDir
                        local idx = _RegisterGame(def)
                        print(("Loaded GAMEID[%d] / GAMEID['%s'] from %s"):format(idx, def.key, gameFile))
                    end
                end
            end
        end
    end

end

function events.GamesNotFound()
    -- Placeholder UI notification until the missing-games screen is implemented.
    AL_print("No supported games were found. Add a valid game definition under Scripts/Launcher.")
end
