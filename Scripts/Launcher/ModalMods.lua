-- FILE:        ModalModManager.lua
-- DESCRIPTION: Mod Manager – discovery, (un)install & option tweaking
------------------------------------------------------------------------
-- Modal Mod Manager

local modsRoot         = FS.PathJoin("Data", "Launcher", "Mods")
local gameScriptsRoot  = FS.PathJoin(GAME_DESTINATION_FOLDER, "Scripts")
local activeLua        = FS.PathJoin(modsRoot, "_active.lua")

local function _Serialize(t, indent)
    indent = indent or ""
    local pad = indent .. "    "
    local buff = {"{\n"}
    for k, v in pairs(t) do
        local key = ("[%q]"):format(k)
        local val
        if type(v) == "table" then
            val = _Serialize(v, pad)
        elseif type(v) == "string" then
            val = ("%q"):format(v)
        else
            val = tostring(v)
        end
        buff[#buff + 1] = pad .. key .. " = " .. val .. ",\n"
    end
    buff[#buff + 1] = indent .. "}"
    return table.concat(buff)
end

local function _TablesEqual(a, b)
    for k, v in pairs(a) do
        if b[k] ~= v then
            return false
        end
    end
    for k, v in pairs(b) do
        if a[k] ~= v then
            return false
        end
    end
    return true
end

--- Scans for mods with a valid `manifest.lua` and returns normalized manifests (`id`, `root`, `screenshot`).
-- @treturn {table,...} List of manifest tables
function AL_ScanMods()
    local list = {}
    for _, dir in ipairs(FS.DirectoryList(modsRoot) or {}) do
        local manifestPath = FS.PathJoin(modsRoot, dir, "manifest.lua")
        if FS.IsFilePresent(manifestPath) then
            local ok, manifest = pcall(dofile, manifestPath)
            if ok and type(manifest) == "table" and manifest.game == GAME_MOD_ID then
                manifest.id         = manifest.id or dir
                manifest.root       = FS.PathJoin(modsRoot, dir)
                manifest.screenshot = FS.PathJoin(manifest.root, "screenshot.jpg")
                list[#list + 1] = manifest
            end
        end
    end
    return list
end

local function _CopyRule(m, rule)
    local srcRel = assert(rule.src, "install rule needs .src")
    local dstRel = rule.dst or srcRel

    local srcAbs = FS.PathJoin(m.root, srcRel)
    local dstAbs = FS.PathJoin(GAME_DESTINATION_FOLDER, dstRel)

    FS.DirectoryEnsure(FS.PathGetDirectory(dstAbs))

    local r = io.open(srcAbs, "rb")
    if not r then
        print("[ModManager] source missing: " .. srcRel)
        return
    end
    local w = io.open(dstAbs, "wb")
    if not w then
        r:close()
        print("[ModManager] cannot write: " .. dstRel)
        return
    end
    w:write(r:read("*all"))
    r:close()
    w:close()
    print(string.format(
        "[ModManager] Copied %-32s -> %s",
        srcRel, dstRel
    ))
end

local function _DeleteRule(rule)
    FS.PathDelete(FS.PathJoin(GAME_DESTINATION_FOLDER, rule.dst))
end

local function _MakeCtx(m, opts)
    return {
        FS       = FS,
        gameRoot = GAME_DESTINATION_FOLDER,
        modRoot  = m.root,
        options  = opts or {},
        log      = function(...) print("[Mod " .. m.id .. "]", ...) end,
    }
end

local function _InstallManifest(m, opts)
    for _, r in ipairs(m.install or {}) do
        _CopyRule(m, r)
    end
    if type(m.onInstall) == "function" then
        pcall(m.onInstall, _MakeCtx(m, opts))
    end
end

local function _UninstallManifest(m, opts)
    for _, r in ipairs(m.install or {}) do
        if r.autoclear then
            _DeleteRule(r)
        end
    end
    FS.PathDelete(FS.PathJoin(gameScriptsRoot, "ModSettings", m.id .. "_settings.lua"))
    if type(m.onUninstall) == "function" then
        pcall(m.onUninstall, _MakeCtx(m, opts))
    end
end

local function _SaveSettings(modId, tbl)
    local dir = FS.PathJoin(gameScriptsRoot, "ModSettings")
    FS.DirectoryEnsure(dir)
    local f = io.open(FS.PathJoin(dir, modId .. "_settings.lua"), "w")
    f:write("return ", _Serialize(tbl))
    f:close()
end

--- Loads the active mods table
-- @treturn table Active mods table (empty if missing/invalid)
function AL_LoadActiveMods()
    if FS.IsFilePresent(activeLua) then
        local ok, t = pcall(dofile, activeLua)
        if ok and type(t) == "table" then
            return t
        end
    end
    return {}
end

local function _SaveActive(list)
    local f = io.open(activeLua, "w")
    f:write("return ", _Serialize(list))
    f:close()
end

local function _DebugList(mods, activeSet)
    print("[ModManager] Discovered mods:")
    for _, m in ipairs(mods) do
        local status = activeSet[m.id] and "ACTIVE" or "inactive"
        print(("  • %-10s  %-30s  v%s  [%s]"):format(m.id, m.name or "", m.version or "?", status))
    end
end

-- Extra Public api functions
function AL_InstallMod(modId, opts)
    assert(type(modId) == "string", "modId must be a string")

    -- Discover and index mods
    local byId = {}
    for _, m in ipairs(AL_ScanMods()) do
        byId[m.id] = m
    end

    local m = byId[modId]
    if not m then
        return nil, "Unknown mod: " .. tostring(modId)
    end

    local active = AL_LoadActiveMods()
    for _, id in ipairs(active) do
        if id == modId then
            return true -- already installed
        end
    end

    -- Install
    _InstallManifest(m, opts or {})
    active[#active + 1] = modId
    _SaveActive(active)

    return true
end

--- Uninstalls the given mod if currently active.
-- @tparam      string modId The ID of the mod to uninstall
-- @tparam[opt] table opts Runtime option overrides
-- @treturn[1]  true When uninstallation succeeds or mod already inactive
-- @treturn[1]  nil|string errmsg Error description on failure
function AL_UninstallMod(modId, opts)
    assert(type(modId) == "string", "modId must be a string")

    local byId = {}

    for _, m in ipairs(AL_ScanMods()) do
        byId[m.id] = m
    end

    local m = byId[modId]
    if not m then
        return nil, "Unknown mod: " .. tostring(modId)
    end

    local active = AL_LoadActiveMods()
    local idx
    for i, id in ipairs(active) do
        if id == modId then
            idx = i break
        end
    end

    if not idx then
        return true
    end

    -- Uninstall
    _UninstallManifest(m, opts or {})
    table.remove(active, idx)
    _SaveActive(active)
    return true
end

--- Reinstalls the given mod by performing an uninstall (if necessary) followed by a fresh install.
-- @tparam      string modId The ID of the mod to reinstall
-- @tparam[opt] table opts Runtime option overrides to apply during (re)install
-- @treturn[1]  true When reinstall succeeds
-- @treturn[1]  nil|string errmsg Error description on failure
function AL_ReinstallMod(modId, opts)

    local ok, err = AL_UninstallMod(modId, opts)

    if not ok and err then
        return nil, err
    end

    return AL_InstallMod(modId, opts)
end

--- Checks whether the given mod is currently active.
-- @tparam string modId
-- @treturn boolean
function AL_IsModActive(modId)

    for _, id in ipairs(AL_LoadActiveMods()) do
        if id == modId then
            return true
        end
    end
    return false
end

-- Manager
local modsTable = {}

function ModalShowModManager()
    local mods      = AL_ScanMods()
    local activeSeq = AL_LoadActiveMods()
    local activeSet = {}
    for _, id in ipairs(activeSeq) do
        activeSet[id] = true
    end
    _DebugList(mods, activeSet)

    modsTable.available = mods
    modsTable.active    = activeSeq

    local rsp = AL.UICall(UIEVENT.MODAL_MODS, modsTable)
    if not (rsp and rsp.status) then
        return
    end

    local newSet, newSeq, seen = {}, {}, {}
    if type(rsp.active) == "table" and not (type(rsp.active[1]) == "string") then
        for _, id in ipairs(activeSeq) do
            if rsp.active[id] then
                newSet[id], newSeq[#newSeq + 1], seen[id] = true, id, true
            end
        end
        for id, state in pairs(rsp.active) do
            if state and not seen[id] then
                newSet[id], newSeq[#newSeq + 1] = true, id
            end
        end
    else
        for _, id in ipairs(rsp.active or {}) do
            newSet[id], newSeq[#newSeq + 1] = true, id
        end
    end

    local function _ExtractRuntime(schema)
        local out = {}
        for k, s in pairs(schema) do
            if type(s) == "table" then
                local v = s.value ~= nil and s.value or s.default
                if v ~= nil then
                    out[k] = v
                end
            end
        end
        return out
    end

    local runtimeByMod = {}
    local byId = {}
    for _, m in ipairs(mods) do
        byId[m.id] = m
    end

    if rsp.options then
        for modId, schema in pairs(rsp.options) do
            local opts = _ExtractRuntime(schema)
            runtimeByMod[modId] = opts

            local prev = {}
            local path = FS.PathJoin(gameScriptsRoot, "ModSettings", modId .. "_settings.lua")
            if FS.IsFilePresent(path) then
                local ok, t = pcall(dofile, path)
                if ok and type(t) == "table" then
                    prev = t
                end
            end

            if not _TablesEqual(prev, opts) then
                local m = byId[modId]
                if m and type(m.onOption) == "function" then
                    for k, v in pairs(opts) do
                        if prev[k] ~= v then
                            pcall(m.onOption, _MakeCtx(m, opts), k, prev[k], v)
                        end
                    end
                end
            end

            if next(opts) then
                _SaveSettings(modId, opts)
            end
        end
    end

    for id, _ in pairs(newSet) do
        if not activeSet[id] then
            _InstallManifest(byId[id], runtimeByMod[id] or {})
        end
    end
    for id, _ in pairs(activeSet) do
        if not newSet[id] then
            _UninstallManifest(byId[id], runtimeByMod[id] or {})
        end
    end

    _SaveActive(newSeq)
end

function events.InitLauncher()
    --
end

