# GitHub Actions Build & Release Workflow

This directory contains GitHub Actions workflows for automatically building PaddleOCR-json binaries for Linux and Windows.

## Workflow: Build and Release

**File:** `build-release.yml`

### Trigger Methods

The workflow can be triggered in two ways:

#### 1. Automatic Release (Tag Push)

Push a version tag to automatically build and create a release:

```bash
git tag v1.4.2
git push origin v1.4.2
```

This will:
- Build binaries for Linux (Ubuntu 20.04) and Windows (Server 2019)
- Create a GitHub Release with the tag
- Upload compiled binaries to the release

#### 2. Manual Dispatch

Manually trigger the workflow from GitHub Actions tab:

1. Go to **Actions** → **Build and Release**
2. Click **Run workflow**
3. (Optional) Enter a release tag (e.g., `v1.4.2`)
4. Click **Run workflow**

If you don't provide a release tag, binaries will be built but no release will be created.

## Build Outputs

### Linux Binary
- **File:** `PaddleOCR-json-linux-x64.tar.gz`
- **Contents:**
  - `PaddleOCR-json` - Main executable
  - `*.so*` - Shared libraries (PaddlePaddle, OpenCV, etc.)
- **Platform:** Linux x86_64 (Ubuntu 18.04+)

### Windows Binary
- **File:** `PaddleOCR-json-windows-x64.zip`
- **Contents:**
  - `PaddleOCR-json.exe` - Main executable
  - `*.dll` - Dynamic libraries (PaddlePaddle, OpenCV, etc.)
- **Platform:** Windows 10+ (x64)

## Caching Strategy

The workflow uses GitHub Actions cache to speed up builds:

- **PaddlePaddle Inference Library** - Cached by version
- **OpenCV** - Cached by version
- **OCR Models** - Cached permanently (rarely changes)

Caches are shared across workflow runs and significantly reduce build times.

## Dependencies

### Build Dependencies

**Linux:**
- CMake 3.14+
- GCC 8.2+
- wget, unzip
- System libraries: libgomp1, libglib2.0-0, libsm6, libxext6, libxrender-dev

**Windows:**
- Visual Studio 2022
- CMake 3.14+
- MSBuild

### Runtime Dependencies

**Linux:**
- Downloaded automatically: PaddlePaddle 3.0.0-beta1, OpenCV 4.5.4
- System libraries for dynamic linking

**Windows:**
- Downloaded automatically: PaddlePaddle 3.0.0-beta1, OpenCV 4.5.4
- Visual C++ Redistributable 2022 (required on user machines)

## Customization

### Change PaddlePaddle or OpenCV Version

Edit the environment variables in `build-release.yml`:

```yaml
env:
  PADDLE_VERSION: '3.0.0-beta1'
  OPENCV_VERSION: '4.5.4'
```

### Add Additional Models

Modify the "Download OCR Models" steps to include more models:

```yaml
- name: Download OCR Models
  run: |
    wget https://paddleocr.bj.bcebos.com/PP-OCRv4/english/en_PP-OCRv4_rec_infer.tar
    tar -xf en_PP-OCRv4_rec_infer.tar
```

### Change Build Configuration

Edit CMake flags in the "Build PaddleOCR-json" step:

```yaml
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_GPU=ON \  # Enable GPU support
  ...
```

## Troubleshooting

### Build Fails on Linux

**Problem:** Missing dependencies

**Solution:** Check the "Install dependencies" step includes all required packages

### Build Fails on Windows

**Problem:** Visual Studio version mismatch

**Solution:** Update the runner to match your MSVC version:
```yaml
runs-on: windows-2022  # For VS 2022
```

### Cache Issues

**Problem:** Stale or corrupted cache

**Solution:** Clear GitHub Actions cache:
1. Go to **Settings** → **Actions** → **Caches**
2. Delete specific caches
3. Re-run the workflow

### Release Not Created

**Problem:** Workflow runs but no release appears

**Solution:** Check:
- Tag was pushed correctly
- `GITHUB_TOKEN` has permission to create releases
- Repository settings allow Actions to create releases

## Local Testing

You can test the build process locally before pushing:

**Linux:**
```bash
cd cpp
mkdir -p .source build
# Follow the workflow steps manually
```

**Windows:**
```powershell
cd cpp
mkdir .source, build
# Follow the workflow steps manually
```

## Maintenance

### Update Runner OS

The workflow uses:
- `ubuntu-20.04` for Linux builds
- `windows-2022` for Windows builds

Update these in the `runs-on` fields when newer stable versions are available.

### Update Actions

Keep GitHub Actions up to date:
- `actions/checkout@v4`
- `actions/cache@v3`
- `actions/upload-artifact@v4`
- `actions/download-artifact@v4`
- `softprops/action-gh-release@v1`

Check for newer versions periodically.

## Support

For issues with the workflow:
1. Check the Actions tab for build logs
2. Review error messages in failed steps
3. Open an issue with the error details

For PaddleOCR-json build issues:
- See main project README
- Check CMakeLists.txt for build requirements
