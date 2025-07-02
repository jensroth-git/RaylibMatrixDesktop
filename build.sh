#!/bin/bash

echo "Building Raylib Matrix Desktop for macOS/Linux..."

# Check if build directory exists, if not create it
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
cmake ..

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo "Building project..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    # Linux and others
    make -j$(nproc 2>/dev/null || echo 4)
fi

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build completed successfully!"

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Application bundle location: build/bin/RaylibMatrixDesktop.app"
    echo "Assets location: build/bin/assets/"
    echo ""
    read -p "Run the application? (y/n): " choice
    if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
        cd bin && open RaylibMatrixDesktop.app
    fi
else
    echo "Executable location: build/bin/RaylibMatrixDesktop"
    echo ""
    read -p "Run the application? (y/n): " choice
    if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
        ./bin/RaylibMatrixDesktop
    fi
fi 