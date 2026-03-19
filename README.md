# 🐍 UEFI Snake

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![EDK2](https://img.shields.io/badge/EDK2-II-blue.svg)](https://github.com/tianocore/edk2)
[![UEFI](https://img.shields.io/badge/UEFI-2.7+-green.svg)](https://uefi.org/)
[![Platform](https://img.shields.io/badge/platform-x64-lightgrey.svg)](https://en.wikipedia.org/wiki/X86-64)

> 一款运行在 **UEFI Shell** 环境下的经典贪吃蛇游戏，纯 C 语言编写，无需操作系统即可运行！

[English](./README_EN.md) | 简体中文

---

## ✨ 特性

- 🎮 **纯 UEFI 环境运行** - 直接在固件层执行，无需操作系统
- 🇨🇳 **全中文界面** - 完整的中文菜单、状态显示和操作提示
- 🎨 **硬件加速图形** - 基于 UEFI GOP 协议直接操作显卡帧缓冲区
- 🖥️ **双缓冲渲染** - 消除画面撕裂，流畅的视觉体验
- 🎯 **多关卡系统** - 随着分数增加自动升级，难度递增
- 🧱 **障碍物系统** - 高级关卡出现障碍物，增加挑战性
- ✨ **粒子特效** - 吃食物时的视觉特效
- ⌨️ **键盘控制** - 支持方向键和 WASD 两种控制方式
- ⏸️ **暂停功能** - 按 P 键随时暂停/继续游戏

---

## 📸 截图

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

## 🚀 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/yourusername/uefi-snake.git
cd uefi-snake
```

### 2. 环境准备

你需要安装以下工具：

- [EDK II](https://github.com/tianocore/edk2) - UEFI 开发环境
- [MinGW-w64](https://www.mingw-w64.org/) 或 Visual Studio 2019+
- [Python 3.10+](https://www.python.org/)
- [NASM](https://www.nasm.us/) 2.16.03+
- [QEMU](https://www.qemu.org/) (可选，用于模拟运行)

### 3. 编译

```bash
# 使用 Python 构建脚本
python build_snake.py

# 或在 Windows 上使用批处理
rebuild_vs.bat
```

编译成功后，将生成 `Build/SnakeApp/DEBUG_GCC/X64/SnakeApp.efi`。

### 4. 运行

#### 使用 QEMU（推荐）

```bash
run_qemu.bat
```

或在 UEFI Shell 中：

```shell
Shell> fs0:
FS0:\> SnakeApp.efi
```

#### 在真实硬件上运行

将 `SnakeApp.efi` 复制到 USB 设备的 EFI 分区，在开机时进入 UEFI Shell 运行。

---

## 🎮 操作指南

| 按键 | 功能 |
|------|------|
| `↑` `↓` `←` `→` | 方向控制 |
| `W` `A` `S` `D` | 备选方向控制 |
| `P` | 暂停/继续 |
| `ESC` | 退出游戏 |
| `SPACE` / `ENTER` | 游戏结束后重新开始 |

---

## 🏗️ 项目结构

```
uefi-snake/
├── SnakeApp/                 # 源代码目录
│   ├── Snake.c/h            # 游戏主逻辑
│   ├── Graphics.c/h         # GOP 图形封装
│   ├── Input.c/h            # 键盘输入处理
│   ├── Font.c/h             # 字体渲染（中英文）
│   ├── SnakeApp.dec         # 包声明文件
│   ├── SnakeApp.dsc         # 包描述文件
│   └── SnakeApp.inf         # 模块信息文件
├── build_snake.py           # Python 构建脚本
├── rebuild_vs.bat           # Windows 构建脚本
├── run_qemu.bat             # QEMU 启动脚本
├── extract_font.py          # 字体提取工具
├── 产品说明书.md            # 中文产品说明书
└── README.md                # 本文件
```

---

## 🔧 技术栈

| 组件 | 说明 |
|------|------|
| **固件接口** | UEFI GOP (Graphics Output Protocol) |
| **输入协议** | UEFI Simple Text Input Protocol |
| **构建系统** | EDK II (EFI Development Kit II) |
| **编译器** | MinGW-w64 GCC / Visual Studio 2019 |
| **目标架构** | x86_64 (X64) |
| **分辨率** | 1024 × 768 @ 32-bit BGRA |
| **网格大小** | 48 × 48 单元格 |

---

## 📝 中文字体

游戏使用 16×16 点阵字体显示中文，共支持 37 个常用汉字：

- 游戏界面：贪吃蛇、分数、最高、关卡、速度、长度
- 操作提示：操作、方向键、移动、暂停、退出、开始
- 状态显示：游戏结束、按空格、重新
- 版本信息：版本

字体数据从 Windows 宋体 (SimSun) 提取，使用 Python 脚本 `extract_font.py` 自动生成。

---

## ⚙️ 配置

编辑 `SnakeApp/Snake.h` 自定义游戏参数：

```c
#define SCREEN_WIDTH    1024    // 屏幕宽度
#define SCREEN_HEIGHT   768     // 屏幕高度
#define CELL_SIZE       16      // 格子大小
#define SNAKE_VERSION   6       // 版本号
```

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建你的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

---

## 📜 许可证

本项目基于 [MIT](LICENSE) 许可证开源。

---

## 🙏 致谢

- [TianoCore EDK2](https://github.com/tianocore/edk2) - UEFI 开发框架
- [QEMU](https://www.qemu.org/) - 虚拟化模拟器
- [OVMF](https://github.com/tianocore/tianocore.github.io/wiki/OVMF) - 开源 UEFI 固件

---

<p align="center">
  Made with ❤️ by UEFI enthusiasts
</p>
