# 🐍 UEFI Snake

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![EDK2](https://img.shields.io/badge/EDK2-II-blue.svg)](https://github.com/tianocore/edk2)
[![UEFI](https://img.shields.io/badge/UEFI-2.7+-green.svg)](https://uefi.org/)
[![Platform](https://img.shields.io/badge/platform-x64-lightgrey.svg)](https://en.wikipedia.org/wiki/X86-64)

> A classic Snake game that runs in the **UEFI Shell** environment, written in pure C, no operating system required!

English | [简体中文](./README.md)

---

## ✨ Features

- 🎮 **Pure UEFI Environment** - Runs directly at the firmware level, no OS needed
- 🇨🇳 **Full Chinese Interface** - Complete Chinese menus, status display, and operation hints
- 🎨 **Hardware-Accelerated Graphics** - Direct framebuffer manipulation via UEFI GOP protocol
- 🖥️ **Double Buffering** - Eliminates screen tearing for smooth visuals
- 🎯 **Multi-Level System** - Automatic level progression with increasing difficulty
- 🧱 **Obstacle System** - Obstacles appear in advanced levels for added challenge
- ✨ **Particle Effects** - Visual effects when eating food
- ⌨️ **Keyboard Controls** - Support for both arrow keys and WASD
- ⏸️ **Pause Function** - Press P to pause/resume anytime

---

## 📸 Screenshot

```
┌─────────────────────────────────────────────────────────────┬─────────────────┐
│  🟢                                                         │  贪吃蛇         │
│                                                             │  ───────────────│
│                           🍎                                │  分数     120   │
│                                                             │  最高     350   │
│                                                             │  关卡      3    │
│        ┌──────────────────────────┐                        │  速度      3    │
│        │      游戏结束            │                        │  长度     15    │
│        │                          │                        │                 │
│        │      分数  120           │                        │  操作           │
│        │                          │                        │  方向键 移动    │
│        │                          │                        │  P-暂停         │
│        │                          │                        │  ESC-退出       │
│        │  按空格重新开始          │                        │                 │
│        └──────────────────────────┘                        │  游戏结束       │
│                                                             │  重退出键       │
│                                                             │  开始           │
│                                                             │                 │
│                                                             │  版本 6         │
└─────────────────────────────────────────────────────────────┴─────────────────┘
```

---

## 🚀 Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/uefi-snake.git
cd uefi-snake
```

### 2. Prerequisites

You need to install the following tools:

- [EDK II](https://github.com/tianocore/edk2) - UEFI development environment
- [MinGW-w64](https://www.mingw-w64.org/) or Visual Studio 2019+
- [Python 3.10+](https://www.python.org/)
- [NASM](https://www.nasm.us/) 2.16.03+
- [QEMU](https://www.qemu.org/) (optional, for emulation)

### 3. Build

```bash
# Using Python build script
python build_snake.py

# Or on Windows using batch file
rebuild_vs.bat
```

After successful compilation, `Build/SnakeApp/DEBUG_GCC/X64/SnakeApp.efi` will be generated.

### 4. Run

#### Using QEMU (Recommended)

```bash
run_qemu.bat
```

Or in UEFI Shell:

```shell
Shell> fs0:
FS0:\> SnakeApp.efi
```

#### Run on Real Hardware

Copy `SnakeApp.efi` to the EFI partition of a USB device, then run it from UEFI Shell at boot.

---

## 🎮 Controls

| Key | Function |
|------|------|
| `↑` `↓` `←` `→` | Direction control |
| `W` `A` `S` `D` | Alternative direction control |
| `P` | Pause/Resume |
| `ESC` | Exit game |
| `SPACE` / `ENTER` | Restart after game over |

---

## 🏗️ Project Structure

```
uefi-snake/
├── SnakeApp/                 # Source code directory
│   ├── Snake.c/h            # Game main logic
│   ├── Graphics.c/h         # GOP graphics wrapper
│   ├── Input.c/h            # Keyboard input handling
│   ├── Font.c/h             # Font rendering (Chinese & English)
│   ├── SnakeApp.dec         # Package declaration file
│   ├── SnakeApp.dsc         # Package description file
│   └── SnakeApp.inf         # Module information file
├── build_snake.py           # Python build script
├── rebuild_vs.bat           # Windows build script
├── run_qemu.bat             # QEMU startup script
├── extract_font.py          # Font extraction tool
├── 产品说明书.md            # Chinese product manual
└── README.md                # This file
```

---

## 🔧 Tech Stack

| Component | Description |
|------|------|
| **Firmware Interface** | UEFI GOP (Graphics Output Protocol) |
| **Input Protocol** | UEFI Simple Text Input Protocol |
| **Build System** | EDK II (EFI Development Kit II) |
| **Compiler** | MinGW-w64 GCC / Visual Studio 2019 |
| **Target Architecture** | x86_64 (X64) |
| **Resolution** | 1024 × 768 @ 32-bit BGRA |
| **Grid Size** | 48 × 48 cells |

---

## 📝 Chinese Font Support

The game uses 16×16 bitmap fonts for Chinese characters, supporting 37 commonly used Chinese characters:

- **Game Interface**: 贪吃蛇 (Snake), 分数 (Score), 最高 (High Score), 关卡 (Level), 速度 (Speed), 长度 (Length)
- **Operation Hints**: 操作 (Controls), 方向键 (Arrow Keys), 移动 (Move), 暂停 (Pause), 退出 (Exit), 开始 (Start)
- **Status Display**: 游戏结束 (Game Over), 按空格 (Press Space), 重新 (Restart)
- **Version Info**: 版本 (Version)

Font data is extracted from Windows SimSun font using the Python script `extract_font.py`.

---

## ⚙️ Configuration

Edit `SnakeApp/Snake.h` to customize game parameters:

```c
#define SCREEN_WIDTH    1024    // Screen width
#define SCREEN_HEIGHT   768     // Screen height
#define CELL_SIZE       16      // Cell size
#define SNAKE_VERSION   6       // Version number
```

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit Issues and Pull Requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📜 License

This project is licensed under the [MIT](LICENSE) License.

---

## 🙏 Acknowledgments

- [TianoCore EDK2](https://github.com/tianocore/edk2) - UEFI development framework
- [QEMU](https://www.qemu.org/) - Virtualization emulator
- [OVMF](https://github.com/tianocore/tianocore.github.io/wiki/OVMF) - Open source UEFI firmware

---

<p align="center">
  Made with ❤️ by UEFI enthusiasts
</p>
