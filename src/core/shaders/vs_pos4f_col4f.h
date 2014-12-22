const GLchar SEN_vs_pos4f_col4f[] = TO_STRING (

attribute vec4 a_pos;
attribute vec4 a_color;

varying vec4 v_frag_color;

void main()
{
    gl_Position = u_mvp*a_pos;
    v_frag_color = a_color;
}
);
