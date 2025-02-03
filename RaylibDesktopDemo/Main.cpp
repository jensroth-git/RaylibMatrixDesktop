#include "RaylibDesktop.h"
#include "raylib.h"
#include <iostream>
#include <vector>
#include <random>

#include "hsl_rgb.h"

using namespace std;

// Matrix rain settings
// The scale of the glyphs
float glyphScale = 0.15f;

//the time it takes for a glyph to change in seconds
float glyphChangeTime = 0.4f;

//the time it takes for a glyph to move down in seconds
float glyphMoveTickTime = 0.037f;

//the time it takes for a glyph to fade out in seconds
float glyphFadeTime = 1.5f;

//the time it takes for new glyphs to spawn in a column in seconds
float spawnTime = 5.0;

default_random_engine generator;
uniform_int_distribution<int> glyphDist;
uniform_real_distribution<float> glyphChangeInitDist(0.0f, glyphChangeTime);
uniform_real_distribution<float> glyphSpawnInitDist(0.0f, 1.0f);

class MatrixGlyph {
public:
	//statics
	static Texture glyphTexture;

	static const int xGlyphs = 8;
	static const int yGlyphs = 8;
	static const int glyphLength = xGlyphs * yGlyphs - 7;

	static void Init()
	{
		glyphTexture = LoadTexture("assets/texture_simplified.png");

		glyphDist = uniform_int_distribution<int>(0, glyphLength);

		//initialize the random number generator with the int dist
		generator.seed(time(nullptr));
	}

	static Vector2 GetGlyphSize()
	{
		return {
			(float)glyphTexture.width / xGlyphs,
			(float)glyphTexture.height / yGlyphs
		};
	}

	//glyph is the index of the glyph in the texture up to glyphLength
	static void DrawGlyph(int x, int y, float scale, int glyph, float opacity)
	{
		if (glyph < 0 || glyph >= glyphLength)
		{
			glyph = 0;
		}

		Rectangle glyphRect;
		int xGlyphTexIndex = glyph % xGlyphs;
		int yGlyphTexIndex = glyph / xGlyphs;

		glyphRect.x = GetGlyphSize().x * xGlyphTexIndex;
		glyphRect.y = GetGlyphSize().y * yGlyphTexIndex;
		glyphRect.width = GetGlyphSize().x;
		glyphRect.height = GetGlyphSize().y;

		//draw with opacity and color
		vector<GradientStop> stops = {
			{ 0.0f, { 0.3f, 0.9f, 0.0f } },
			{ 0.2f, { 0.3f, 0.9f, 0.2f } },
			{ 0.7f, { 0.3f, 0.9f, 0.5f } },
			{ 0.9f, { 0.3f, 0.9f, 0.7f } },
			{ 1.0f, { 0.3f, 0.9f, 0.7f } }
		};

		RGB rgbColor = lerpHSL(stops, opacity);
		Color color = { (unsigned char)rgbColor.r, (unsigned char)rgbColor.g, (unsigned char)rgbColor.b, 255 };

		DrawTexturePro(glyphTexture, glyphRect,
			{
				(float)x,
				(float)y,
				GetGlyphSize().x * scale,
				GetGlyphSize().y * scale
			},
			{
				0,
				0
			},
			0,
			color
		);
	}
};

//the sprite sheet texture
Texture MatrixGlyph::glyphTexture;

struct GlyphCell
{
	int currentGlyph = glyphDist(generator);
	float timeTillGlyphChange = glyphChangeInitDist(generator);
	float opacity = 0.0f;
	bool spawnCell = false;
};

class MatrixRain
{
public:
	MatrixRain(int screenWidth, int screenHeight, float glyphScale)
		: glyphScale(glyphScale)
	{
		columnWidth = (int)(MatrixGlyph::GetGlyphSize().x * glyphScale);
		rowHeight = (int)(MatrixGlyph::GetGlyphSize().y * glyphScale);

		//calculate the number of columns and rows based on the screen size
		columns = (int)ceil((double)screenWidth / columnWidth);
		rows = (int)ceil((double)screenHeight / rowHeight);

		//seed the random number generator
		generator.seed(time(nullptr));

		//initialize the glyphs
		glyphs.resize(columns);

		for (int col = 0; col < columns; col++)
		{
			glyphs[col].resize(rows);
		}

		//initialize the spawn cells
		spawnTimes.resize(columns);

		//initialize the spawn times
		for (int col = 0; col < columns; col++)
		{
			spawnTimes[col] = glyphSpawnInitDist(generator) * spawnTime;
		}
	}

