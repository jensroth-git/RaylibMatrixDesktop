# Raylib Matrix Desktop Wallpaper

A cross-platform Matrix rain live wallpaper built with raylib and C++. Creates an animated Matrix-style digital rain effect that runs as a desktop background.

## Features

- **Cross-platform**: Works on Windows and macOS, untested on linux(X11)
- **Live wallpaper integration**: Properly integrates with the desktop background
- **Interactive**: Mouse interaction creates new rain streams
- **Multi-monitor support**: Can target specific monitors or span across all
- **Occlusion detection**: Automatically detects when windows cover the wallpaper
- **High performance**: Uses raylib for efficient graphics rendering

## Screenshots

![Matrix Rain Demo](images/video.gif)

## Platform Support

### Windows
- Full desktop wallpaper integration
- Reparents window to desktop worker process
- Comprehensive occlusion detection
- Multi-monitor support with per-monitor DPI awareness

### macOS
- Desktop-level window positioning
- Basic occlusion detection
- Multi-monitor support
- Integrates with macOS window management

## Building

### Prerequisites

- CMake 3.16 or higher
- C++20 compatible compiler
- Git (for fetching raylib dependency)

#### Windows
- Visual Studio 2019/2022 or MinGW-w64
- Windows SDK

#### macOS
- Xcode Command Line Tools
- macOS 10.15 or higher

### Build Instructions

#### Clone the Repository
```bash
git clone <repository-url>
cd RaylibMatrixDesktop
```

#### Windows (Visual Studio)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Windows (MinGW)
```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

#### macOS
```bash
mkdir build
cd build
cmake .. -G "Xcode"
cmake --build . --config Release
```

Or for command line build:
```bash
mkdir build
cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Running

#### Windows
```bash
# From build directory
bin/RaylibMatrixDesktop.exe
```

#### macOS
```bash
# From build directory
bin/RaylibMatrixDesktop.app/Contents/MacOS/RaylibMatrixDesktop
```

Or simply open the app bundle:
```bash
open bin/RaylibMatrixDesktop.app
```

## Configuration

You can modify behavior by editing constants in `src/main.cpp`:

- `GLYPH_SCALE`: Size of the matrix characters (0.1-0.3 recommended)
- `TARGET_MONITOR`: Which monitor to target (-1 for all, 0+ for specific)

Matrix rain parameters can be adjusted in `src/matrix_rain.h`:

- `glyphChangeTime`: How fast characters change
- `glyphMoveTickTime`: Speed of falling effect
- `glyphFadeTime`: How long characters fade out
- `spawnTime`: Frequency of new rain streams

## Controls

- **Escape**: Exit the application

## Technical Details

### Architecture

The project uses a platform abstraction layer to handle desktop integration:

- **Windows**: Uses the WorkerW window technique to inject the wallpaper behind desktop icons
- **macOS**: Creates a desktop-level window using Cocoa APIs

### Dependencies

- **raylib 5.5.0**: Graphics library (automatically downloaded via CMake)
- **Platform libraries**:
  - Windows: dwmapi, shcore, user32, gdi32
  - macOS: Cocoa, IOKit, CoreVideo, OpenGL

### Performance Considerations

- Targets 60 FPS but automatically adjusts based on system capability
- Uses occlusion detection to optimize when windows cover the wallpaper
- Delta time-based animation for smooth playback regardless of framerate
- Intelligent asset loading with multiple fallback paths for cross-platform compatibility

## Troubleshooting

### Windows
- If the wallpaper doesn't appear behind icons, try running as administrator
- Ensure Windows Desktop Window Manager is running
- Some third-party desktop customization tools may interfere

### macOS
- The application may need permission to control the screen
- On macOS 10.15+, you may need to grant accessibility permissions
- If the window appears in front, check System Preferences > Mission Control

### General
- If compilation fails, ensure you have all prerequisites installed
- If raylib fails to download, check your internet connection and firewall
- For performance issues, try increasing `GLYPH_SCALE` or decreasing target framerate
- If the application shows a white texture instead of Matrix characters, the asset file couldn't be loaded - ensure you're running from the correct directory

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Adding Platform Support

To add support for a new platform:

1. Create a new implementation file in `src/platform/yourplatform/`
2. Implement all functions from `desktop_integration.h`
3. Add platform detection and linking in `CMakeLists.txt`
4. Test thoroughly and update documentation

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [raylib](https://www.raylib.com/) - Amazing C library for graphics programming
- Original Matrix digital rain effect inspiration from The Matrix (1999)
- Matrix effect implementation inspiration and texture from [Rezmason](https://github.com/Rezmason/matrix)