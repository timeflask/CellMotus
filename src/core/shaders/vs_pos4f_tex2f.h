static const GLchar SEN_vs_pos4f_tex2f[] = TO_STRING (

attribute vec4 a_pos;
attribute vec2 a_tex_coords;
varying vec2 v_tex_coords;

void main()
{
    gl_Position = u_mvp * a_pos;
    v_tex_coords = a_tex_coords;
}
);
