
-- Figure out OS through lua
function os.name()
	return _OS_NAME
end

-- enums
UIEVENT = {
    NULL                = 0,  -- Empty Window
    DEBUG               = 1,  -- Debug console
    PRINT               = 2,  -- Print text into currernt text-view widget
    MAIN                = 3,  -- Revert back to main window (ASYNC)
    PLAY                = 4,  -- Launch game
    EXITAPP             = 5,  -- Exits launcher
    AUTOCONFIG          = 6,  -- Autoconfig wizard (ASYNC)
    MODAL_CLOSE         = 7,  -- Closes current modal window
    MODAL_MESSAGE       = 8,  -- Modal: Show Message
    MODAL_QUESTION      = 9,  -- Modal: Ask question
    MODAL_GAMENOTFOUND  = 10,  -- Modal: game-not-found error + specify path
    MODAL_TWEAKER       = 11, -- Modal: Wizard for game tweaks
    MODAL_LOCALISATION  = 12, -- Modal: Select language for game and mod
    MODAL_OPTIONS       = 13, -- Modal: All in one options tweaker
    MODAL_MODS          = 14, -- Modal: Mod Manager
    MODAL_TOOLS         = 15, -- Modal: External App/Command launcher
    MODAL_UPDATER       = 16, -- Modal: Auto Update
    MAX                 = 17
}

UIWIDGET = {
    NULL                = 0,
    RADIO               = 1,
    CHECKBOX            = 2,
    POPUP               = 3,
    EDIT                = 4,
    LISTBOX             = 5,
    MAX                 = 6
}

UISIDEBUTTON = {
    NULL                = 0,
    MAIN                = 1,
    SETTINGS            = 2,
    MODS                = 3,
    TOOLS               = 4,
    UPDATE              = 5,
    WEB_HOMEPAGE        = 6,
    WEB_DISCORD         = 7,
    MAX                 = 8
}
UISIDEBUTTON_NAMES = {}
for k, v in pairs(UISIDEBUTTON) do
    UISIDEBUTTON_NAMES[v] = k
end

GAME_NULL = {
    key             = "NULL",
    name            = "null",
    title           = "(null)",
    exeName         = "null.exe",
    iniName         = "null.ini",
    manifestName    = "null.json",
    folderNames     = function()
        return {
            "(null)",
        }
    end,
    basePaths       = function()
        return {
            "(null)",
        }
    end
}
GAMEID                  = GAMEID or {}
local idx               = #GAMEID + 1
GAMEID[idx]             = GAME_NULL
GAMEID[GAME_NULL.key]   = GAME_NULL
GAME_NULL.id            = idx
GAME_CURRENT            = GAME_NULL
GAME_DEFAULT_KEY        = "AMBER"

MODID = {
    NULL                = 0,
    AMBER               = 1,
    AMBERENROTH         = 2,
    AMBERMERGE          = 3,
    MM6                 = 4,
    MM7                 = 5,
    MM8                 = 6,
    OPENENROTH7         = 7,
    MMERGE              = 8,
    MAX                 = 9
}
MODID_NAMES = {
    [MODID.NULL]        = "null",
    [MODID.AMBER]       = "amber",
    [MODID.AMBERENROTH] = "amberenroth",
    [MODID.AMBERMERGE]  = "ambermerge",
    [MODID.MM6]         = "mm6",
    [MODID.MM7]         = "mm7",
    [MODID.MM8]         = "mm8",
    [MODID.OPENENROTH7] = "enroth7",
    [MODID.MMERGE]      = "mmerge",
}

MODID_TITLE = {
    [MODID.NULL]        = "(null)",
    [MODID.AMBER]       = "Amber Island",
    [MODID.AMBERENROTH] = "Amber Island (Open Enroth)",
    [MODID.AMBERMERGE]  = "Amber Island (Merge Mod)",
    [MODID.MM6]         = "Might & Magic 6",
    [MODID.MM7]         = "Might & Magic 7",
    [MODID.MM8]         = "Might & Magic 8",
    [MODID.OPENENROTH7] = "Might & Magic 7 (Open Enroth)",
    [MODID.MMERGE]      = "Merge Mod",
}

-- Variables
OS_NAME                 = OS_NAME or os.name()
GAME_FOLDER_NAME        = {}
GAME_EXECUTABLE_FOLDERS = {}
GAME_BASE_PATHS         = {}
GAME_EXECUTABLE_NAME    = GAME_NULL.exeName
GAME_MOD_ID             = MODID.NULL
GAME_MOD_ID_NAME        = GAME_NULL.name
GAME_MOD_ID_TITLE       = GAME_NULL.title
GAME_MOD_VERSION        = GAME_MOD_VERSION or 0
GAME_MOD_VERSION_STR    = GAME_MOD_VERSION_STR or "0.0.0"

LAUNCHER_PATH           = lfs.currentdir() or ""
LAUNCHER_NAME           = LAUNCHER_NAME or "AmberLauncher"
LAUNCHER_BUILD          = LAUNCHER_BUILD or _BUILD_NUMBER
LAUNCHER_VERSION        = LAUNCHER_VERSION or 100

URL_UPDATER_ROOT              = "https://mightandmagicmod.com/updater/"
URL_UPDATER_LAUNCHER_MANIFEST = URL_UPDATER_ROOT.."al_manifest.json"
URL_UPDATER_MOD_MANIFEST      = URL_UPDATER_ROOT..GAME_NULL.manifestName

-- OS Separator
if OS_NAME == "Windows" then 
    OS_FILE_SEPARATOR = '\\'
else
    OS_FILE_SEPARATOR = '/'
end

GAME_CURRENT_FOLDER_OS  = OS_NAME == "Windows" and "" or "."
GAME_DESTINATION_FOLDER = _DEBUG and "DST" or ""
GAME_DESTINATION_PATH   = GAME_DESTINATION_PATH or "."..OS_FILE_SEPARATOR..GAME_DESTINATION_FOLDER

-- Common system-specific folders for the game
if OS_NAME == "Windows" then
    GAME_BASE_PATHS = {
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
    GAME_BASE_PATHS = {
        homePath.."/Games/Windows/",
        homePath.."/.wine/drive_c/Program Files (x86)/GOG Galaxy/Games/",
        homePath.."/.wine/drive_c/Program Files (x86)/",
        homePath.."/.wine/drive_c/Program Files/",
        homePath.."/.wine/drive_c/Games/",
        homePath.."/Games/Heroic/",
        homePath.."/Games/",
    }
end

-- Mods
MOD_ROOT_FOLDER = (GAME_DESTINATION_PATH .. OS_FILE_SEPARATOR ..
                table.concat({ "Data", "Launcher", "Mods" }, OS_FILE_SEPARATOR))

-- Common files
INI_PATH_MM7            = GAME_DESTINATION_PATH..OS_FILE_SEPARATOR.."mm7.ini"
INI_PATH_MOD            = GAME_DESTINATION_PATH..OS_FILE_SEPARATOR.."mod.ini"

-- Generate the combinations of base paths and game folder names
for _, basePath in ipairs(GAME_BASE_PATHS) do
    for _, folderName in ipairs(GAME_FOLDER_NAME) do
        local entry = basePath..folderName..OS_FILE_SEPARATOR..GAME_EXECUTABLE_NAME
        table.insert(GAME_EXECUTABLE_FOLDERS, entry)
    end
end

