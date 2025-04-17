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

> Current: n/a

## Main
*General points towards development*

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
- [ ] SLuaState_PushGlobalVariable should utilize SVar
- [ ] add small variable reflection (from c) so we can auto-generate lua tables for enums, etc.

## GUI Library 
*NAPPGUI user interface library that wraps Core Library calls.* 

- [x] Autoconfigure should handle failed state
- [x] Figure out how to show file browser (required for non-found game)
- [ ] Re-design Modal: GameNotFound

## Lua Scripts
*Provide alternative and/or extended functionality for the launcher.*

- [ ] ConfigTweak.lua
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