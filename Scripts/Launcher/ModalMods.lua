-- FILE:        ModalMods.lua
-- DESCRIPTION: Compatible mods, persistent options and installation.

local modsRoot = FS.PathJoin("Data", "Launcher", "Mods")

local function _Serialize(t, indent)
    indent = indent or ""
    local pad, out = indent.."    ", {"{\n"}
    for k, v in pairs(t) do
        local key = type(k) == "number" and ("[%s]"):format(k) or ("[%q]"):format(k)
        local value = type(v) == "table" and _Serialize(v, pad) or
            type(v) == "string" and ("%q"):format(v) or tostring(v)
        out[#out + 1] = pad..key.." = "..value..",\n"
    end
    out[#out + 1] = indent.."}"
    return table.concat(out)
end

local function _TablesEqual(a, b)
    for k, v in pairs(a) do
        if type(v) == "table" then
            if type(b[k]) ~= "table" or not _TablesEqual(v, b[k]) then return false end
        elseif b[k] ~= v then
            return false
        end
    end
    for k in pairs(b) do
        if a[k] == nil then return false end
    end
    return true
end

local function _LoadTable(path)
    if path and FS.IsFilePresent(path) then
        local ok, values = pcall(dofile, path)
        if ok and type(values) == "table" then return values end
    end
    return {}
end

local function _SaveTable(path, values)
    if not FS.DirectoryEnsure(FS.PathGetDirectory(path)) then return nil, "Cannot create settings directory" end
    local file, err = io.open(path, "w")
    if not file then return nil, err end
    local written, writeError = file:write("return ", _Serialize(values))
    local closed, closeError = file:close()
    if not written or not closed then return nil, writeError or closeError end
    return true
end

local function _SettingsPath(id)
    return FS.PathJoin(GAME_DESTINATION_PATH, "Scripts", "ModSettings", id.."_settings.lua")
end

local function _GetCompatibleGames(manifest)
    if manifest.games ~= nil then return manifest.games end
    if type(manifest.game) == "string" then
        for key, id in pairs(MODID) do
            if id > MODID.NULL and id < MODID.MAX and
                (manifest.game == key or manifest.game == MODID_NAMES[id]) then return {key} end
        end
    end
end

function AL_ScanMods()
    local list = {}
    if GAME_CURRENT == GAME_NULL or not FS.IsFilePresent(modsRoot) then return list end
    for _, dir in ipairs(FS.DirectoryList(modsRoot) or {}) do
        local path = FS.PathJoin(modsRoot, dir, "manifest.lua")
        if FS.IsFilePresent(path) then
            local ok, manifest = pcall(dofile, path)
            if ok and type(manifest) == "table" and AL_IsGameCompatible(_GetCompatibleGames(manifest)) then
                manifest.games = _GetCompatibleGames(manifest)
                manifest.id    = manifest.id or dir
                if type(manifest.id) == "string" and manifest.id:match("^[%w_-]+$") then
                    manifest.root       = FS.PathJoin(modsRoot, dir)
                    manifest.screenshot = FS.PathJoin(manifest.root, "screenshot.jpg")
                    list[#list + 1]      = manifest
                end
            end
        end
    end
    return list
end

local function _FindMod(id)
    assert(type(id) == "string", "modId must be a string")
    for _, mod in ipairs(AL_ScanMods()) do
        if mod.id == id then return mod end
    end
    return nil, "Unknown or incompatible mod: "..id
end

local function _WidgetValue(option, value)
    if option.type == UIWIDGET.CHECKBOX then
        if type(value) == "boolean" then return value and 1 or 0 end
    elseif option.type == UIWIDGET.EDIT then
        if type(value) == "string" then return value end
    elseif option.type == UIWIDGET.LISTBOX then
        if type(value) ~= "table" then return nil end
        local bits, selected = 0, {}
        for _, item in ipairs(value) do selected[item] = true end
        for i, choice in ipairs(option.values or option.optTitle or {}) do
            if selected[choice] then
                bits = bits + 2^(i - 1)
                selected[choice] = nil
            end
        end
        if next(selected) then return nil end
        return bits
    else
        for i, choice in ipairs(option.values or option.optTitle or {}) do
            if choice == value then return i - 1 end
        end
    end
end

local function _RuntimeValue(option, value)
    if option.type == UIWIDGET.CHECKBOX then return value == 1 end
    if option.type == UIWIDGET.EDIT then return value end
    local choices = option.values or option.optTitle
    if option.type == UIWIDGET.LISTBOX then
        local selected = {}
        for i, choice in ipairs(choices) do
            if math.floor(value / 2^(i - 1)) % 2 == 1 then selected[#selected + 1] = choice end
        end
        return selected
    end
    return choices[value + 1]
end

local function _Options(mod, overrides, saved)
    local values = {}
    for _, option in ipairs(mod.options or {}) do
        local value = overrides and overrides[option.id]
        if value == nil then value = saved[option.id] end
        local widget = _WidgetValue(option, value)
        if widget == nil then widget = _WidgetValue(option, option.default) end
        assert(AL_ValidateOptionValue(option, widget, option.optTitle), "Invalid mod option: "..tostring(option.id))
        assert(not option.values or option.optTitle and #option.values == #option.optTitle,
            "Option labels and values must match")
        values[option.id] = _RuntimeValue(option, widget)
    end
    return values
end

local function _ActiveIndex(active, id)
    for i, installed in ipairs(active) do
        if installed == id then return i end
    end
end

local function _CopyRule(mod, rule)
    local source = FS.PathResolveCaseInsensitive(mod.root, assert(rule.src, "Install rule needs src"))
    local relative = rule.dst or rule.src
    local destination = FS.PathResolveCaseInsensitive(GAME_DESTINATION_PATH, relative) or
        FS.PathJoin(GAME_DESTINATION_PATH, relative)
    if not source then return nil, "Source missing: "..rule.src end
    if lfs.attributes(source, "size") == lfs.attributes(destination, "size") then
        local hash = AL.SHA256HashFile(source)
        if hash and hash == AL.SHA256HashFile(destination) then return true end
    end
    if not FS.DirectoryEnsure(FS.PathGetDirectory(destination)) then return nil, "Cannot create directory: "..relative end
    local input, err = io.open(source, "rb")
    if not input then return nil, err end
    local output, openError = io.open(destination, "wb")
    if not output then input:close(); return nil, openError end
    local success = true
    while true do
        local chunk, readError = input:read(65536)
        if not chunk then success = readError == nil; break end
        if not output:write(chunk) then success = false; break end
    end
    local readClosed, writeClosed = input:close(), output:close()
    if not success or not readClosed or not writeClosed then return nil, "Cannot copy: "..relative end
    AL_print("[Mod "..mod.id.."] Copied "..relative)
    return true
end

local function _Delete(relative)
    local path = FS.PathResolveCaseInsensitive(GAME_DESTINATION_PATH, relative)
    if not path then return true end
    if not FS.PathDelete(path) then return nil, "Cannot remove: "..relative end
    return true
end

local function _MakeCtx(mod, options, previous, force)
    local context = {FS = FS, gameRoot = GAME_DESTINATION_PATH, modRoot = mod.root,
                     options = options, previous = previous or {}, force = force or false}
    context.log = function(message) AL_print("[Mod "..mod.id.."] "..tostring(message)) end
    context.copy = function(src, dst)
        local ok, err = _CopyRule(mod, {src = src, dst = dst})
        if not ok then context.error = err end
        return ok, err
    end
    context.remove = function(path)
        local ok, err = _Delete(path)
        if not ok then context.error = err end
        return ok, err
    end
    return context
end

local function _CallHook(mod, name, context)
    if type(mod[name]) ~= "function" then return true end
    local ok, result, err = pcall(mod[name], context)
    if not ok then return nil, result end
    if result == false or (result == nil and err ~= nil) or context.error then return nil, context.error or err or (mod.id..": "..name.." failed") end
    return true
end

local function _InstallManifest(mod, options)
    for _, rule in ipairs(mod.install or {}) do
        local ok, err = _CopyRule(mod, rule)
        if not ok then return nil, err end
    end
    local context = _MakeCtx(mod, options, {}, true)
    local ok, err = _CallHook(mod, "onInstall", context)
    if not ok then return nil, err end
    return _CallHook(mod, "onApply", context)
end

local function _UninstallManifest(mod, options)
    local ok, err = _CallHook(mod, "onUninstall", _MakeCtx(mod, options))
    if not ok then return nil, err end
    for _, rule in ipairs(mod.install or {}) do
        if rule.autoclear then
            ok, err = _Delete(rule.dst or rule.src)
            if not ok then return nil, err end
        end
    end
    return true
end

function AL_LoadActiveMods()
    return _LoadTable(MOD_ACTIVE_PATH)
end

local function _ChangeMod(mod, enabled, overrides, force, active)
    local path     = _SettingsPath(mod.id)
    local saved    = _LoadTable(path)
    local previous = _Options(mod, nil, saved)
    local values   = overrides and _Options(mod, overrides, saved) or previous
    local index    = _ActiveIndex(active, mod.id)
    local changed  = enabled ~= nil and enabled ~= (index ~= nil)
    local ok, err  = true
    if enabled and (changed or force) then
        ok, err = _InstallManifest(mod, values)
    elseif changed then
        ok, err = _UninstallManifest(mod, previous)
    elseif index and not _TablesEqual(previous, values) then
        ok, err = _CallHook(mod, "onApply", _MakeCtx(mod, values, previous))
    end
    if not ok then return nil, err end
    if #(mod.options or {}) > 0 and not _TablesEqual(saved, values) then
        ok, err = _SaveTable(path, values)
        if not ok then return nil, err end
    end
    if changed then
        if enabled then active[#active + 1] = mod.id else table.remove(active, index) end
        return _SaveTable(MOD_ACTIVE_PATH, active)
    end
    return true
end

local function _UpdateMod(id, enabled, overrides, force)
    local mod, err = _FindMod(id)
    if not mod then return nil, err end
    return _ChangeMod(mod, enabled, overrides, force, AL_LoadActiveMods())
end

function AL_IsModActive(id)
    return _FindMod(id) ~= nil and _ActiveIndex(AL_LoadActiveMods(), id) ~= nil
end

function AL_LoadModOptions(id)
    local mod, err = _FindMod(id)
    if not mod then return nil, err end
    return _Options(mod, nil, _LoadTable(_SettingsPath(id)))
end

function AL_SetModOptions(id, overrides)
    return _UpdateMod(id, nil, overrides)
end

function AL_InstallMod(id, overrides)
    return _UpdateMod(id, true, overrides)
end

function AL_UninstallMod(id, overrides)
    return _UpdateMod(id, false, overrides)
end

function AL_ReinstallMod(id, overrides)
    return _UpdateMod(id, true, overrides, true)
end

function ModalShowModManager()
    if GAME_CURRENT == GAME_NULL then return false end
    local game   = GAME_CURRENT
    local mods   = AL_ScanMods()
    local active = AL_LoadActiveMods()
    local prepared, available = {}, {}
    local ok, err = pcall(function()
        for _, mod in ipairs(mods) do
            local values = _Options(mod, nil, _LoadTable(_SettingsPath(mod.id)))
            local form, displayed = AL_BuildOptions(mod.options or {}, values, _WidgetValue)
            prepared[mod.id] = {values = values, displayed = displayed}
            available[#available + 1] = {
                id = mod.id, name = mod.name or mod.id, version = mod.version or "",
                author = mod.author or "", description = mod.description or "",
                website = mod.website or "", screenshot = mod.screenshot, settings = form,
                active = _ActiveIndex(active, mod.id) ~= nil
            }
        end
    end)
    if not ok then
        AL.UICall(UIEVENT.MODAL_MESSAGE, "Cannot read mod settings: "..tostring(err))
        return false
    end
    local response = AL.UICall(UIEVENT.MODAL_MODS, available)
    if not response or not response.status or GAME_CURRENT ~= game then return false end
    ok, err = pcall(function()
        for _, mod in ipairs(mods) do
            local values = response.options and response.options[mod.id]
            if values then
                local widgetValues = AL_ReadOptionValues(prepared[mod.id].displayed, values)
                for _, shown in ipairs(prepared[mod.id].displayed) do
                    local option = shown.definition
                    prepared[mod.id].values[option.id] = _RuntimeValue(option, widgetValues[option.id])
                end
            end
        end
        for _, mod in ipairs(mods) do
            local state = response.active and response.active[mod.id]
            if state ~= nil then
                local applied, applyError = _ChangeMod(mod, state, prepared[mod.id].values, false, active)
                if not applied then error(mod.id..": "..tostring(applyError)) end
            end
        end
    end)
    if not ok then
        AL_print("[ModManager] "..tostring(err))
        AL.UICall(UIEVENT.MODAL_MESSAGE, "Failed to apply mod settings: "..tostring(err))
    end
    return ok
end
