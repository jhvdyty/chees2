#version 330 core
in vec3 ourColor;
//in vec3 ourPosition;
out vec4 color;

in vec2 TexCoord;

uniform float mixValue;

uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;



void main() {
    vec4 texColor = texture(ourTexture1, TexCoord);
    
    if(texColor.a < 0.1)
        discard;

    color = texColor;
}