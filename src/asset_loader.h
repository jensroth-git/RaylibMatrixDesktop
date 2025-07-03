#pragma once

#include <string>

// Simple macro-based asset loading for different platforms
#ifdef __APPLE__
// For macOS app bundles, assets are in the Resources directory
// For development builds, they're next to the executable
#define ASSET_PATH(filename) GetAssetPath(filename)
std::string GetAssetPath(const char *filename);
#else
// For Windows and other platforms, assets are in the assets directory next to executable
#define ASSET_PATH(filename) (std::string("assets/") + filename)
#endif
