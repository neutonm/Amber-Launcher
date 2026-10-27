events = events or {}
local event_storage = {}

-- Events metatable, imitates MMEXT API calls (event.<whatever>)
setmetatable(events, {
    __index = function(t, key)
        -- When accessing events[key], return the stored value
        return event_storage[key]
    end,
    __newindex = function(t, key, value)
        if type(value) == 'function' then
            local prev = event_storage[key]
            if prev then
                -- Stack the new function with the existing one
                local old_func = prev
                local new_func = function(...)
                    old_func(...)
                    value(...)
                end
                event_storage[key] = new_func
            else
                -- Store the function directly
                event_storage[key] = value
            end
        else
            event_storage[key] = value
        end
    end
})

--- Dump helper function
--- @param o any
function dump(value, level)

    level = level or 0
    local indent      = string.rep('\t', level)
    local indentInner = string.rep('\t', level + 1)

    if type(value) ~= 'table' then
        return tostring(value)
    end

    local out = '{\n'
    for k, v in pairs(value) do
        local key = (type(k) == 'number') and k or ('"' .. k .. '"')
        out = out .. indentInner
                  .. '[' .. key .. '] = '
                  .. dump(v, level + 1)
                  .. ',\n'
    end
    return out .. indent .. '}'
end

--- os.sleep for short delays only
--- @param sec
function sleep(sec)
    local t0 = os.clock()
    while os.clock() - t0 < sec do end
end

---Prints to current textview and duplicates to stdout
---@param msg string
function AL_print(msg)
    AL.UICall(UIEVENT.PRINT, msg)
    print(msg)
end

---Parses version: "major.minor.patch" or "major.minor"
---@param v string
function AL_ParseVersion(v)
    local maj, min, pat = v:match("^v?(%d+)%.(%d+)%.?(%d*)$")
    return tonumber(maj) or 0,
           tonumber(min) or 0,
           tonumber(pat) or 0
end

-- Includes
FS = require("Scripts.Launcher._filesystem")

-- Main launcher calls
function OnAppInit()

    print("App Init")
    if _DEBUG then
        AL_print("Running in debug mode")
    end
end

function OnAppScriptsLoaded()
    AL_DiscoverGames()
    if not AL_SelectCurrentGame() then
        return false
    end

    AL_ApplyCurrentGame()
    if not AL_LoadGameScripts(GAME_CURRENT.scriptsPath) then
        return false
    end
    if not AL_SaveCurrentGame() then
        AL_print("Failed to save launcher preferences.")
        return false
    end
    return true
end

function OnPostAppInit()

    print("Post App Init")

    if GAME_CURRENT == GAME_NULL then
        return
    end

    local src, how = AL_DetectGame(nil, false) -- "."
    if not src or how ~= "local" or
        (GAME_CURRENT.isConfigured and not GAME_CURRENT.isConfigured()) then
        AL.UICall(UIEVENT.AUTOCONFIG)
        return
    end

    -- Set GAME_MOD_VERSION during check (second argument)
    if type(AL_DetectMod) == "function" then
        AL_DetectMod(GAME_DESTINATION_PATH, true)
        print("MOD VERSION: "..tostring(GAME_MOD_VERSION_STR))
    end
end

function OnAppDestroy()

    print("App Destroy")
end

function OnPostAppDestroy()

    print("Post App Destroy")
end

function OnAppConfigure()

    local configSuccessful = true

    FS.DirectoryEnsure(GAME_DESTINATION_PATH)

    -- Execute all available commands
    local commandTable = AL.GetTableOfCommands()
    table.sort(commandTable, function(a, b)
        return a.priority < b.priority
    end)
    print("List of commands: ")
    for _, cmd in ipairs(commandTable) do
        print("\t* "..cmd.name.." ["..cmd.priority.."]")
    end

    -- Execute all commands
    AL_print("Configuration start...")
    for _, cmd in ipairs(commandTable) do
        if not AL.CommandCall(cmd.name) then
            configSuccessful = false
            break
        end
    end
    --AL.CommandCall("DetectAndCopyGame")

    return configSuccessful
end

function OnPostAppConfigure(configSuccessful)
    AL_print("Configuration "..(configSuccessful and "is succesful!" or "failed."))
    if configSuccessful then
        AL.UICall(UIEVENT.AUTOCONFIG,true)
    end
end

function OnPlay()

    local ini            = AL.INILoad(INI_PATH_MOD)
    local closeOnLaunch  = 0
    local command        = GAME_EXECUTABLE_NAME
    if ini then
        closeOnLaunch = tonumber(AL.INIGet(ini, "Settings", "CloseOnLaunch")) or 0
        local saved   = AL.INIGet(ini, GAME_SETTINGS_SECTION, "LaunchCommand")
        if saved and saved ~= "" then
            command = saved
        end
        AL.INIClose(ini)
    end

    local executable = FS.PathResolveCaseInsensitive(GAME_DESTINATION_PATH, GAME_EXECUTABLE_NAME) or
        FS.PathJoin(GAME_DESTINATION_PATH, GAME_EXECUTABLE_NAME)
    local quoted     = OS_NAME == "Windows" and ('"'..executable..'"') or
        ("'"..executable:gsub("'", "'\"'\"'").."'")

    -- Replace only the bare executable token, keeping user-supplied wrappers and
    -- arguments (e.g. wine mm7.exe -windowed). Absolute custom commands stay intact.
    local start = 1
    while true do
        local first, last = command:find(GAME_EXECUTABLE_NAME, start, true)
        if not first then break end
        local left, right = first, last
        local before      = command:sub(first - 1, first - 1)
        if (before == '"' or before == "'") and command:sub(last + 1, last + 1) == before then
            left, right = first - 1, last + 1
        end
        if (left == 1 or command:sub(left - 1, left - 1):match("%s")) and
            (right == #command or command:sub(right + 1, right + 1):match("%s")) then
            command = command:sub(1, left - 1)..quoted..command:sub(right + 1)
            break
        end
        start = last + 1
    end
    AL.SetLaunchCommand(command)
    print("Starting game in "..GAME_DESTINATION_PATH..": "..command)

    if OS_NAME ~= "Windows" and not FS.IsFileExecutable(executable) then
        os.execute("chmod +x "..quoted.." 2>/dev/null")
    end
    if closeOnLaunch > 0 then
        AL.UICall(UIEVENT.EXITAPP)
    end
end

--- Processed side buttons
--- @param buttonID -Button ID (UISIDEBUTTON enum)
--- @return bool    -Proceed with built in button processing on success
function OnSidebuttonClick(buttonID)

    local retVal = true
    if (buttonID == UISIDEBUTTON.SETTINGS) then
        ModalShowOptions()
        retVal = false
    elseif (buttonID == UISIDEBUTTON.MODS) then
        ModalShowModManager()
        retVal = false
    elseif (buttonID == UISIDEBUTTON.TOOLS) then
        ModalShowTools()
        retVal = false
    elseif (buttonID == UISIDEBUTTON.UPDATE) then
        ModalShowUpdater()
        retVal = false
    end

    print("Sidebutton clicked: "..UISIDEBUTTON_NAMES[buttonID])
    return retVal
end

function OnUpdateStart()

end

function OnUpdateEnd()

end
