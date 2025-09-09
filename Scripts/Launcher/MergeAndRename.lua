-- List of proper folder names
local folderNames = {
    "Anims",
    "Data",
    "SOUNDS",
    "Saves",
}

-- List of proper file names
local fileNames = {
    GAME_EXECUTABLE_NAME,
    "MM7Setup.exe",
}

-------------------------------------------------------------------------------

local function ProcessFolders(base_dir, wanted_names)

    local entries = FS.DirectoryList(base_dir)

    -- Map lowercased name -> {actual1, actual2, ...}
    local buckets = {}
    for _, entry in ipairs(entries) do
        local k = entry:lower()
        if not buckets[k] then buckets[k] = {} end
        buckets[k][#buckets[k]+1] = entry
    end

    for _, correct in ipairs(wanted_names) do
        local correct_lower = correct:lower()
        local correct_path  = FS.PathJoin(base_dir, correct)

        local matches   = buckets[correct_lower] or {}
        local has_exact = false
        local improper  = {}

        for _, actual in ipairs(matches) do
            if actual == correct then has_exact = true else improper[#improper+1] = actual end
        end

        if not has_exact then
            if #improper > 0 then
                local from = FS.PathJoin(base_dir, improper[1])
                local ok, err = os.rename(from, correct_path)
                if not ok then
                    AL_print("Error renaming folder '"..improper[1].."' -> '"..correct.."': "..tostring(err))
                else
                    print("Renamed folder '"..improper[1].."' to '"..correct.."'")
                end
                table.remove(improper, 1)
            else
                FS.DirectoryEnsure(correct_path)
                print("Created missing folder '"..correct.."'")
            end
        end

        -- Merge remaining variants into the canonical folder
        for _, name in ipairs(improper) do
            local src = FS.PathJoin(base_dir, name)
            AL_print("Merging contents from '"..name.."' -> '"..correct.."'")
            FS.FilesMove(src, correct_path)
            FS.PathDelete(src)
            print("Removed folder '"..name.."'")
        end
    end
end

local function ProcessFiles(base_dir, wanted_names)

    local entries = FS.FilesList(base_dir)

    local buckets = {}
    for _, entry in ipairs(entries) do
        local k = entry:lower()
        if not buckets[k] then buckets[k] = {} end
        buckets[k][#buckets[k]+1] = entry
    end

    for _, correct in ipairs(wanted_names) do
        local correct_lower = correct:lower()
        local correct_path  = FS.PathJoin(base_dir, correct)

        local matches   = buckets[correct_lower] or {}
        local has_exact = false
        local improper  = {}

        for _, actual in ipairs(matches) do
            if actual == correct then has_exact = true else improper[#improper+1] = actual end
        end

        if not has_exact then
            if #improper > 0 then
                local from = FS.PathJoin(base_dir, improper[1])
                local ok, err = os.rename(from, correct_path)
                if not ok then
                    AL_print("Error renaming file '"..improper[1].."' -> '"..correct.."': "..tostring(err))
                else
                    print("Renamed file '"..improper[1].."' to '"..correct.."'")
                end
                table.remove(improper, 1)
            else
                AL_print("File '"..correct.."' is missing.")
            end
        end

        for _, name in ipairs(improper) do
            local src = FS.PathJoin(base_dir, name)
            AL_print(("Overwriting '%s' with '%s' and removing the duplicate"):format(correct, name))

            -- gpt5 moment (new model sucks), too lazy to review tho...
            --- @todo review and think again
            -- Try a fast rename first (POSIX can overwrite; Windows won't if dest exists)
            local renamed = os.rename(src, correct_path)
            if not renamed then
                -- Fallback: copy bytes over the canonical file, then delete the duplicate
                local r = io.open(src, "rb")
                local w = r and io.open(correct_path, "wb")  -- "wb" overwrites/truncates
                if r and w then
                    w:write(r:read("*all"))
                    r:close(); w:close()
                    FS.PathDelete(src)
                    print("Removed file '"..name.."'")
                else
                    if r then r:close() end
                    AL_print("Error: couldn't overwrite '"..correct.."' with '"..name.."'")
                end
            else
                -- Rename succeeded; the duplicate no longer exists in its old place
                print("Removed file '"..name.."'")
            end
            -- gpt5 end
        end
    end
end

-------------------------------------------------------------------------------
local function _MergeAndRename()

    AL_print("Merging/Renaming files...")

    -- Run the processing function
    ProcessFolders(GAME_DESTINATION_PATH, folderNames)
    ProcessFiles(GAME_DESTINATION_PATH, fileNames)

    AL_print("Done!")

    return true
end

function events.InitLauncher()

    print("Initialize MergeAndRename.lua")
    AL.CommandAdd("MergeAndRename", _MergeAndRename, -80) 

end
