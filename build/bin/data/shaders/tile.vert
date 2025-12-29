#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec2 uResolution; // Screen dimensions
uniform vec2 uOffset;     // Where to draw the tile (in pixels)
uniform vec2 uSize;       // Size of the tile (64x32)

void main() {
    // 1. Scale the normalized vertex by the tile size
    vec2 scaledPos = aPos * vec2(uSize.x * 0.5, uSize.y); 

    // 2. Move to the correct position on screen
    vec2 finalPos = scaledPos + uOffset;

    // 3. Convert Screen Pixels (0..800) to OpenGL Clip Space (-1..1)
    float ndcX = (finalPos.x / uResolution.x) * 2.0 - 1.0;
    float ndcY = 1.0 - (finalPos.y / uResolution.y) * 2.0; // Flip Y for Top-Left origin

    gl_Position = vec4(ndcX, ndcY, 0.0, 1.0);
}