	void Update(float delta)
	{
		if (delta > 0.1f)
		{
			delta = 0.1f;
		}

		bool isTick = false;

		tickTime += delta;

		if (tickTime > glyphMoveTickTime)
		{
			tickTime = 0.0f;
			isTick = true;
		}

		for (int col = 0; col < columns; col++)
		{
			//update spawns
			spawnTimes[col] -= delta;

			if (spawnTimes[col] < 0.0f)
			{
				spawnTimes[col] = spawnTime;
				glyphs[col][0].spawnCell = true;
			}

			for (int row = rows - 1; row >= 0; row--)
			{
				//subtract opacity from the cell
				glyphs[col][row].opacity -= delta / glyphFadeTime;

				if (glyphs[col][row].opacity < 0.0f)
				{
					glyphs[col][row].opacity = 0.0f;
				}

				//change the glyph on the cell
				glyphs[col][row].timeTillGlyphChange -= delta;

				if (glyphs[col][row].timeTillGlyphChange < 0.0f)
				{
					glyphs[col][row].timeTillGlyphChange = glyphChangeTime;
					glyphs[col][row].currentGlyph = glyphDist(generator);
				}

				//move down the spawn cells on tick
				if (glyphs[col][row].spawnCell && isTick)
				{
					//spawn a new cell
					glyphs[col][row].spawnCell = false;
					glyphs[col][row].currentGlyph = glyphDist(generator);
					glyphs[col][row].opacity = 1.0f;

					//move spawn cell one row down if possible
					if (row != rows - 1)
					{
						glyphs[col][row + 1].spawnCell = true;
					}
				}
			}
		}
	}

	void Draw() const
	{
		for (int col = 0; col < columns; col++)
		{
			for (int row = 0; row < rows; row++)
			{
				if (glyphs[col][row].opacity > 0.0f)
				{
					MatrixGlyph::DrawGlyph(col * columnWidth, row * rowHeight, glyphScale, glyphs[col][row].currentGlyph, glyphs[col][row].opacity);
				}
			}
		}
	}

	Vector2 GetCellPositionFromPoint(Vector2 point) const
	{
		return {
			floor(point.x / columnWidth),
			floor(point.y / rowHeight)
		};
	}

	void SetSpawnCell(Vector2 point)
	{
		Vector2 cellPos = GetCellPositionFromPoint(point);
		if (cellPos.x >= 0 && cellPos.x < columns && cellPos.y >= 0 && cellPos.y < rows)
		{
			glyphs[(int)cellPos.x][(int)cellPos.y].spawnCell = true;
		}
	}

private:
	int columns, rows;
	int screenWidth, screenHeight;
	int columnWidth, rowHeight;
	float glyphScale;
	float tickTime = 0.0f;

	// Stores glyphs for each column
	vector<vector<GlyphCell>> glyphs; 
	// Stores spawn times for each column
	vector<float> spawnTimes; 
};

int main()
{
	// Initializes desktop replacement magic
	InitRaylibDesktop();

	//setups the desktop (-1 is the entire desktop spanning all monitors)
	MonitorInfo monitorInfo = GetWallpaperTarget(-1);

	// Initialize the raylib window
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(monitorInfo.monitorWidth, monitorInfo.monitorHeight, "Matrix Rain Effect with Raylib");

	// Retrieve the handle for the raylib-created window.
	void* raylibWindowHandle = GetWindowHandle();
	RaylibDesktopReparentWindow(raylibWindowHandle);

	// Configure the desktop positioning.
	ConfigureDesktopPositioning(monitorInfo);

	MatrixGlyph::Init();

	MatrixRain matrixRain(monitorInfo.monitorWidth, monitorInfo.monitorHeight, glyphScale);

	RenderTexture2D target = LoadRenderTexture(monitorInfo.monitorWidth, monitorInfo.monitorHeight);

	SetTextureWrap(target.texture, TEXTURE_WRAP_CLAMP);

	//load shader 
	Shader bloom = LoadShader(0, "assets/bloom.fs");

	if (!bloom.id)
	{
		cout << "Failed to load shader" << endl;
		return 0;
	}

	//render size
	int renderSizeLoc = GetShaderLocation(bloom, "size");
	Vector2 renderSize = { (float)monitorInfo.monitorWidth, (float)monitorInfo.monitorHeight };
	SetShaderValue(bloom, renderSizeLoc, &renderSize, SHADER_UNIFORM_VEC2);

	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		RaylibDesktopUpdateMouseState();

		//skip rendering if the wallpaper is occluded more than 95%
		if (IsMonitorOccluded(monitorInfo, 0.95))
		{
			WaitTime(0.1);
			continue;
		}

		Vector2 mousePos = RaylibDesktopGetMousePosition();
		matrixRain.SetSpawnCell(mousePos);

		// Update the rain effect
		matrixRain.Update(GetFrameTime());

		//draw to the render texture
		BeginTextureMode(target);

		ClearBackground(BLACK);

		// Draw the rain effect
		matrixRain.Draw();

		EndTextureMode();

		// Draw the render texture
		BeginDrawing();

		// Apply bloom shader
		BeginShaderMode(bloom);

		// NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
		DrawTextureRec(target.texture, Rectangle{ 0, 0, (float)target.texture.width, (float)-target.texture.height }, Vector2{ 0, 0 }, WHITE);
		EndShaderMode();

		//DrawFPS(30, 30);
		EndDrawing();
	}

	CloseWindow();

	CleanupRaylibDesktop();

	return 0;
}

