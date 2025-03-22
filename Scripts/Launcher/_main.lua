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

-- Dump helper function
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

-- Main launcher calls
function AppInit()

    print("App Init")
end

function AppDestroy()

    print("App Destroy")
end

function AppConfigure()

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
    print("Configuration start...")
    for _, cmd in ipairs(commandTable) do
        --AL.CommandCall(cmd.name)
    end

    --temp
    AL.CommandCall("DetectAndCopyGame")
    --AL.CommandCall("MergeAndRename")
    --AL.CommandCall("DetectAndInstallMod")
    
    print("Configuration Successful!")
end

