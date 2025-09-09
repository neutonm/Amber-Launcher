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

-- Includes
FS = require("Scripts.Launcher._filesystem")

-- Main launcher calls
function OnAppInit()

    print("App Init")
    if _DEBUG then
        AL_print("Running in debug mode")
    end
end

function OnPostAppInit()

    print("Post App Init")

    local src, how = AL_DetectGame(nil, false)
    if not src then
        AL.UICall(UIEVENT.AUTOCONFIG)
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

    print("Starting game...")

    local destDir = (GAME_DESTINATION_PATH or ".")
    local sep     = FS.OS_FILE_SEPARATOR
    destDir       = destDir:gsub(sep.."*$", "")

    if destDir == "." then
        -- extGameFolder was "", so nothing to do
        return
    end

    local fullExePath  = FS.PathJoin(destDir, GAME_EXECUTABLE_NAME)
    local cmd          = AL.GetLaunchCommand()
    if cmd:find(fullExePath, 1, true) then
        return
    end

    -- Replace mm7.exe, preserving quotes
    local pattern = '(["\']?)%f[%w]' ..
                    GAME_EXECUTABLE_NAME:gsub("%.", "%%.") ..
                    '%f[^%w]%1'

    local newCmd, n = cmd:gsub(pattern,
        function(q)
            return (q or "")..fullExePath..(q or "")
        end, 1)

    if n > 0 then
        AL.SetLaunchCommand(newCmd)
    end

    -- Permission for exe on Linux
    if OS_NAME ~= "Windows" and not FS.IsFileExecutable(fullExePath) then
        os.execute(('chmod +x "%s" 2>/dev/null'):format(fullExePath))
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
