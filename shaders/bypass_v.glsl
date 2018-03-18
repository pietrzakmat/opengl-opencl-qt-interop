#version 130

in vec2 v_vertex;
in vec2 v_texCoord;
out vec2 f_texCoord;

void main(void)
{
    f_texCoord = v_texCoord;
    gl_Position = vec4(v_vertex, 0.0, 1.0);
}
