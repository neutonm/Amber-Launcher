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
function dump(o)
    if type(o) == 'table' then
       local s = '{ '
       for k,v in pairs(o) do
          if type(k) ~= 'number' then k = '"'..k..'"' end
          s = s .. '\n\t['..k..'] = ' .. dump(v) .. ','
       end
       return s .. '\n} '
    else
       return tostring(o)
    end
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
function AppInit()

    print("App Init")
    if _DEBUG then
        AL_print("Running in debug mode")
    end
end

function PostAppInit()

    print("Post App Init")

    local src, how = AL_DetectGame(nil, false)
    if not src then
        AL.UICall(UIEVENT.AUTOCONFIG)
    end
end

function AppDestroy()

    print("App Destroy")
end

function PostAppDestroy()

    print("Post App Destroy")
end

function AppConfigure()

    local configSuccessful = true

    FS.DirectoryEnsure(GAME_DESTINATION_FOLDER)

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

function PostAppConfigure(configSuccessful)
    AL_print("Configuration "..(configSuccessful and "is succesful!" or "failed."))
    if configSuccessful then
        AL.UICall(UIEVENT.AUTOCONFIG,true)
    end
end

function Play()

    print("Starting game...")

    local destDir = (GAME_DESTINATION_FOLDER or ".")
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
