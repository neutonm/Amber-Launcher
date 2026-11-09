-- FILE:        GamesAPI.lua
-- DESCRIPTION: Shared game detection, copying and file normalization.

local function _NormalizePaths(files)

    local result = {}
    for i, path in ipairs(files) do
        result[i] = FS.PathNormalize(path)
    end
    return result
end

function AL_DetectGameFiles(options)

    local destination = options.destination
    local files       = _NormalizePaths(options.files)
    local searchPaths = options.searchPaths or {}

    AL_print("Detecting game...")
    print("Search path: "..dump(searchPaths))

    if FS.PathResolveCaseInsensitive(destination, options.exeName) then
        if FS.FilesCheck(destination, files) then
            return destination, "local"
        end
        AL_print("Game in destination is incomplete; continuing search.")
    end

    for _, candidate in ipairs(searchPaths) do
        local path = FS.PathNormalize(candidate)
        local attr = lfs.attributes(path)
        if attr then
            local root = attr.mode == "directory" and path or FS.PathGetDirectory(path)
            if FS.PathResolveCaseInsensitive(root, options.exeName) then
                if FS.FilesCheck(root, files) then
                    AL_print("Found valid installation at "..root)
                    return root, "external"
                end
                AL_print("Game data files are incomplete at "..root)
            end
        end
    end

    return nil, "notfound"
end

function AL_CopyGameFiles(options)

    AL_print("Copying game from '"..options.source.."'...")
    if not FS.DirectoryEnsure(options.destination) then
        AL_print("Failed to create game directory: "..options.destination)
        return false
    end

    for _, path in ipairs(options.files) do
        if not FS.FileCopy(options.source, options.destination, FS.PathNormalize(path)) then
            AL_print("Failed to copy: "..path)
            return false
        end
    end

    AL_print("Copy succeeded.")
    return true
end

function AL_DetectAndCopyGameFiles(options)

    local detection = {
        destination = options.destination,
        exeName     = options.exeName,
        searchPaths = options.searchPaths,
        files       = options.files
    }

    while true do
        local source, location = AL_DetectGameFiles(detection)
        if source then
            if location == "local" then
                AL_print("No copy needed - game verified in destination folder.")
                return true
            end

            sleep(1)
            return AL_CopyGameFiles({
                source      = source,
                destination = options.destination,
                files       = options.files
            })
        end

        AL_print("Failed to find the game!")
        local currentDir = FS.CurrentDir()
        local response   = AL.UICall(UIEVENT.MODAL_GAMENOTFOUND)
        local restored   = lfs.chdir(currentDir)
        if not restored then
            AL_print("Failed to restore launcher directory: "..currentDir)
            return false
        end
        if not response or response.status ~= true or not response.path or response.path == "" then
            return false
        end
        detection.searchPaths = {response.path}
    end
end

function AL_VerifyGameFiles(destination, files)

    if not FS.FilesCheck(destination, _NormalizePaths(files)) then
        AL_print("Game file verification failed!")
        return false
    end

    AL_print("Game file verification succeeded!")
    return true
end

local function _GroupNames(directory, mode)

    local groups = {}
    for _, name in ipairs(FS.DirectoryList(directory)) do
        if lfs.attributes(FS.PathJoin(directory, name), "mode") == mode then
            local key = name:lower()
            groups[key] = groups[key] or {}
            table.insert(groups[key], name)
        end
    end
    return groups
end

local function _FindVariants(names, expected)

    local found    = false
    local variants = {}
    for _, name in ipairs(names or {}) do
        if name == expected then
            found = true
        else
            table.insert(variants, name)
        end
    end
    return found, variants
end

local function _NormalizeFolders(directory, names)

    local groups = _GroupNames(directory, "directory")
    for _, name in ipairs(names) do
        local destination     = FS.PathJoin(directory, name)
        local found, variants = _FindVariants(groups[name:lower()], name)

        if not found then
            if #variants > 0 then
                local source = FS.PathJoin(directory, variants[1])
                local ok, err = os.rename(source, destination)
                if not ok then
                    AL_print("Failed to rename folder '"..source.."': "..tostring(err))
                    return false
                end
                table.remove(variants, 1)
            elseif not FS.DirectoryEnsure(destination) then
                AL_print("Failed to create folder: "..destination)
                return false
            end
        end

        for _, variant in ipairs(variants) do
            local source = FS.PathJoin(directory, variant)
            if not FS.FilesMove(source, destination) then
                AL_print("Failed to merge folder: "..source)
                return false
            end
            if not lfs.rmdir(source) then
                AL_print("Failed to remove merged folder: "..source)
                return false
            end
        end
    end
    return true
end

local function _MoveFile(source, destination, overwrite)

    if os.rename(source, destination) then
        return true
    end
    if not overwrite then
        return false
    end

    -- Windows cannot rename over an existing file.
    local input = io.open(source, "rb")
    if not input then return false end
    local contents = input:read("*all")
    input:close()
    if not contents then return false end

    local output = io.open(destination, "wb")
    if not output then return false end
    local written = output:write(contents)
    local closed  = output:close()
    if not written or not closed then return false end

    return os.remove(source) and true or false
end

local function _NormalizeFiles(directory, names)

    local groups = _GroupNames(directory, "file")
    for _, name in ipairs(names) do
        local destination     = FS.PathJoin(directory, name)
        local found, variants = _FindVariants(groups[name:lower()], name)

        if not found and #variants == 0 then
            AL_print("File is missing: "..destination)
            return false
        end

        for _, variant in ipairs(variants) do
            local source = FS.PathJoin(directory, variant)
            if not _MoveFile(source, destination, found) then
                AL_print("Failed to rename file '"..source.."' to '"..destination.."'")
                return false
            end
            found = true
        end
    end
    return true
end

function AL_NormalizeGameFiles(options)

    AL_print("Merging/Renaming files...")
    if not _NormalizeFolders(options.destination, options.folderNames) or
        not _NormalizeFiles(options.destination, options.fileNames) then
        return false
    end

    AL_print("Done!")
    return true
end
