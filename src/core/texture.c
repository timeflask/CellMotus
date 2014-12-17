#include "texture.h"
#include "texture-manager.h"
#include "opengl.h"
#include "logger.h"

unsigned int g_current_tex = 0;

void
sen_texture_bind(const texture_t* tex)
{
  if (tex) {
    if (!tex->id) {
      _logfi("Need to upload texture [%s]", tex->name);
      sen_texture_manager_upload((void*)tex->_node);
    }
    if (g_current_tex != tex->id) {

      glBindTexture( GL_TEXTURE_2D, tex->id );
      g_current_tex = tex->id;

    }
  }
  else {
    glBindTexture( GL_TEXTURE_2D, 0 );
    g_current_tex = 0;
  }

}

