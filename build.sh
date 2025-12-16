#!/bin/bash

#!/bin/bash

# Script to build the ENIGMA project

echo "======================================"
echo "  ENIGMA - Build Script"
echo "======================================"
echo ""

# Check if SimGrid is installed
SIMGRID_FOUND=false
SIMGRID_PATH=""

# Method 1: Check with pkg-config
if pkg-config --exists simgrid 2>/dev/null; then
    echo "✓ SimGrid detected via pkg-config"
    SIMGRID_FOUND=true
# Method 2: Check in /opt/simgrid-4.1
elif [ -d "/opt/simgrid-4.1" ]; then
    echo "✓ SimGrid detected in /opt/simgrid-4.1"
    SIMGRID_PATH="/opt/simgrid-4.1"
    SIMGRID_FOUND=true
    export CMAKE_PREFIX_PATH="$SIMGRID_PATH:$CMAKE_PREFIX_PATH"
# Method 3: Check in /usr/local
elif [ -d "/usr/local/include/simgrid" ] || [ -f "/usr/local/lib/libsimgrid.so" ]; then
    echo "✓ SimGrid detected in /usr/local"
    SIMGRID_PATH="/usr/local"
    SIMGRID_FOUND=true
    export CMAKE_PREFIX_PATH="$SIMGRID_PATH:$CMAKE_PREFIX_PATH"
# Method 4: Check in /opt for other installations
elif [ -d "/opt/lib/libsimgrid.so" ] || [ -d "/opt/include/simgrid" ]; then
    echo "✓ SimGrid detected in /opt"
    SIMGRID_PATH="/opt"
    SIMGRID_FOUND=true
    export CMAKE_PREFIX_PATH="$SIMGRID_PATH:$CMAKE_PREFIX_PATH"
fi

if [ "$SIMGRID_FOUND" = false ]; then
    echo "Warning: SimGrid not detected"
    echo "   Make sure SimGrid 4.1+ is installed"
    echo ""
    echo "   Installation options:"
    echo "   1. Via package manager: sudo apt install simgrid"
    echo "   2. Manual installation in /opt/simgrid-4.1"
    echo "   3. Build from source: https://simgrid.org"
    echo ""
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    if [ -n "$SIMGRID_PATH" ]; then
        echo "   Path: $SIMGRID_PATH"
    fi
fi
echo ""

# Create necessary directories
echo "📁 Creating directories..."
mkdir -p build
mkdir -p platforms
mkdir -p deployments

# Compile
echo ""
echo "🔨 Building project..."
cd build

# Pass SimGrid path to CMake if detected manually
if [ -n "$SIMGRID_PATH" ]; then
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$SIMGRID_PATH"
else
    cmake .. -DCMAKE_BUILD_TYPE=Release
fi

if [ $? -ne 0 ]; then
    echo "Error in CMake configuration"
    exit 1
fi

make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Error in compilation"
    exit 1
fi

echo ""
echo "Build successful!"
echo ""
echo "Generated executables in: build/"
echo "  - platform_generator    : Platform generator"
echo "  - edge_computing_app    : Edge application"
echo "  - fog_analytics_app     : Fog application"
echo "  - hybrid_cloud_app      : Hybrid application"
echo "  - data_offloading_app   : Offloading application"
echo ""
echo "To generate a platform:"
echo "  ./build/platform_generator hybrid 10 5 3"
echo ""
echo "To run an application:"
echo "  ./build/edge_computing_app platforms/edge_platform.xml"

