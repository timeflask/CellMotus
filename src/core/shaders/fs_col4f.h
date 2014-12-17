const GLchar SEN_fs_col4f[] = TO_STRING(

varying vec4 v_frag_color;

void main()
{
    gl_FragColor = v_frag_color;
}

);
