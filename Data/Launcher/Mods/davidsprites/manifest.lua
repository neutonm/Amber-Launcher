return {
  id          = "davidsprites",
  games       = {"AMBER", "MM7"},
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
      src       = "Assets/dsft.bin", 
      dst       = "DataFiles/dsft.bin", 
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

  onApply = function(ctx)
  end,

  options = {
    { id = "trees",   title = "New trees",    type = UIWIDGET.CHECKBOX, default = true },
    { id = "foliage", title = "New foliage",  type = UIWIDGET.CHECKBOX, default = true },
    { id = "water",   title = "New water",    type = UIWIDGET.CHECKBOX, default = true }
  }
}
