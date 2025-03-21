# ToDo List
 
## Main
*General points  towards development*

- [ ] Write code style document
- [ ] Clang format
- [ ] Separate GUI  (cmake)
- [ ] Separate Scripting language (cmake)
- [ ] tools folder for auxilary bash/python scripts
- - [X] video conversion script (for tweak previews)

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