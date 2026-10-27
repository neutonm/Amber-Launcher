-- FILE:        DetectAndCopyGame.lua
-- DESCRIPTION: Detect and Copy Game contents into launcher folder

-- List of files to check and copy
GRAYFACE_FILES = {
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

MM7_COPY_FILES = {
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

-- Build detection list that expects WAV instead of MP3
local function _BuildDetectList(copyList)
    local detectList = {}
    for _, path in ipairs(copyList) do
        if path:match("^Music/.*%.mp3$") then
            detectList[#detectList+1] = path:gsub("%.mp3$", ".wav")
        else
            detectList[#detectList+1] = path
        end
    end
    return detectList
end

local MM7_DETECT_FILES = _BuildDetectList(MM7_COPY_FILES)

-- Public functions:
function AL_DetectGame(searchFolder, bIgnoreWav)

    AL_print("Detecting game...")

    bIgnoreWav               = bIgnoreWav   and bIgnoreWav or false
    local searchFolders      = searchFolder and {searchFolder} or GAME_EXECUTABLE_FOLDERS
    local listOfFilesToCheck = bIgnoreWav   and MM7_COPY_FILES or MM7_DETECT_FILES
    print("Search path: "..dump(searchFolders))

	-- Normalize path (os separator)
	for i, path in ipairs(listOfFilesToCheck) do
		listOfFilesToCheck[i] = FS.PathNormalize(path)
	end

    -- 1) Already in launcher folder?
    local localExe = FS.PathResolveCaseInsensitive(GAME_DESTINATION_PATH, GAME_EXECUTABLE_NAME)
    if localExe then

        print("Game executable present in destination folder.")
        if FS.FilesCheck(GAME_DESTINATION_PATH, listOfFilesToCheck) then
            return GAME_DESTINATION_PATH, "local"
        else
            print("Game in destination is incomplete -> will continue searching…")
        end
    end

    -- 2) Search predefined folders (these are directories)
    for _, candidate in ipairs(searchFolders) do

        local exe = FS.IsFilePresent(candidate)
        if exe and FS.IsFileExecutable(candidate) then
            local root = FS.PathGetDirectory(candidate)
            if FS.FilesCheck(root, listOfFilesToCheck) then
                print("Found valid installation at "..root)
                return root, "external"
            else
                print("Found executable at \""..root.."\" but data files are incomplete.")
            end
        end
    end

    return nil, "notfound"
end

-- Static functions
local function _CopyGameFiles(src, dst)
    AL_print("Copying game from '"..src.."…")
    FS.DirectoryEnsure(dst)
    FS.FilesCopy(src, dst, MM7_COPY_FILES)

    AL_print("Copy succeeded.")
    return true
end

local function _DetectAndCopyGame(searchFolder)
    local src, how = AL_DetectGame(searchFolder, true)
    if not src then
        AL_print("Failed to find the game!")

        local curDir    = FS.CurrentDir()
        local uiRes     = AL.UICall(UIEVENT.MODAL_GAMENOTFOUND)
        print(dump(uiRes))
        if uiRes then

            if uiRes.status == false then
                return false
            end

            lfs.chdir(curDir)
            if uiRes.path and uiRes.path ~= "" then
                return _DetectAndCopyGame(uiRes.path) -- recursive
            end
        end
        
        return false
    end

    if how == "local" then
        AL_print("No copy needed - game verified in destination folder.")
        return true
    end

    AL_print("Fetching game from: "..src)
    sleep(1) -- give that ui time to close

    return _CopyGameFiles(src, GAME_DESTINATION_PATH)
end

-- Global callbacks
function events.InitLauncher()
    print("Initialize DetectAndCopyGame.lua")
    AL.CommandAdd("DetectAndCopyGame", _DetectAndCopyGame, -100)
end
