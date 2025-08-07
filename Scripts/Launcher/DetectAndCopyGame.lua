-- FILE:        DetectAndCopyGame.lua
-- DESCRIPTION: Detect and Copy Game contents into launcher folder

-- List of files to check and copy
local GRAYFACE_FILES = {
    "Data/00 add.icons.lod",
    "Data/00 patch.bitmaps.lod",
    "Data/00 patch.events.lod",
    "Data/00 patch.games.lod",
    "Data/00 patch.icons.lod",
    "Data/01 dragon.games.lod",
    "Data/01 mon pal.bitmaps.lod",
    "Data/01 tunnels.events.lod",
    "Data/01 water.bitmaps.lwd",
    "Data/icons.UI.lod",
    "Data/MouseCursorShooter.bmp",
    "Data/MouseCursorShooterHD.bmp",
    "Data/MouseLookCursor.bmp",
    "Data/MouseLookCursorHD.bmp",
    "MM7Patch ReadMe.TXT",
    "MM7patch.dll",
    "MM7-Rel.exe",
}

local MM7_COPY_FILES = {
    "Anims/Magic7.vid",
    "Anims/Might7.vid",
    "Data/BITMAPS.LOD",
    "Data/d3dbitmap.hwl",
    "Data/d3dsprite.hwl",
    "Data/Events.lod",
    "Data/GAMES.LOD",
    "Data/ICONS.LOD",
    "Data/SPRITES.LOD",
    "Music/2.mp3",
    "Music/3.mp3",
    "Music/4.mp3",
    "Music/5.mp3",
    "Music/6.mp3",
    "Music/7.mp3",
    "Music/8.mp3",
    "Music/9.mp3",
    "Music/10.mp3",
    "Music/11.mp3",
    "Music/12.mp3",
    "Music/13.mp3",
    "Music/14.mp3",
    "Music/15.mp3",
    "Music/16.mp3",
    "Music/17.mp3",
    "Music/18.mp3",
    "Music/19.mp3",
    "Music/20.mp3",
    "SOUNDS/Audio.snd",
    "audiere.dll",
    "audio.dll",
    "BINKW32.DLL",
    "MANUAL.PDF",
    "MAP.PDF",
    "mm7.exe",
    "MM7Setup.exe",
    "MSS32.DLL",
    "MSSA3D.M3D",
    "MSSDS3DH.M3D",
    "Mssds3ds.m3d",
    "MSSEAX.M3D",
    "Mssfast.m3d",
    "msvcp90.dll",
    "msvcr90.dll",
    "Readme.txt",
    "Reference_card.PDF",
    "SMACKW32.DLL",
    "VIC32.DLL",
}

-- We need to check for wav files during detection, not mp3
local function _BuildDetectList(copyList)
    local detectList = {}
    for _, path in ipairs(copyList) do
        if path:match("^Music/.*%.mp3$") then
            table.insert(detectList, path:gsub("%.mp3$", ".wav"))
        else
            table.insert(detectList, path)
        end
    end
    return detectList
end

local MM7_DETECT_FILES = _BuildDetectList(MM7_COPY_FILES)

-- Public functions:
function AL_DetectGame(searchFolder)

    print("Detecting game…")
    local searchFolders = searchFolder and {searchFolder} or GAME_EXECUTABLE_FOLDERS

    -- 1) Already in launcher folder?
    local localExePath  = FS.join_paths(GAME_DESTINATION_FOLDER, GAME_EXECUTABLE_NAME)
    local localExe      = FS.IsFilePresent(localExePath)
    if localExe then
        print("Game executable present in destination folder.")
        if FS.CheckFiles(GAME_DESTINATION_FOLDER, MM7_DETECT_FILES) then
            return GAME_DESTINATION_FOLDER, "local"
        else
            print("Game in destination is incomplete -> will continue searching…")
        end
    end

    -- 2) Search predefined folders
    for _, candidate in ipairs(searchFolders) do

        local exe = FS.IsFilePresent(candidate)
        if exe and FS.IsFileExecutable(exe) then
            local root = FS.GetDirectory(exe)
            if FS.CheckFiles(root, MM7_DETECT_FILES) then
                print("Found valid installation at "..root)
                return root, "external"
            else
                print("Found executable at "..root.." but data files are incomplete.")
            end
        end
    end

    return nil, "notfound"
end

-- Static functions
local function _CopyGameFiles(src, dst)

    print("Copying game from '"..src.."' -> '"..dst.."' …")

    FS.ensure_directory(dst)
    FS.CopyFiles(src, dst, MM7_COPY_FILES)

    if not FS.CheckFiles(dst, MM7_COPY_FILES) then
        print("Verification failed after copy.")
        return false
    end

    print("Copy succeeded.")
    return true
end

local function _DetectAndCopyGame(searchFolder)

    local src, how = AL_DetectGame(searchFolder)
    if not src then

        print("Failed to find the game!")
        local uiRes = AL.UICall(UIEVENT.MODAL_GAMENOTFOUND)

        if uiRes and uiRes.status == false then
            return false
        end

        if uiRes and uiRes.path and uiRes.path ~= "" then
            return _DetectAndCopyGame(uiRes.path) -- recursive
        end

        return false
    end

    if how == "local" then
        print("No copy needed - game verified in destination folder.")
        return true
    end

    return _CopyGameFiles(src, GAME_DESTINATION_FOLDER)
end

-- Global callbacks
function events.InitLauncher()

    print("Initialize DetectAndCopyGame.lua")
    AL.CommandAdd("DetectAndCopyGame", _DetectAndCopyGame, -100) 

end
