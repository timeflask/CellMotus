const GLchar SEN_vs_pos4f_tex2f_col4f[] = TO_STRING (

attribute vec4 a_pos;
attribute vec2 a_tex_coords;
attribute vec4 a_color;

varying vec4 v_frag_color;
varying vec2 v_tex_coords;


void main()
{
    gl_Position =  u_mvp*a_pos;
    v_frag_color = a_color;
    v_tex_coords = a_tex_coords;
}
);
