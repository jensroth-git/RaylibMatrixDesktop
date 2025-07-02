#pragma once
#include "raylib.h"
#include "hsl_rgb.h"
#include <vector>
#include <random>

class MatrixGlyph {
public:
    static Texture glyphTexture;
    static constexpr int xGlyphs = 8;
    static constexpr int yGlyphs = 8;
    static constexpr int glyphLength = xGlyphs * yGlyphs - 7;

    static void Init();
    static void Cleanup();
    static Vector2 GetGlyphSize();
    static void DrawGlyph(int x, int y, float scale, int glyph, float opacity);
};

struct GlyphCell {
    int currentGlyph = 0;
    float timeTillGlyphChange = 0.0f;
    float opacity = 0.0f;
    bool spawnCell = false;
};

class MatrixRain {
public:
    MatrixRain(int screenWidth, int screenHeight, float glyphScale);
    ~MatrixRain() = default;

    void Update(float delta);
    void Draw() const;
    void SetSpawnCell(Vector2 cellPos);
    
    Vector2 GetCellPositionFromPoint(Vector2 point) const;
    
    // Public mouse position tracking (matching previous implementation)
    Vector2 lastMouseCellPos = Vector2();

private:
    // Settings
    static constexpr float glyphChangeTime = 0.4f;
    static constexpr float glyphMoveTickTime = 0.037f;
    static constexpr float glyphFadeTime = 1.5f;
    static constexpr float spawnTime = 5.0f;

    int columns, rows;
    int columnWidth, rowHeight;
    float glyphScale;
    float tickTime = 0.0f;
    
    std::vector<std::vector<GlyphCell>> glyphs;
    std::vector<float> spawnTimes;
    
    // Random number generation
    std::default_random_engine generator;
    std::uniform_int_distribution<int> glyphDist;
    std::uniform_real_distribution<float> glyphChangeInitDist;
    std::uniform_real_distribution<float> glyphSpawnInitDist;
    std::uniform_real_distribution<float> glyphMouseResponse;
}; 