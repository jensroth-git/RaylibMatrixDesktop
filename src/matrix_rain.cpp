#include "matrix_rain.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include "asset_loader.h"

#include <lumin.h>

// Initialize static members
Texture MatrixGlyph::glyphTexture;

void MatrixGlyph::Init()
{
	std::string texturePath = ASSET_PATH("texture_simplified.png");
	glyphTexture = LoadTexture(texturePath.c_str());

	// If loading failed, notify the user
	if (glyphTexture.id == 0) {
		lumin::ShowAlert("Error", "Failed to load glyph texture!");
		exit(1);
	}
}

void MatrixGlyph::Cleanup()
{
	UnloadTexture(glyphTexture);
}

Vector2 MatrixGlyph::GetGlyphSize()
{
	return {static_cast<float>(glyphTexture.width) / xGlyphs, static_cast<float>(glyphTexture.height) / yGlyphs};
}

void MatrixGlyph::DrawGlyph(int x, int y, float scale, int glyph, float opacity)
{
	if (glyph < 0 || glyph >= glyphLength) {
		glyph = 0;
	}

	Rectangle glyphRect;
	const int xGlyphTexIndex = glyph % xGlyphs;
	const int yGlyphTexIndex = glyph / xGlyphs;

	glyphRect.x = GetGlyphSize().x * static_cast<float>(xGlyphTexIndex);
	glyphRect.y = GetGlyphSize().y * static_cast<float>(yGlyphTexIndex);
	glyphRect.width = GetGlyphSize().x;
	glyphRect.height = GetGlyphSize().y;

	// Create gradient for matrix green effect
	std::vector<GradientStop> stops = {
		{0.0f, {0.3f, 0.9f, 0.0f}},
		{0.2f, {0.3f, 0.9f, 0.2f}},
		{0.7f, {0.3f, 0.9f, 0.5f}},
		{0.9f, {0.3f, 0.9f, 0.7f}},
		{1.0f, {0.3f, 0.9f, 0.7f}}
	};

	RGB rgbColor = HSLRGB::lerpHSL(stops, opacity);
	Color color = {(unsigned char)rgbColor.r, (unsigned char)rgbColor.g, (unsigned char)rgbColor.b, 255};

	DrawTexturePro(
		glyphTexture,
		glyphRect,
		{(float)x, (float)y, GetGlyphSize().x * scale, GetGlyphSize().y * scale},
		{0, 0},
		0,
		color
	);
}

MatrixRain::MatrixRain(int screenWidth, int screenHeight, float glyphScale) :
	glyphScale(glyphScale), glyphChangeInitDist(0.0f, glyphChangeTime), glyphSpawnInitDist(0.0f, 1.0f),
	glyphMouseResponse(0.0f, 1.0f)
{
	columnWidth = (int)(MatrixGlyph::GetGlyphSize().x * glyphScale);
	rowHeight = (int)(MatrixGlyph::GetGlyphSize().y * glyphScale);

	// Calculate the number of columns and rows based on the screen size
	columns = (int)ceil((double)screenWidth / columnWidth);
	rows = (int)ceil((double)screenHeight / rowHeight);

	// Initialize random number generation
	generator.seed(time(nullptr));
	glyphDist = std::uniform_int_distribution<int>(0, MatrixGlyph::glyphLength - 1);

	// Initialize the glyphs
	glyphs.resize(columns);
	for (int col = 0; col < columns; col++) {
		glyphs[col].resize(rows);
		for (int row = 0; row < rows; row++) {
			glyphs[col][row].currentGlyph = glyphDist(generator);
			glyphs[col][row].timeTillGlyphChange = glyphChangeInitDist(generator);
		}
	}

	// Initialize the spawn times
	spawnTimes.resize(columns);
	for (int col = 0; col < columns; col++) {
		spawnTimes[col] = glyphSpawnInitDist(generator) * spawnTime;
	}
}

void MatrixRain::Update(float delta)
{
	delta = std::min(delta, 0.1f);

	bool isTick = false;

	tickTime += delta;

	if (tickTime > glyphMoveTickTime) {
		tickTime = 0.0f;
		isTick = true;
	}

	for (int col = 0; col < columns; col++) {
		// Update spawns
		spawnTimes[col] -= delta;

		if (spawnTimes[col] < 0.0f) {
			spawnTimes[col] = spawnTime;
			glyphs[col][0].spawnCell = true;
		}

		for (int row = rows - 1; row >= 0; row--) {
			// Subtract opacity from the cell
			glyphs[col][row].opacity -= delta / glyphFadeTime;
			glyphs[col][row].opacity = std::max(glyphs[col][row].opacity, 0.0f);

			// Change the glyph on the cell
			glyphs[col][row].timeTillGlyphChange -= delta;

			if (glyphs[col][row].timeTillGlyphChange < 0.0f) {
				glyphs[col][row].timeTillGlyphChange = glyphChangeTime;
				glyphs[col][row].currentGlyph = glyphDist(generator);
			}

			// Move down the spawn cells on tick
			if (glyphs[col][row].spawnCell && isTick) {
				// Spawn a new cell
				glyphs[col][row].spawnCell = false;
				glyphs[col][row].currentGlyph = glyphDist(generator);
				glyphs[col][row].opacity = 1.0f;

				// Move spawn cell one row down if possible
				if (row != rows - 1) {
					bool shouldSpawn = true;

					// If mouse is over the cell, 70% chance to block the drop
					if (lastMouseCellPos.x == col && lastMouseCellPos.y == row &&
						glyphMouseResponse(generator) < 0.7f) {
						shouldSpawn = false;
					}

					if (shouldSpawn) {
						glyphs[col][row + 1].spawnCell = true;
					}
				}
			}
		}
	}
}

void MatrixRain::Draw() const
{
	for (int col = 0; col < columns; col++) {
		for (int row = 0; row < rows; row++) {
			if (glyphs[col][row].opacity > 0.0f) {
				int x = col * columnWidth;
				int y = row * rowHeight;

				MatrixGlyph::DrawGlyph(x, y, glyphScale, glyphs[col][row].currentGlyph, glyphs[col][row].opacity);
			}
		}
	}
}

Vector2 MatrixRain::GetCellPositionFromPoint(Vector2 point) const
{
	return {(float)floor(point.x / columnWidth), (float)floor(point.y / rowHeight)};
}

void MatrixRain::SetSpawnCell(Vector2 cellPos)
{
	int col = (int)cellPos.x;
	int row = (int)cellPos.y;

	if (col >= 0 && col < columns && row >= 0 && row < rows) {
		glyphs[col][row].spawnCell = true;
	}
}
