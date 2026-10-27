-- FILE:        ModalTools.lua
-- DESCRIPTION: Provides options for launching external apps and tools

local _ToolsTable = {}
local _ToolsByKey = {}
local _NextToolID = 0

-- Register from events.InitLauncher in shared or game-specific scripts.
-- A repeated key replaces the tool while retaining its numeric GUI ID and order.
function AL_RegisterTool(tool)

    assert(type(tool) == "table", "Tool definition must be a table")
    for _, field in ipairs({"key", "title", "description", "iconPath"}) do
        assert(type(tool[field]) == "string" and tool[field] ~= "",
            "Tool field '"..field.."' must be a non-empty string")
    end
    assert(type(tool.onClick) == "function", "Tool onClick must be a function")
    assert(tool.iconDarkPath == nil or type(tool.iconDarkPath) == "string",
        "Tool iconDarkPath must be a string")
    assert(type(tool.games) == "table" and #tool.games > 0,
        "Tool games must be a non-empty compatibility list")

    local games = {}
    local count = 0
    for index in pairs(tool.games) do
        assert(type(index) == "number" and index >= 1 and index % 1 == 0,
            "Tool games must be an array of game keys")
        count = count + 1
    end
    for index, key in ipairs(tool.games) do
        local modID = MODID[key]
        assert(key == "*" or (type(key) == "string" and modID ~= nil and
            modID > MODID.NULL and modID < MODID.MAX),
            "Unknown game key in tool compatibility list: "..tostring(key))
        games[index] = key
    end
    assert(#games == count, "Tool games must not contain gaps")

    local previous = _ToolsByKey[tool.key]
    if not previous then
        _NextToolID = _NextToolID + 1
    end
    local id    = previous and previous.id or _NextToolID
    local entry = {
        id           = id,
        key          = tool.key,
        games        = games,
        iconPath     = tool.iconPath,
        iconDarkPath = tool.iconDarkPath or tool.iconPath,
        title        = tool.title,
        description  = tool.description,
        onClick      = tool.onClick
    }

    if previous then
        for i, existing in ipairs(_ToolsTable) do
            if existing == previous then
                _ToolsTable[i] = entry
                break
            end
        end
    else
        _ToolsTable[#_ToolsTable + 1] = entry
    end
    _ToolsByKey[tool.key] = entry

    return id
end

function AL_UnregisterTool(key)

    local tool = _ToolsByKey[key]
    if not tool then
        return false
    end
    for i, existing in ipairs(_ToolsTable) do
        if existing == tool then
            table.remove(_ToolsTable, i)
            break
        end
    end
    _ToolsByKey[key] = nil
    return true
end

function AL_GetToolsForCurrentGame()

    local tools = {}
    for _, tool in ipairs(_ToolsTable) do
        if AL_IsGameCompatible(tool.games) then
            -- Pass only the GUI fields, with a callback that rechecks compatibility.
            local entry = {
                id           = tool.id,
                iconPath     = tool.iconPath,
                iconDarkPath = tool.iconDarkPath,
                title        = tool.title,
                description  = tool.description,
                onClick      = function()
                    if _ToolsByKey[tool.key] == tool and AL_IsGameCompatible(tool.games) then
                        return tool.onClick()
                    end
                end
            }
            tools[#tools + 1] = entry
        end
    end

    return tools
end

function ModalShowTools()

    local tools      = AL_GetToolsForCurrentGame()
    local uiResponse = AL.UICall(UIEVENT.MODAL_TOOLS, tools)
    if uiResponse and uiResponse.status == true then
        print(dump(uiResponse))
    end
end

function events.InitLauncher()
    AL_RegisterTool({
        key          = "autoconfig",
        games        = {"*"},
        iconPath     = "Data/Launcher/tool-autoconfig.png",
        iconDarkPath = "Data/Launcher/tool-autoconfig_dark.png",
        title        = "Autoconfig",
        description  = "Re-launches the automatic configuration wizard, letting you fine-tune performance, graphics, and gameplay settings in one guided flow.",
        onClick      = function()
            AL.UICall(UIEVENT.MODAL_CLOSE)
            AL.UICall(UIEVENT.AUTOCONFIG)
        end
    })
end
