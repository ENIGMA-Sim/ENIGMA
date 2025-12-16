#!/bin/bash

# ENIGMA project verification script

echo "╔════════════════════════════════════════════════════════╗"
echo "║     ENIGMA Project - Integrity Verification           ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

EXIT_CODE=0

# Verify directory structure
echo "📁 Verifying directory structure..."
DIRS=("include" "include/platform" "include/utils" "src" "src/apps" "src/platform" "src/tools" "src/utils" "examples")
for dir in "${DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo "  ✓ $dir"
    else
        echo "  ✗ $dir MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify headers
echo "📄 Verifying headers..."
HEADERS=(
    "include/platform/PlatformGenerator.hpp"
    "include/platform/PlatformBuilder.hpp"
    "include/platform/EdgePlatform.hpp"
    "include/platform/FogPlatform.hpp"
    "include/platform/CloudPlatform.hpp"
    "include/utils/XMLWriter.hpp"
)
for header in "${HEADERS[@]}"; do
    if [ -f "$header" ]; then
        echo "  ✓ $header"
    else
        echo "  ✗ $header MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify implementations
echo "🔧 Verifying implementations..."
SOURCES=(
    "src/platform/PlatformGenerator.cpp"
    "src/platform/PlatformBuilder.cpp"
    "src/platform/EdgePlatform.cpp"
    "src/platform/FogPlatform.cpp"
    "src/platform/CloudPlatform.cpp"
    "src/utils/XMLWriter.cpp"
)
for source in "${SOURCES[@]}"; do
    if [ -f "$source" ]; then
        echo "  ✓ $source"
    else
        echo "  ✗ $source MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify applications
echo "🚀 Verifying applications..."
APPS=(
    "src/apps/edge_computing.cpp"
    "src/apps/fog_analytics.cpp"
    "src/apps/hybrid_cloud.cpp"
    "src/apps/data_offloading.cpp"
)
for app in "${APPS[@]}"; do
    if [ -f "$app" ]; then
        echo "  ✓ $app"
    else
        echo "  ✗ $app MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify tools
echo "🛠️  Verifying tools..."
TOOLS=(
    "src/tools/platform_generator_main.cpp"
    "examples/generate_all_platforms.cpp"
)
for tool in "${TOOLS[@]}"; do
    if [ -f "$tool" ]; then
        echo "  ✓ $tool"
    else
        echo "  ✗ $tool MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify documentation
echo "📚 Verifying documentation..."
DOCS=(
    "README.md"
    "USAGE.md"
    "QUICKSTART.md"
    "ARCHITECTURE.md"
    "PROJECT_SUMMARY.md"
    "LICENSE"
)
for doc in "${DOCS[@]}"; do
    if [ -f "$doc" ]; then
        echo "  ✓ $doc"
    else
        echo "  ✗ $doc MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify scripts
echo "🔨 Verifying scripts..."
SCRIPTS=("build.sh" "run_examples.sh")
for script in "${SCRIPTS[@]}"; do
    if [ -f "$script" ]; then
        if [ -x "$script" ]; then
            echo "  ✓ $script (executable)"
        else
            echo "  ⚠ $script (not executable)"
        fi
    else
        echo "  ✗ $script MISSING"
        EXIT_CODE=1
    fi
done
echo ""

# Verify CMakeLists.txt
echo "⚙️  Verifying build system..."
if [ -f "CMakeLists.txt" ]; then
    echo "  ✓ CMakeLists.txt"
else
    echo "  ✗ CMakeLists.txt MISSING"
    EXIT_CODE=1
fi
echo ""

# Verify VS Code configuration
echo "🔧 Verifying VS Code configuration..."
VSCODE_FILES=(
    ".vscode/launch.json"
    ".vscode/tasks.json"
    ".vscode/c_cpp_properties.json"
)
for vscode_file in "${VSCODE_FILES[@]}"; do
    if [ -f "$vscode_file" ]; then
        echo "  ✓ $vscode_file"
    else
        echo "  ⚠ $vscode_file not found (optional)"
    fi
done
echo ""

# Statistics
echo "📊 Project statistics..."
CPP_COUNT=$(find . -name "*.cpp" 2>/dev/null | wc -l)
HPP_COUNT=$(find . -name "*.hpp" 2>/dev/null | wc -l)
echo "  ✓ .cpp files: $CPP_COUNT"
echo "  ✓ .hpp files: $HPP_COUNT"

if command -v cloc &> /dev/null; then
    echo ""
    echo "📈 Lines of code (detailed):"
    cloc --quiet src/ include/ examples/ 2>/dev/null || echo "  (cloc not available)"
fi
echo ""

# Verify dependencies
echo "🔍 Verifying dependencies..."
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -1)
    echo "  ✓ CMake: $CMAKE_VERSION"
else
    echo "  ✗ CMake not found"
    EXIT_CODE=1
fi

if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -1)
    echo "  ✓ G++: $GCC_VERSION"
else
    echo "  ✗ G++ not found"
    EXIT_CODE=1
fi

if pkg-config --exists simgrid 2>/dev/null; then
    SIMGRID_VERSION=$(pkg-config --modversion simgrid)
    echo "  ✓ SimGrid: $SIMGRID_VERSION"
else
    echo "  ⚠ SimGrid not detected (required for compilation)"
fi
echo ""

# Final result
echo "═══════════════════════════════════════════════════════"
if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ VERIFICATION COMPLETE: Project is intact and complete"
    echo ""
    echo "Next steps:"
    echo "  1. Install SimGrid if you don't have it yet"
    echo "  2. Run: ./build.sh"
    echo "  3. Run: ./run_examples.sh"
else
    echo "⚠️  WARNING: Some files are missing"
    echo "   Review the errors above"
fi
echo "═══════════════════════════════════════════════════════"

exit $EXIT_CODE

