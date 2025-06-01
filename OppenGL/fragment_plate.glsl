// fragment_plate.glsl
#version 330 core
uniform sampler2D ourTexture1;

in vec2 TexCoords;
out vec4 FragColor;

void main()
{
    vec4 tex1 = texture(ourTexture1, TexCoords);
    FragColor = tex1;
}