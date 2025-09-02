return {
  id          = "davidsprites",
  game        = MODID_NAMES[MODID.AMBER],
  name        = "DaveHer's Foliage Overhaul",
  version     = "1.0.0",
  website     = "https://www.celestialheavens.com/forum/10",
  author      = "DaveHer",
  description = [[
Complete overhaul of foliage sprites: trees, flowers, plant pots, etc.
  ]],
  install = {
    { 
      src       = "Assets/dav.SPRITES.LOD", 
      dst       = "Data/dav.SPRITES.LOD", 
      autoclear = true 
    },
    { 
      src       = "Assets/SFT.txt",
      dst       = "Data/Tables/SFT_david.txt",
      autoclear = true
    },
    { 
      src       = "Assets/david.lua",
      dst       = "Scripts/General/david.lua",
      autoclear = true
    },
  },

  onInstall = function(ctx)
  end,

  onUninstall = function(ctx)
    --ctx.log("Removing Dummy.lua")
    --FS.PathDelete(FS.PathJoin(ctx.gameRoot, "Scripts", "General", "Dummy.lua"))
  end,

  onOption = function(ctx)
  end,

  options = {
    dummy = {
      type    = "enum",
      values  = {1, 2, 3},
      default = 1
    }
  }
}
