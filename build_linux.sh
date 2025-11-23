#!/bin/bash

# PaddleOCR-json Linux Build Script
# This script automates the compilation of PaddleOCR-json on Linux systems
#
# Usage:
#   ./build_linux.sh [options]
#
# Options:
#   -h, --help              Show this help message
#   -j, --jobs N            Number of parallel jobs for compilation (default: auto-detect)
#   --skip-deps             Skip dependency installation
#   --skip-downloads        Skip downloading resources (assumes they exist)
#   --opencv-system         Use system OpenCV (libopencv-dev)
#   --opencv-prebuilt       Download prebuilt OpenCV package (default)
#   --opencv-compile        Compile OpenCV from source
#   --install               Install after building (to /usr/ by default)
#   --install-prefix PATH   Install to specified path
#   --debug                 Build in Debug mode (default: Release)
#   --clean                 Clean build directory before building

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
JOBS=$(nproc 2>/dev/null || echo 4)
SKIP_DEPS=false
SKIP_DOWNLOADS=false
OPENCV_MODE="prebuilt"  # prebuilt, system, compile
DO_INSTALL=false
INSTALL_PREFIX=""
BUILD_TYPE="Release"
CLEAN_BUILD=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            grep "^#" "$0" | grep -v "#!/bin/bash" | sed 's/^# \?//'
            exit 0
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --skip-deps)
            SKIP_DEPS=true
            shift
            ;;
        --skip-downloads)
            SKIP_DOWNLOADS=true
            shift
            ;;
        --opencv-system)
            OPENCV_MODE="system"
            shift
            ;;
        --opencv-prebuilt)
            OPENCV_MODE="prebuilt"
            shift
            ;;
        --opencv-compile)
            OPENCV_MODE="compile"
            shift
            ;;
        --install)
            DO_INSTALL=true
            shift
            ;;
        --install-prefix)
            INSTALL_PREFIX="$2"
            DO_INSTALL=true
            shift 2
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
CPP_DIR="$PROJECT_ROOT/cpp"
SOURCE_DIR="$CPP_DIR/.source"

log_info "PaddleOCR-json Linux Build Script"
log_info "===================================="
log_info "Build Type: $BUILD_TYPE"
log_info "Parallel Jobs: $JOBS"
log_info "OpenCV Mode: $OPENCV_MODE"
log_info ""

# Step 1: Check CPU compatibility (AVX support)
log_info "Step 1: Checking CPU compatibility..."
if ! lscpu | grep -q avx; then
    log_error "Your CPU does not support AVX instruction set!"
    log_error "PaddleOCR-json requires AVX support to run."
    log_error "Consider using RapidOCR-json instead: https://github.com/hiroi-sora/RapidOCR-json"
    exit 1
fi
log_success "CPU supports AVX instruction set"

# Step 2: Install dependencies
if [ "$SKIP_DEPS" = false ]; then
    log_info "Step 2: Installing system dependencies..."

    if ! command -v apt-get &> /dev/null; then
        log_warning "apt-get not found. Skipping automatic dependency installation."
        log_warning "Please manually install: wget tar zip unzip git gcc g++ cmake make libgomp1"
    else
        log_info "Installing packages: wget tar zip unzip git gcc g++ cmake make libgomp1"
        if [ "$OPENCV_MODE" = "system" ]; then
            sudo apt-get update
            sudo apt-get install -y wget tar zip unzip git gcc g++ cmake make libgomp1 libopencv-dev
        else
            sudo apt-get update
            sudo apt-get install -y wget tar zip unzip git gcc g++ cmake make libgomp1
        fi
        log_success "System dependencies installed"
    fi
else
    log_info "Step 2: Skipping dependency installation (--skip-deps)"
fi

# Step 3: Prepare source directory
log_info "Step 3: Preparing source directory..."
mkdir -p "$SOURCE_DIR"
cd "$SOURCE_DIR"
log_success "Source directory ready: $SOURCE_DIR"

