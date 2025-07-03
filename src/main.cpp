#include <cmath>
#include <iostream>
#include <random>
#include "asset_loader.h"
#include "matrix_rain.h"
#include "platform/desktop_integration.h"
#include "raylib.h"

// Configuration
constexpr float GLYPH_SCALE = 0.15f;
constexpr int TARGET_MONITOR = -1; // -1 for all monitors, 0+ for specific monitor

// Global random distributions for mouse spawning
std::default_random_engine mouseGenerator;
std::uniform_real_distribution<float> glyphMouseResponse(0.0f, 1.0f);

int main()
{
	// Initialize desktop integration
	if (!DesktopIntegration::Initialize()) {
		DesktopIntegration::ShowAlert("Error", "Failed to initialize desktop integration!");
		return -1;
	}

	// Get target monitor info
	MonitorInfo monitor = DesktopIntegration::GetWallpaperTarget(TARGET_MONITOR);

	std::cout << "Target monitor: " << monitor.width << "x" << monitor.height << " at (" << monitor.x << ", "
			  << monitor.y << ")" << std::endl;

	// Initialize raylib window
	#ifdef __APPLE__
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_UNDECORATED);
	#endif

	#ifdef _WIN32
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
	#endif

	#ifdef __linux__
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
	#endif

	// Set target FPS
	InitWindow(monitor.width, monitor.height, "Matrix Desktop Wallpaper");

	// Set target FPS
	SetTargetFPS(60);

	// Initialize matrix glyph system
	MatrixGlyph::Init();

	// Create matrix rain effect
	MatrixRain matrixRain(monitor.width, monitor.height, GLYPH_SCALE);

	// Configure the window for wallpaper mode
	void *windowHandle = GetWindowHandle();
	DesktopIntegration::ConfigureWallpaperWindow(windowHandle, monitor);

	// Create render texture for bloom effect
	RenderTexture2D target = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
	SetTextureWrap(target.texture, TEXTURE_WRAP_CLAMP);

	// Load bloom shader
	std::string shaderPath = ASSET_PATH("bloom.fs");
	Shader bloom = LoadShader(0, shaderPath.c_str());
	if (!bloom.id) {
		DesktopIntegration::ShowAlert("Error", "Failed to load bloom shader!");
		return -1;
	}

	// Set shader uniforms
	int renderSizeLoc = GetShaderLocation(bloom, "size");
	Vector2 renderSize = {(float)GetRenderWidth(), (float)GetRenderHeight()};
	SetShaderValue(bloom, renderSizeLoc, &renderSize, SHADER_UNIFORM_VEC2);

	// Initialize mouse generator
	mouseGenerator.seed(time(nullptr));

	// Main loop
	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime();

		// Update desktop integration mouse state
		DesktopIntegration::UpdateMouseState();

		// Check for occlusion (skip rendering if mostly occluded)
		if (DesktopIntegration::IsMonitorOccluded(monitor, 0.90)) {
			WaitTime(0.1);
			continue;
		}

		// Get mouse position and calculate cell position
		auto raylibMousePos = DesktopIntegration::GetMousePosition();
		Vector2 mouseCellPos = matrixRain.GetCellPositionFromPoint(Vector2 {raylibMousePos.x, raylibMousePos.y});

		float mouseSpawnChance = 0.0f;

		// Check if mouse has moved to a new cell
		if (mouseCellPos.x != matrixRain.lastMouseCellPos.x || mouseCellPos.y != matrixRain.lastMouseCellPos.y) {
			matrixRain.lastMouseCellPos = mouseCellPos;
			mouseSpawnChance = 0.6f; // 60% chance to spawn when mouse moves
		}

		// Spawn a new cell if the mouse moved and random check passes
		if (glyphMouseResponse(mouseGenerator) < mouseSpawnChance) {
			matrixRain.SetSpawnCell(matrixRain.lastMouseCellPos);
		}

		// Update matrix rain
		matrixRain.Update(deltaTime);

#if defined(_WIN32) || defined(__linux__)
		BeginTextureMode(target);
		{
			ClearBackground(BLACK);
			matrixRain.Draw();
		}
		EndTextureMode();

		// Render to screen with bloom effect
		BeginDrawing();
		{
			// Apply bloom shader
			BeginShaderMode(bloom);
			{
				DrawTexturePro(
					target.texture,
					{0, 0, (float)target.texture.width, (float)-target.texture.height},
					{0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
					{0, 0},
					0,
					WHITE
				);
			}
			EndShaderMode();
#endif

#ifdef __APPLE__
			BeginDrawing();
			{
				ClearBackground(BLACK);
				matrixRain.Draw();
#endif

// Optional: Draw debug info
#if 1
		const int yStart = 100;
		const int yStep = 30;
		DrawText(TextFormat("FPS: %d", GetFPS()), 10, yStart, 20, GREEN);
		DrawText(TextFormat("Mouse: %.0f, %.0f", raylibMousePos.x, raylibMousePos.y), 10, yStart + yStep, 20, GREEN);
		DrawText(TextFormat("Monitor: %dx%d", monitor.width, monitor.height), 10, yStart + yStep * 2, 20, GREEN);
		DrawText(TextFormat("Screen: %dx%d", GetScreenWidth(), GetScreenHeight()), 10, yStart + yStep * 3, 20, GREEN);
		DrawText(TextFormat("Render: %dx%d", GetRenderWidth(), GetRenderHeight()), 10, yStart + yStep * 4, 20, GREEN);
#endif
			}
			EndDrawing();

			// Handle window events
			if (IsKeyPressed(KEY_ESCAPE)) {
				break;
			}
		}

		// Cleanup
		UnloadShader(bloom);
		UnloadRenderTexture(target);
		MatrixGlyph::Cleanup();
		CloseWindow();
		DesktopIntegration::Cleanup();

		return 0;
	}
