return {
    key             = "AMBER",
    name            = "amber",
    title           = "Amber Island",
    exeName         = "mm7.exe",
    iniName         = "mm7.ini",
    manifestName    = "amber_manifest.json",
    folderNames     = function()
        return {
            "Might and Magic 7",
            "Might and Magic VII"
        }
    end,
    basePaths       = function()
        if OS_NAME == "Windows" then
            return {
                os.getenv("PROGRAMFILES(X86)") .. '\\GOG Galaxy\\Games\\',
                os.getenv("PROGRAMFILES(X86)") .. '\\',
                os.getenv("PROGRAMFILES") .. '\\',
                "C:\\GOG Galaxy\\Games\\",
                "C:\\Program Files (x86)\\",
                "C:\\Program Files\\",
                "C:\\",
                os.getenv("SYSTEMDRIVE") .. "\\Games\\",
                os.getenv("SYSTEMDRIVE") .. "\\",
                "D:\\Program Files (x86)\\",
                "D:\\Program Files\\",
                "D:\\Games\\",
                "D:\\GOG Galaxy\\Games\\",
            }
        elseif OS_NAME == "Linux" then
            local homePath  = os.getenv("HOME")
            return {
                homePath.."/Games/Windows/",
                homePath.."/.wine/drive_c/Program Files (x86)/GOG Galaxy/Games/",
                homePath.."/.wine/drive_c/Program Files (x86)/",
                homePath.."/.wine/drive_c/Program Files/",
                homePath.."/.wine/drive_c/Games/",
                homePath.."/Games/Heroic/",
                homePath.."/Games/",
            }
        end

        return {}
    end,
    detectInstalled = function(root)
        if not FS.PathResolveCaseInsensitive(root, "mm7.exe") then
            return false
        end
        local manifest = FS.PathResolveCaseInsensitive(root, "Scripts/manifest.lua")
        if manifest then
            local ok, info = pcall(dofile, manifest)
            if ok and type(info) == "table" and info.name == "Amber Island" then
                return true
            end
        end
        -- Legacy Amber releases predate Scripts/manifest.lua.
        local legacy = FS.PathResolveCaseInsensitive(root, "Scripts/General/Mod.lua")
        local attr   = legacy and lfs.attributes(legacy)
        return attr and attr.size and attr.size > 4 * 1024 or false
    end
}
