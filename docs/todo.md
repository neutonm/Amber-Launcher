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
 
> **CURRENT:** Figure how to show file browser via lua and connect UI and logic somehow with 
Core and GUI libraries

## Main
*General points  towards development*

- [ ] Write code style document
- [ ] Clang format
- [ ] Separate GUI  (cmake)
- [ ] Separate Scripting language (cmake)
- [x] tools folder for auxilary bash/python scripts
- [x] video conversion script (for tweak previews)
- [ ] add cpack (cmake): pack release into "distr" folder

## Core Library
*Responsible for providing main functionality for the launcher app. Must be independent module.* 

- [ ] Code Review for header/sources
- [ ] add theoraplay library
 
#### Lua Module:
- [ ] Return status code from AL.CommandCall

## GUI Library 
*NAPPGUI user interface library that wraps Core Library calls.* 

- [ ] Autoconfigure should handle failed state
- [ ] Figure out how to show file browser (required for non-found game)
- [ ] test video playback via theoraplay (required for illustrating option tweaking)

## Lua Scripts
*Provide alternative and/or extended functionality for the launcher.*

- [ ] ConfigTweak.lua
- [ ] ConvertMusic.lua
- [ ] DetectAndCopyGame.lua
- [ ] DetectAndInstallMod.lua
- [ ] MergeAndRename.lua
- [ ] RegistryTweaks