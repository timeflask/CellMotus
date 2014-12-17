const GLchar SEN_fs_tex2f[] = TO_STRING(

varying vec2 v_tex_coords;

void main()
{
    gl_FragColor =  texture2D(u_tex0, v_tex_coords);
}
);
