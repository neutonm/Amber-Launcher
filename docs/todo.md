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

## Main
*General points towards development*

- [X] Fix memory leaks
- [ ] Uninstall Game/Mod feature (should totally revert everything + backup saves)
- [ ] App and scripts must be based on specified game (Amber Island by default)
- - It's step into the future, providing app usage for multiple mods
- [ ] Update LICENSE file
- [ ] Write code style document
- [ ] Clang format
- [ ] Separate GUI  (cmake)
- [ ] Separate Scripting language (cmake)
- [x] tools folder for auxilary bash/python scripts
- [x] video conversion script (for tweak previews)
- [x] add cpack (cmake): pack release into "distr" folder
- [ ] SVar: should rather utilize #define _SVAR_GET_WHATEVER(v) and then #define SVAR_GET_WHATEVER(v) _SVAR_GET_WHATEVER(v) rather than wrap everything in do-while loop

## Core Library
*Responsible for providing main functionality for the launcher app. Must be independent module.* 

- [ ] Code Review for header/sources
- [ ] save argc/argv into AppCore struct
- [ ] rename header guards: conform to one convention
- [ ] extern_c for every header!
- [ ] lua error report
- [ ] SCommand - change SCommandArg to SVar
 
#### Lua Module:
- [x] Return status code from AL.CommandCall
- [x] SLuaState_PushGlobalVariable should utilize SVar (edit: removed it, no use)
- [ ] add small variable reflection (from c) so we can auto-generate lua tables for enums, etc.

## GUI Library 
*NAPPGUI user interface library that wraps Core Library calls.* 

- [ ] Mod Manager: options for mods are implemented, but there's no GUI widgets to tweak it
- [ ] String operations that re-create themselves should rely on "str_upd()"
- [x] Autoconfigure should handle failed state
- [x] Figure out how to show file browser (required for non-found game)
- [ ] Re-design Modal: GameNotFound
- [ ] Improve Tweaker GUI
- - window size must be fixed

## Lua Scripts
*Provide alternative and/or extended functionality for the launcher.*

- [ ] INI:
- - INI keys should be predefined in _const.lua as they repeat themselves at couple of places
- - Checks must be made for INILoad, recreate default mod.ini and mm7.ini if file doesn't exist (extract from archive) 
- [ ] ModalOptions.lua
- - UI_WIDGET_LISTBOX binary "choice" representation of return value for ui call result is a bit overkill
- - "Default" button
- - - "default" value of global table should be responsible for reset value.
- - - "value" should be responsible for current value, remove opt.value from choice processing part
- - INI file gets opened too many times, very stupid move.
- [ ] DetectAndCopyGame.lua
- - check and warn if user suddenly points to mm6 or mm8 (optional)
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
- [X] DetectAndInstallMod.lua
- [X] MergeAndRename.lua
- [X] ConvertMusic.lua
- [X] RegistryTweaks.lua
- [X] ConfigTweak.lua
- [X] Localization.lua