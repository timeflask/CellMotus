const GLchar  SEN_fs_text_tex2f_col4f[] = TO_STRING(

varying vec2 v_tex_coords;
varying vec4 v_frag_color;

void main()
{
  //discard;
     float a = texture2D(u_tex0, v_tex_coords).a;
   //if (a>0.01)
      gl_FragColor = vec4(v_frag_color.rgb,a *v_frag_color.a  );
   //else
     // discard;
    //gl_FragColor =  v_frag_color*texture2D(u_tex0, v_tex_coords).a;


 }
);
