
local ffi = require "ffi"
local C   = ffi.C

ffi.cdef[[
  
  void
  sen_textures_load(const char* filename,
                    const char* name,
                    const char* atlas_name,
                    size_t      atlas_size);
  
  void
  sen_textures_load_font(const char*      filename,
                         const float      font_size,
                         const char*      utf8_alphabet,
                         const char*      name,
                         const char*      atlas_name,
                         size_t           atlas_size);
  void
  sen_textures_reload();
  
  void
  sen_textures_collect(const char* atlas_name);
  
  void
  sen_shaders_load(const char* name,
                   const char* vs_name,
                   const char* fs_name);
  
  void
  sen_shaders_load_files(const char* name,
                         const char* vertex_shader_data,
                         const char* fragment_shader_data);
  
  void
  sen_shaders_reload();
  
  void
  sen_shaders_collect();
    
]]
 
local function ResManagerClosure(AtlasName, AtlasSize)
  -- private
  local m_atlas_name = AtlasName or "LuaAtlas"
  local m_atlas_size = AtlasSize or 512
  local m_search_path = {
    fonts   = "assets/fonts/",
    images  = "assets/images/",
    shaders = "assets/shaders/",
  }
  
  -- public
  local _ = {}
  
  function _.setAtlasName(AtlasName)
    m_atlas_name = AtlasName or "LuaAtlas"
  end
   
  function _.getAtlasName()
    return m_atlas_name
  end
   
  function _.setAtlasSize (AtlasSize)
    m_atlas_size = AtlasSize or 512
  end
  
  function _.getAtlasSize ()
    return m_atlas_size
  end
  
  function _.loadFont (name, file, size)
    C.sen_textures_load_font(m_search_path.fonts..file, size, nil, name, m_atlas_name, m_atlas_size)
  end
  
  function _.loadLabelShaders ()
    C.sen_shaders_load("label", "vs_pos4f_tex2f_col4f", "fs_text_tex2f_col4f")
  end

  function _.loadSpriteShaders ()
    C.sen_shaders_load("sprite", "vs_pos4f_tex2f_col4f", "fs_tex2f_col4f")
    C.sen_shaders_load("quad", "vs_pos4f_col4f", "fs_col4f")
  end

  function _.loadTexture (name, file)
    C.sen_textures_load(m_search_path.images..file, name, m_atlas_name, m_atlas_size)
  end

  function _.collect()
    C.sen_textures_collect(m_atlas_name);
  --  C.sen_shaders_collect();
  end
                      
  return _;
end

return ResManagerClosure 
