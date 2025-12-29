#pragma once
#include <glm/glm.hpp>
#include <cmath> // for floor()

class VMath {
public:
    // Standard Diablo tile size
    static constexpr float TILE_WIDTH = 64.0f;
    static constexpr float TILE_HEIGHT = 32.0f;

    // Convert Grid Coordinate (Logic) -> Screen Pixel (Visual)
    // Uses glm::ivec2 for integer grid coordinates
    static glm::vec2 tileToScreen(glm::ivec2 isoPos) {
        glm::vec2 screen;
        // The classic isometric formula
        // (x - y) moves us along the horizontal axis
        // (x + y) moves us along the vertical axis
        screen.x = (isoPos.x - isoPos.y) * (TILE_WIDTH * 0.5f);
        screen.y = (isoPos.x + isoPos.y) * (TILE_HEIGHT * 0.5f);
        return screen;
    }

    // Convert Screen Pixel (Visual) -> Grid Coordinate (Logic)
    // Uses glm::vec2 for floating point screen coordinates
    static glm::ivec2 screenToTile(glm::vec2 screenPos) {
        glm::ivec2 iso;
        
        // This is the algebraic inverse of the tileToScreen function
        float halfW = TILE_WIDTH * 0.5f;
        float halfH = TILE_HEIGHT * 0.5f;

        // Calculate rough floating point coordinates
        float isoX = (screenPos.x / halfW + screenPos.y / halfH) * 0.5f;
        float isoY = (screenPos.y / halfH - (screenPos.x / halfW)) * 0.5f;

        // Round down to get the exact tile index
        iso.x = static_cast<int>(floor(isoX));
        iso.y = static_cast<int>(floor(isoY));
        
        return iso;
    }
};