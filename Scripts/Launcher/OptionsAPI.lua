-- FILE:        OptionsAPI.lua

function AL_ValidateOptionValue(option, value, choices)

    if option.type == UIWIDGET.EDIT then return type(value) == "string" end
    if type(value) ~= "number" or value % 1 ~= 0 then return false end
    local count = choices and #choices or 2
    local limit = option.type == UIWIDGET.LISTBOX and 2^count - 1 or count - 1
    if option.type == UIWIDGET.CHECKBOX then limit = 1 end
    return value >= 0 and value <= limit
end

function AL_BuildOptions(options, context, read)

    local sections, byName, displayed, ids = {}, {}, {}, {}
    for _, option in ipairs(options) do
        assert(type(option.id) == "string" and option.id ~= "" and not ids[option.id], "Invalid option ID")
        assert(type(option.title) == "string", "Option title is required")
        assert(type(option.type) == "number" and option.type % 1 == 0 and
            option.type > UIWIDGET.NULL and option.type < UIWIDGET.MAX, "Invalid option widget type")
        ids[option.id] = true
        local choices = option.optTitle
        if type(choices) == "function" then choices = choices(context) end
        assert(choices == nil or type(choices) == "table", "Invalid option choices")
        if option.type == UIWIDGET.RADIO or option.type == UIWIDGET.POPUP or option.type == UIWIDGET.LISTBOX then
            assert(choices ~= nil, "Missing choices for "..option.id)
        end
        if choices == nil or #choices > 0 then
            assert(#displayed < 32, "Too many options")
            assert(choices == nil or #choices <= 8, "Too many option choices")
            if choices then
                for _, title in ipairs(choices) do
                    assert(type(title) == "string", "Option labels must be strings")
                end
            end
            local value = option.default
            if read then value = read(option, context[option.id])
            elseif option.read then value = option.read(context) end
            assert(AL_ValidateOptionValue(option, value, choices), "Invalid default for "..option.id)
            local widget = {id = option.id, title = option.title, type = option.type,
                            optTitle = choices, default = value}
            local name = option.section or "General"
            if not byName[name] then
                byName[name] = {sectionName = name}
                sections[#sections + 1] = byName[name]
            end
            byName[name][#byName[name] + 1] = widget
            displayed[#displayed + 1] = {definition = option, widget = widget}
        end
    end
    return sections, displayed
end

function AL_ReadOptionValues(displayed, values)

    assert(type(values) == "table", "Missing option values")
    local result = {}
    for _, shown in ipairs(displayed) do
        local widget = shown.widget
        local value  = values[widget.id]
        assert(AL_ValidateOptionValue(widget, value, widget.optTitle), "Invalid value for "..widget.id)
        result[widget.id] = value
    end
    return result
end
