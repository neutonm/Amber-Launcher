```
  __________  ____  ____     __    _______________
 /_  __/ __ \/ __ \/ __ \   / /   /  _/ ___/_  __/
  / / / / / / / / / / / /  / /    / / \__ \ / /   
 / / / /_/ / /_/ / /_/ /  / /____/ / ___/ // /    
/_/  \____/_____/\____/  /_____/___//____//_/     
                                                                                    
```
Table Of Contents:
- [Main](#main)
- [Core Library](#core-library)
    - [Lua Module:](#lua-module)
- [GUI Library](#gui-library)
- [Lua Scripts](#lua-scripts)

> CURRENT:
> - code review & refactor this commit

> ARCHIVED:
> UPD 30 may: shit left to do:
> - process selected options
> - do something about them (in lua)
> - - let user define the name of the option (in lua) so he can check/process later (uiResponse)
> - previous button: should remember last choice
> - code review/refactor
> UPD 21 may: implement grouped _TWEAKER processing (e.g: local t = { {info = 'derp'},{info = 'hoho'} })
> UPD 7 may: i need:
> - The major goal is to convert lua to SVAR from UI Module
> - SVAR_ARRAY of sorts
> - UICall converts lua variadic argument (which is table) which is decomposed inside UI module into smaller SVARs. 
> - So, at the end:
> - - LUA: ideally we pass lua tables with whatever there is (e.g: table of tables)
> - - UI Module: converts lua tables into SVAR and pass it to MODAL window
> - - *Modal Tweaker turns into wizard*

## Main
*General points towards development*

- [ ] Fix memory leaks
- [ ] Revert/Enable/Disable features for every command
- [ ] Uninstall Mod feature (should totally revert everything + backup saves)
- [ ] App and scripts must be based on specified game (Amber Island by default)
- - It's step into the future, providing app usage for multiple mods
- [ ] Refactor whole number variables - change d to n
- [ ] Update LICENSE file
- [ ] Write code style document
- [ ] Clang format
- [ ] Separate GUI  (cmake)
- [ ] Separate Scripting language (cmake)
- [x] tools folder for auxilary bash/python scripts
- [x] video conversion script (for tweak previews)
- [ ] add cpack (cmake): pack release into "distr" folder
- [ ] SVar: should rather utilize #define _SVAR_GET_WHATEVER(v) and then #define SVAR_GET_WHATEVER(v) _SVAR_GET_WHATEVER(v) rather than wrap everything in do-while loop

## Core Library
*Responsible for providing main functionality for the launcher app. Must be independent module.* 

- [ ] Code Review for header/sources
- [ ] add theoraplay library
- - [ ] test video playback via theoraplay (required for illustrating option tweaking)
- - Maybe ditch it and just stick to GIF? Nappgui already provides support for gif playback.
- [ ] save argc/argv into AppCore struct
- [ ] rename header guards: conform to one convention
- [ ] extern_c for every header!
- [ ] lua error report
- [ ] SCOmmand - change SCommandArg to SVar
 
#### Lua Module:
- [x] Return status code from AL.CommandCall
- [x] SLuaState_PushGlobalVariable should utilize SVar (edit: removed it, no use)
- [ ] add small variable reflection (from c) so we can auto-generate lua tables for enums, etc.

## GUI Library 
*NAPPGUI user interface library that wraps Core Library calls.* 

- [x] Autoconfigure should handle failed state
- [x] Figure out how to show file browser (required for non-found game)
- [ ] Re-design Modal: GameNotFound
- [ ] Improve Tweaker GUI
- - window size must be fixed

## Lua Scripts
*Provide alternative and/or extended functionality for the launcher.*

- [ ] ConfigTweak.lua
- - import precached default mm7.ini
> Tweaker: 
> - language select (audio, game, mod)
> - mouse look start
> - old/grayface gui
> - W and D strafe or turn
> - David's HD sprite pack
> - 
- [ ] ConvertMusic.lua
- [ ] DetectAndCopyGame.lua
- - check and warn if user suddenly points to mm6 or mm8 (optional)
- - Browse button: if file browser window is canceled without specifiying, you get appcrash (nappgui error)
- - if during game detection issues occur - decide whether to continue searching game or not
- - UI:
- - - extra: put a header picture
- - - Add event for "Edit" as well, user might manually type game path (currently, pApp->pString won't update if done manuallly)
- - - Error modal:  should show current game path (for local issues)
- - - Error modal:  continue and abort button
- - Set file browser to start with working directory
- - if there's no mm7.exe then don't report missing files, instead report wrong file or game
- - File browser should visually clear itself after confirming game path (rendering issue - it does't look great)
- - Errors should show modal window with error message and allow to continue or abort operation.
- - - UserEvent call can be utilized here (or separate internal errorReport event)
- - printfs should really print to pApp->pTextView
- [ ] DetectAndInstallMod.lua
- [ ] MergeAndRename.lua
- [ ] RegistryTweaks