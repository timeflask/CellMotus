const GLchar  SEN_fs_tex2f_col4f[] = TO_STRING(

varying vec2 v_tex_coords;
varying vec4 v_frag_color;


void main()
{
  gl_FragColor =  v_frag_color*texture2D(u_tex0, v_tex_coords);
}
);
