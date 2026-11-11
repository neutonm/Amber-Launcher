-- FILE:        ModalOptions.lua
-- DESCRIPTION: Shared Settings registry and INI lifetime.

local _Options      = {}
local _OptionsByID  = {}
local _Generation   = 0

-- Readers run when the modal opens.
function AL_RegisterOption(option)

    assert(type(option) == "table", "Option definition must be a table")
    for _, field in ipairs({"id", "section", "title"}) do
        assert(type(option[field]) == "string" and option[field] ~= "",
            "Option field '"..field.."' must be a non-empty string")
    end
    assert(option.id ~= "status", "Option ID 'status' is reserved by the GUI")
    assert(type(option.type) == "number" and option.type > UIWIDGET.NULL and
        option.type < UIWIDGET.MAX and option.type % 1 == 0, "Invalid option widget type")
    assert(type(option.read) == "function", "Option read must be a function")
    assert(type(option.callback) == "function", "Option callback must be a function")
    assert(option.optTitle == nil or type(option.optTitle) == "table" or
        type(option.optTitle) == "function", "Option optTitle must be a table or function")

    local entry = {}
    for key, value in pairs(option) do
        entry[key] = value
    end
    if type(option.optTitle) == "table" then
        entry.optTitle = {}
        for i, title in ipairs(option.optTitle) do
            entry.optTitle[i] = title
        end
    end

    local previous = _OptionsByID[entry.id]
    if previous then
        for i, existing in ipairs(_Options) do
            if existing == previous then
                _Options[i] = entry
                break
            end
        end
    else
        _Options[#_Options + 1] = entry
    end
    _OptionsByID[entry.id] = entry
    return entry.id
end

function AL_UnregisterOption(id)

    local entry = _OptionsByID[id]
    if not entry then
        return false
    end
    for i, existing in ipairs(_Options) do
        if existing == entry then
            table.remove(_Options, i)
            break
        end
    end
    _OptionsByID[id] = nil
    return true
end

local function _BuildOptions(context)

    local sections  = {}
    local byName    = {}
    local displayed = {}

    for _, option in ipairs(_Options) do
        -- Match MAX_OPT_ELEMS in the native Settings modal.
        if #displayed >= 32 then
            break
        end
        local choices = option.optTitle
        if type(choices) == "function" then
            choices = choices(context)
        end
        -- Empty choice providers (e.g. no available languages) have no control.
        if choices == nil or #choices > 0 then
            local widget = {
                id       = option.id,
                title    = option.title,
                type     = option.type,
                optTitle = choices,
                default  = option.read(context)
            }
            local section = byName[option.section]
            if not section then
                section                 = {sectionName = option.section}
                byName[option.section]  = section
                sections[#sections + 1] = section
            end
            section[#section + 1]     = widget
            displayed[#displayed + 1] = {definition = option, widget = widget}
        end
    end
    return sections, displayed
end

function ModalShowOptions()

    if GAME_CURRENT == GAME_NULL then return false end

    local game       = GAME_CURRENT
    local generation = _Generation
    local context    = AL_ConfigContext()
    local ok, data, displayed = pcall(_BuildOptions, context)
    context:Close(false)
    if not ok then
        AL_print("[Error]: failed to read settings: "..tostring(data))
        return false
    end

    local response = AL.UICall(UIEVENT.MODAL_OPTIONS, data)
    if not response or response.status ~= true or GAME_CURRENT ~= game or generation ~= _Generation then
        return false
    end

    -- Reopen after the modal, so unrelated edits made while it was open survive.
    local success = true
    local applied, err = pcall(function()
        for _, shown in ipairs(displayed) do
            local option = shown.definition
            local widget = shown.widget
            local value  = response[option.id]
            if _OptionsByID[option.id] == option and
                value ~= nil and tostring(value) ~= tostring(widget.default) then
                local valid = option.type == UIWIDGET.EDIT and type(value) == "string"
                if option.type ~= UIWIDGET.EDIT and type(value) == "number" then
                    local count = widget.optTitle and #widget.optTitle or 2
                    valid       = value % 1 == 0 and value >= 0 and value < count
                end
                if valid then
                    widget.value = value
                    if option.callback(widget, context) == false then
                        success = false
                    end
                end
            end
        end
    end)
    local saved = context:Close(applied)
    if not applied then
        AL_print("[Error]: failed to apply settings: "..tostring(err))
    end
    return applied and saved and success
end

function events.GameDeactivated()

    _Generation = _Generation + 1
end

function events.InitLauncher()

    AL_RegisterOption({
        id       = "cmdargs",
        section  = "General",
        title    = "Launch arguments:",
        type     = UIWIDGET.EDIT,
        read     = function(context)
            return context:Get("project", "Settings", "LaunchCommand", AL.GetLaunchCommand())
        end,
        callback = function(option, context)
            AL.SetLaunchCommand(option.value)
            context:Set("project", "Settings", "LaunchCommand", option.value)
        end
    })
    AL_RegisterOption({
        id       = "closeonlaunch",
        section  = "General",
        title    = "Close App On Game Launch:",
        type     = UIWIDGET.CHECKBOX,
        read     = function(context)
            return tonumber(context:Get("launcher", "Settings", "CloseOnLaunch", "0")) == 1 and 1 or 0
        end,
        callback = function(option, context)
            context:Set("launcher", "Settings", "CloseOnLaunch", option.value)
        end
    })
end
