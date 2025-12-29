#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse; // The texture unit
uniform vec3 uColor;               // Fallback color if no texture

void main() {
    // Sample the texture at the given UV coordinate
    vec4 texColor = texture(texture_diffuse, TexCoords);
    
    // Simple Alpha Testing: Discard transparent pixels (useful for foliage/hair)
    if(texColor.a < 0.1)
        discard;

    FragColor = texColor;
}