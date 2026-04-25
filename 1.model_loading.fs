#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 meshColor;   // Matches mesh.h
uniform bool hasTexture;  // Matches mesh.h

void main()
{    
    if(hasTexture) {
        FragColor = texture(texture_diffuse1, TexCoords);
    } else {
        FragColor = vec4(meshColor, 1.0);
    }
}