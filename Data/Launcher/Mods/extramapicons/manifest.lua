return {
  id          = "extramapicons",
  game        = MODID_NAMES[MODID.AMBER],
  name        = "Extra Map Icons",
  version     = "1.0.0",
  website     = "https://mightandmagicmod.com",
  author      = "Henrik Chukhran",
  description = [[
This mod refreshes world map by replacing the base texture with a icon-rich version: shops, teachers, and dungeons are marked, making navigation faster and easier without changing core gameplay.
  ]],
  install = {
    { 
      src       = "Assets/xmapicons.ICONS.LOD", 
      dst       = "Data/xmapicons.ICONS.LOD", 
      autoclear = true 
    },
  },

  onInstall = function(ctx)
  end,

  onUninstall = function(ctx)
  end,

  onOption = function(ctx)
  end,

  options = {
    extramapicons = {
      type    = "enum",
      values  = {1},
      default = 1
    }
  }
}