# Step 4: Download and prepare resources
if [ "$SKIP_DOWNLOADS" = false ]; then
    log_info "Step 4: Downloading and preparing resources..."

    # Download Paddle Inference library
    PADDLE_INFERENCE_FILE="paddle_inference.tgz"
    PADDLE_INFERENCE_DIR="paddle_inference_manylinux_cpu_avx_mkl_gcc8.2"

    if [ ! -d "$PADDLE_INFERENCE_DIR" ]; then
        log_info "Downloading Paddle Inference library..."
        if [ ! -f "$PADDLE_INFERENCE_FILE" ]; then
            wget https://paddle-inference-lib.bj.bcebos.com/3.0.0-beta1/cxx_c/Linux/CPU/gcc8.2_avx_mkl/paddle_inference.tgz
        fi
        log_info "Extracting Paddle Inference library..."
        tar -xf "$PADDLE_INFERENCE_FILE"
        mv paddle_inference "$PADDLE_INFERENCE_DIR"
        log_success "Paddle Inference library ready"
    else
        log_info "Paddle Inference library already exists, skipping download"
    fi

    # Download models
    MODELS_FILE="models_v1.4.1.zip"
    MODELS_DIR="models"

    if [ ! -d "$MODELS_DIR" ]; then
        log_info "Downloading model library..."
        if [ ! -f "$MODELS_FILE" ]; then
            wget https://github.com/hiroi-sora/PaddleOCR-json/releases/download/v1.4.1-dev/models_v1.4.1.zip
        fi
        log_info "Extracting model library..."
        unzip -q "$MODELS_FILE"
        log_success "Model library ready"
    else
        log_info "Model library already exists, skipping download"
    fi

    # Prepare OpenCV
    case "$OPENCV_MODE" in
        prebuilt)
            log_info "Downloading prebuilt OpenCV package..."
            OPENCV_FILE="opencv-release_debian_x86-64.zip"
            OPENCV_DIR="opencv-release"

            if [ ! -d "$OPENCV_DIR" ]; then
                if [ ! -f "$OPENCV_FILE" ]; then
                    wget https://github.com/hiroi-sora/PaddleOCR-json/releases/download/v1.4.0-beta.2/opencv-release_debian_x86-64.zip
                fi
                log_info "Extracting OpenCV..."
                unzip -q "$OPENCV_FILE"
                log_success "Prebuilt OpenCV ready"
            else
                log_info "OpenCV already exists, skipping download"
            fi
            ;;

        compile)
            log_info "Compiling OpenCV from source..."
            OPENCV_SOURCE="opencv-4.10.0"
            OPENCV_ZIP="opencv.zip"

            if [ ! -d "$OPENCV_SOURCE" ]; then
                if [ ! -f "$OPENCV_ZIP" ]; then
                    wget -O "$OPENCV_ZIP" https://github.com/opencv/opencv/archive/refs/tags/4.10.0.zip
                fi
                unzip -q "$OPENCV_ZIP"
            fi

            if [ -f "$CPP_DIR/tools/linux_build_opencv.sh" ]; then
                log_info "Running OpenCV build script..."
                "$CPP_DIR/tools/linux_build_opencv.sh" "$OPENCV_SOURCE"
                log_success "OpenCV compiled successfully"
            else
                log_error "OpenCV build script not found!"
                exit 1
            fi
            ;;

        system)
            log_info "Using system OpenCV (libopencv-dev)"
            if ! dpkg -l | grep -q libopencv-dev; then
                log_warning "libopencv-dev may not be installed"
            fi
            ;;
    esac

    log_success "All resources prepared"
else
    log_info "Step 4: Skipping resource downloads (--skip-downloads)"
fi

# Step 5: Set environment variables
log_info "Step 5: Setting environment variables..."
export PADDLE_LIB="$SOURCE_DIR/$(ls -d *paddle_inference*/ 2>/dev/null | head -n1)"
export MODELS="$SOURCE_DIR/models"

if [ "$OPENCV_MODE" != "system" ]; then
    export OPENCV_DIR="$SOURCE_DIR/opencv-release"
    log_info "OPENCV_DIR=$OPENCV_DIR"
else
    unset OPENCV_DIR
    log_info "Using system OpenCV"
fi

log_info "PADDLE_LIB=$PADDLE_LIB"
log_info "MODELS=$MODELS"

# Verify paths
if [ ! -d "$PADDLE_LIB" ]; then
    log_error "Paddle Inference library not found at: $PADDLE_LIB"
    exit 1
fi

if [ ! -d "$MODELS" ]; then
    log_error "Models directory not found at: $MODELS"
    exit 1
fi

# Step 6: Build the project
log_info "Step 6: Building PaddleOCR-json..."
cd "$CPP_DIR"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ] && [ -d "build" ]; then
    log_info "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build

# Configure with CMake
log_info "Configuring with CMake..."
CMAKE_ARGS=(
    -S .
    -B build/
    -DPADDLE_LIB="$PADDLE_LIB"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
)

if [ -n "$OPENCV_DIR" ]; then
    CMAKE_ARGS+=(-DOPENCV_DIR="$OPENCV_DIR")
fi

if [ -n "$INSTALL_PREFIX" ]; then
    CMAKE_ARGS+=(-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX")
fi

cmake "${CMAKE_ARGS[@]}"

# Build
log_info "Compiling with $JOBS parallel jobs..."
cmake --build build/ --config "$BUILD_TYPE" -j "$JOBS"

log_success "Build completed successfully!"

# Verify binary
BINARY_PATH="$CPP_DIR/build/bin/PaddleOCR-json"
if [ -f "$BINARY_PATH" ]; then
    log_success "Binary created: $BINARY_PATH"
    log_info "Binary size: $(du -h "$BINARY_PATH" | cut -f1)"
else
    log_error "Binary not found at expected location: $BINARY_PATH"
    exit 1
fi

# Step 7: Optional installation
if [ "$DO_INSTALL" = true ]; then
    log_info "Step 7: Installing PaddleOCR-json..."

    if [ -n "$INSTALL_PREFIX" ]; then
        log_info "Installing to: $INSTALL_PREFIX"
        cmake --install build --prefix "$INSTALL_PREFIX"
    else
        log_info "Installing to system directory (requires sudo)..."
        sudo cmake --install build
    fi

    log_success "Installation completed!"
fi

# Print usage instructions
log_info ""
log_info "===================================="
log_success "Build process completed successfully!"
log_info "===================================="
log_info ""
log_info "Binary location: $BINARY_PATH"
log_info "Models location: $MODELS"
log_info ""
log_info "To run PaddleOCR-json:"
log_info "  cd $CPP_DIR"
log_info "  ./tools/linux_run.sh -models_path=\"$MODELS\" -config_path=\"$MODELS/config_chinese.txt\""
log_info ""
log_info "Or set up library path manually:"
log_info "  LD_LIBRARY_PATH=$CPP_DIR/build/bin $BINARY_PATH -models_path=\"$MODELS\" -config_path=\"$MODELS/config_chinese.txt\""
log_info ""

if [ "$DO_INSTALL" = false ]; then
    log_info "To install the binary system-wide, run:"
    log_info "  sudo cmake --install build"
    log_info ""
fi

log_info "For more information, see: $CPP_DIR/README-linux.md"
