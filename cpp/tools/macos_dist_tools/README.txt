Usage Manual (macOS x64)

Thanks for downloading & using PaddleOCR-json engine (https://github.com/hiroi-sora/PaddleOCR-json).

# Requirements

- macOS 10.15 (Catalina) or later
- Intel Mac: Runs natively
- Apple Silicon Mac (M1/M2/M3): Runs via Rosetta 2 emulation

# Running the Engine

You can use "run.sh" script to run the engine:

    ./run.sh

Or run with custom arguments:

    ./run.sh --image_path=test.jpg

# HTTP Server Mode

To start the HTTP server:

    ./run.sh --server --server_port=8080

# Apple Silicon (M1/M2/M3) Notes

This is an x86_64 binary that runs on Apple Silicon Macs via Rosetta 2 emulation.
Rosetta 2 is automatically installed when you first run an x86_64 app on Apple Silicon.

If you see a message about installing Rosetta, click "Install" to proceed.

Performance on Apple Silicon is good thanks to Rosetta 2's efficient translation.

# Troubleshooting

## Library not loaded error

If you see an error like:

    dyld: Library not loaded: @rpath/libpaddle_inference.dylib

Make sure the lib/ directory is in the same location as the run.sh script,
and contains all required .dylib files.

## Code signing / Gatekeeper issues

On first run, macOS may block the executable. You can allow it by:

1. Go to System Settings > Privacy & Security
2. Find the blocked app and click "Open Anyway"

Or run this command to remove the quarantine attribute:

    xattr -dr com.apple.quarantine .

## Missing OpenMP library

If you see an error about libomp.dylib, install it via Homebrew:

    # For Intel Mac:
    brew install libomp

    # For Apple Silicon Mac (install Intel version via Rosetta):
    arch -x86_64 /usr/local/bin/brew install libomp

---

使用手册 (macOS x64)

感谢下载使用PaddleOCR-json引擎。

# 运行要求

- macOS 10.15 (Catalina) 或更新版本
- Intel Mac：原生运行
- Apple Silicon Mac (M1/M2/M3)：通过 Rosetta 2 模拟运行

# 运行引擎

使用 "run.sh" 脚本运行引擎：

    ./run.sh

或添加自定义参数运行：

    ./run.sh --image_path=test.jpg

# HTTP 服务器模式

启动 HTTP 服务器：

    ./run.sh --server --server_port=8080

# Apple Silicon (M1/M2/M3) 说明

这是一个 x86_64 二进制文件，在 Apple Silicon Mac 上通过 Rosetta 2 模拟运行。
首次运行 x86_64 应用时，Rosetta 2 会自动安装。

如果看到安装 Rosetta 的提示，请点击"安装"继续。

得益于 Rosetta 2 高效的转译，在 Apple Silicon 上的性能表现良好。

# 常见问题

## 库加载错误

如果出现类似以下错误：

    dyld: Library not loaded: @rpath/libpaddle_inference.dylib

请确保 lib/ 目录与 run.sh 脚本在同一位置，并包含所有必需的 .dylib 文件。

## 代码签名 / Gatekeeper 问题

首次运行时，macOS 可能会阻止执行程序。您可以通过以下方式允许运行：

1. 前往 系统设置 > 隐私与安全性
2. 找到被阻止的应用程序并点击"仍要打开"

或运行以下命令移除隔离属性：

    xattr -dr com.apple.quarantine .

## 缺少 OpenMP 库

如果出现关于 libomp.dylib 的错误，请通过 Homebrew 安装：

    # Intel Mac:
    brew install libomp

    # Apple Silicon Mac（安装 Intel 版本）:
    arch -x86_64 /usr/local/bin/brew install libomp
