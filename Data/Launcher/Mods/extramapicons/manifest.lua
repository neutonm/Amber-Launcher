return {
  id          = "extramapicons",
  games       = {"AMBER"},
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
    { 
      src       = "Assets/xmapicons.ICONS.hard.LOD", 
      dst       = "Data/Warrior/xmapicons.ICONS.hard.LOD", 
      autoclear = true 
    },
  },

  onApply = function(ctx)
  end,

  options = {
    {
      id       = "markers",
      title    = "Map icons:",
      type     = UIWIDGET.RADIO,
      optTitle = {"Shops", "Teachers", "Everything"},
      values   = {"shops", "teachers", "all"},
      default  = "all"
    }
  }
}
