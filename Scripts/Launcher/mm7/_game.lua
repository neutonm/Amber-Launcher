return {
    key             = "MM7",
    name            = "mm7",
    title           = "Might and Magic VII",
    exeName         = "mm7.exe",
    iniName         = "mm7.ini",
    manifestName    = "mm7_manifest.json",
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
        return not not FS.PathResolveCaseInsensitive(root, "mm7.exe")
    end,
    isConfigured    = function()
        return not not FS.PathResolveCaseInsensitive(GAME_DESTINATION_PATH, "MM7patch.dll") and
            FS.IsFilePresent(INI_PATH_GAME) and FS.IsFilePresent(INI_PATH_MOD)
    end
}
