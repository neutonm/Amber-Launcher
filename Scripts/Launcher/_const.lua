
-- Figure out OS through lua
local BinaryFormat = package.cpath:match("%p[\\|/]?%p(%a+)")
if BinaryFormat == "dll" then
    function os.name()
        return "Windows"
    end
elseif BinaryFormat == "so" then
    function os.name()
        return "Linux"
    end
else
    function os.name()
        return "Unknown"
    end
end

-- enums
UIEVENT = {
    NULL                = 0,
    PRINT               = 1,
    MODAL_MESSAGE       = 2,
    MODAL_QUESTION      = 3,
    MODAL_GAMENOTFOUND  = 4,
    MODAL_TWEAKER       = 5,
    MODAL_LOCALISATION  = 6,
    MODAL_OPTIONS       = 7,
    MAX
}

UIWIDGET = {
    NULL                = 0,
    RADIO               = 1,
    POPUP               = 2,
    EDIT                = 3,
    LISTBOX             = 4,
    MAX
}

-- Variables
OS_NAME                 = OS_NAME or "Unknown"
GAME_FOLDER_NAME        = GAME_FOLDER_NAME or {
    "Might and Magic 7",
    "Might and Magic VII"
}
GAME_EXECUTABLE_FOLDERS = GAME_EXECUTABLE_FOLDERS or {}
GAME_BASE_PATHS         = GAME_BASE_PATHS or {}
GAME_EXECUTABLE_NAME    = GAME_EXECUTABLE_NAME or "mm7.exe"

-- OS Specific variables
OS_NAME = os.name()
if OS_NAME == "Windows" then 
    OS_FILE_SEPARATOR = '\\'
else
    OS_FILE_SEPARATOR = '/'
end
GAME_DESTINATION_FOLDER = GAME_DESTINATION_FOLDER or "."..OS_FILE_SEPARATOR.."DST"

-- Common system-specific folders for the game
if OS_NAME == "Windows" then
    GAME_BASE_PATHS = {
        os.getenv("PROGRAMFILES(X86)") .. '\\GOG Galaxy\\Games\\',
        os.getenv("PROGRAMFILES(X86)") .. '\\',
        os.getenv("PROGRAMFILES") .. '\\',
        "C:\\Program Files (x86)\\",
        "C:\\Program Files\\",
        os.getenv("SYSTEMDRIVE") .. "\\Games\\",
        os.getenv("SYSTEMDRIVE") .. "\\",
        "D:\\Program Files (x86)\\",
        "D:\\Program Files\\",
        "D:\\Games\\",
    }
elseif OS_NAME == "Linux" then
    local homePath  = os.getenv("HOME")
    GAME_BASE_PATHS = {
        homePath.."/Games/Windows/",
        homePath.."/.wine/drive_c/Program Files (x86)/GOG Galaxy/Games/",
        homePath.."/.wine/drive_c/Program Files (x86)/",
        homePath.."/.wine/drive_c/Program Files/",
        homePath.."/.wine/drive_c/Games/",
        homePath.."/Games/Heroic/",
        homePath.."/Games/",
    }
else
    OS_FILE_SEPARATOR = '/'
end

-- Common files
INI_PATH_MM7 = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR.."mm7.ini"
INI_PATH_MOD = GAME_DESTINATION_FOLDER..OS_FILE_SEPARATOR.."mod.ini"

-- Generate the combinations of base paths and game folder names
for _, basePath in ipairs(GAME_BASE_PATHS) do
    for _, folderName in ipairs(GAME_FOLDER_NAME) do
        local entry = basePath..folderName..OS_FILE_SEPARATOR..GAME_EXECUTABLE_NAME
        table.insert(GAME_EXECUTABLE_FOLDERS, entry)
    end
end

