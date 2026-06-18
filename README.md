# STM32F407 GCC 示例项目

> 基于 [STM32F407 GCC Build Framework](https://github.com/ChenYouYuan0413/STM32F407_GCC_Build_Framework) 的示例工程。
> 原 Keil MDK 标准外设库模板，现已迁移为 ARM GCC + OpenOCD + VSCode Cortex-Debug + LinkScope 开发流程。

## 快速开始

### 1. 克隆框架

```bash
git clone https://github.com/ChenYouYuan0413/STM32F407_GCC_Build_Framework.git
```

### 2. 获取本示例项目

`Makefile` 指向框架路径，默认为 `../stm32f407-gcc-framework/`。如果框架目录名不同，修改 `Makefile` 中的 include 路径即可。

### 3. 编译 & 调试

```bash
make BUILD=debug all     # 编译
make flash               # 烧录
```

或在 VSCode 中 F5 一键调试。

---

## 环境

| 工具 | 版本 | 安装（macOS） |
|------|------|------|
| `arm-none-eabi-gcc` | 16.1.0 | `brew install arm-none-eabi-gcc` |
| `arm-none-eabi-gdb` | 17.2 | 随 GCC 安装 |
| `OpenOCD` | 0.12.0 | `brew install openocd` |
| VSCode Cortex-Debug | 1.12.1 | 扩展 `marus25.cortex-debug` |
| LinkScope | 1.3.0-mac | [ChenYouYuan0413/LinkScope](https://github.com/ChenYouYuan0413/LinkScope/tree/mac)（`mac` 分支），需自行编译（Qt5） |

> Windows / Linux 环境配置见 [框架 README](https://github.com/ChenYouYuan0413/STM32F407_GCC_Build_Framework#环境)

## 项目结构

```
stm32/
├── stm32f407-gcc-framework/   # ⭐ 共享框架（所有 F407 工程复用）
│   └── (从 https://github.com/ChenYouYuan0413/STM32F407_GCC_Build_Framework 克隆)
│
├── STM32F407/                 # ← 本示例项目
│   ├── USER/Template/src/     # 应用代码（main.c, delay.c, led.c, uart.c...）
│   ├── Makefile               # 4 行：项目名 + USER 路径 + HSE 值 + include 框架
│   ├── openocd_dap.cfg        # DAPLink 本地配置
│   ├── openocd_stlink.cfg     # ST-Link 本地配置
│   └── .vscode/               # launch.json, tasks.json, c_cpp_properties.json
│
└── LinkScope/                 # 开源高速变量监视器（mac 分支）
```
```

## 快速开始

### 编译

```bash
make BUILD=debug all     # 调试版（-O0 -g3，默认，不做优化防止变量被优化掉）
make BUILD=release all   # 发布版（-Os -g0）
make clean               # 清理
```

### 调试（VSCode）

1. 连接 DAPLink：SWDIO → PA13, SWCLK → PA14, GND → GND
2. 确保 BOOT0 和 BOOT1 接地
3. F5 启动调试，自动编译 → 烧录 → 停在 `main()`

### 烧录

```bash
make flash
```

### 新工程接入

1. 克隆框架到本地
2. 创建项目目录，放入 `USER/` 应用代码
3. 写一个 4 行 Makefile：

```makefile
PROJECT   = MyProject
USER      = ./USER/Template/src
HSE_VALUE = 8000000             # 可选，默认 8MHz
include   ../stm32f407-gcc-framework/Makefile.common
```

4. 复制 `.vscode/launch.json` 模板（可选，F5 调试用）

> 完整说明见 [框架仓库](https://github.com/ChenYouYuan0413/STM32F407_GCC_Build_Framework)

## 调试器配置

| 文件 | 调试器 |
|------|--------|
| `openocd_dap.cfg` | DAPLink / CMSIS-DAP |
| `openocd_stlink.cfg` | ST-Link/V2 |

两个配置文件都包含 `$_TARGETNAME configure -gdb-max-connections 4`。

**为什么需要 4 个 GDB 连接**：

| 连接数 | 占用者 | 用途 |
|--------|--------|------|
| 1 | Cortex-Debug 主 GDB | 断点、单步、烧录、变量查看 |
| 1 | Cortex-Debug Live Watch | 后台轮询 Watch 面板变量值 |
| 1 | LinkScope | 高速批量采样 `x/Nwx` |
| 1 | 预留 | 第二个 LinkScope 连接或手动 GDB 调试 |

OpenOCD 默认只允许 1 个 GDB 连接，Cortex-Debug 的 helper 脚本会自动 +1（变成 2）。但 Cortex-Debug 自己的 Live Watch 就占了第 2 个，所以必须手动提到 4 以上才能让 LinkScope 接入。

## LinkScope 高速变量监控

### 获取 & 编译

```bash
git clone https://github.com/ChenYouYuan0413/LinkScope.git
cd LinkScope
git checkout mac          # ⚠️ macOS 功能在 mac 分支，master 是原始 Windows 版
brew install qt@5
/opt/homebrew/Cellar/qt@5/5.15.19/bin/qmake LinkScope.pro
make -j8
open LinkScope.app
```

> **分支说明**：
> - `master` — 原版（Windows），不支持 Cortex Watch 同步、高速模式、停靠窗口等
> - `mac` — macOS 增强版，包含所有本文描述的功能
> - Windows 增强版待开发，可参考 `mac` 分支的修改移植

### 使用

1. VSCode F5 启动调试
2. 打开 LinkScope
3. **菜单栏 → 高级设置**：设 GDB 端口 = `50000`，选择接口 CMSIS-DAP、目标 stm32f4x、符号文件 `.elf`、项目目录
4. ✅ 勾选「外部 OpenOCD」→ 点「连接」
5. ✅ 勾选「同步 Cortex Watch」→ VSCode Watch 变量自动同步（运行时新增也会自动刷新）
6. ✅ 默认启用「高速模式」→ 批量 `x/Nwx` 读内存，~3kHz 采样率
7. 点击「暂停」按钮可冻结曲线和分析数值（发送 GDB `monitor halt`）

### 主要功能

- **窗口停靠**：图形/日志/选择器可拖拽合并、Tab 分页、拖出浮动，布局自动保存
- **Cortex Watch 同步**：每 2 秒自动从 VSCode Watch 面板同步变量名，新增变量自动解析地址开始采样
- **结构体自动展开**：`p` → `p.x`、`p.y`、`p.color.r`（支持多级嵌套，GDB `ptype` 解析）
- **高速批量读**：一次 `x/Nwx` 读完所有变量所在内存区域，本地按偏移拆值
- **类型感知**：`int8_t`/`int16_t` → 1/2 字节读 + 符号扩展；`float` → IEEE 754 位模式转换
- **实时 FPS**：日志窗口每 500 帧输出实际采样率
- **暂停采样**：操作栏「暂停」按钮发送 GDB halt/resume，冻结曲线分析
- **UI 精简**：调试配置全部移到「高级设置」，主窗口只留变量表和操作栏

### macOS 适配注意事项

LinkScope 源码为 Windows 开发，macOS 编译需要以下修改（已全部在 `mac` 分支实现）：

| 问题 | 修复 |
|------|------|
| `#include <windows.h>` | `#ifdef Q_OS_WIN32` 宏保护 |
| `setNativeArguments()` | Windows 专用 API，macOS 用 `setArguments()` |
| GDB 输出 `\r\n` → `\n` | 所有正则 `\r\n` → `\r?\n` |
| 临时文件路径 | App bundle 无写权限，改用 `QDir::tempPath()` |
| 配置文件路径 | `conf.ini` 相对路径 → `~/.linkscope/conf.ini` |
| `taskkill` 杀进程 | macOS 用 `QProcess::terminate()` |
| `1u << 32` 未定义行为 | ARM64 shift mask 导致 4 字节变量归零，特判 `size >= 4` |
| GDB 管道分批到达 | `trimmed().endsWith("(gdb)")` + 逐行解析 |
| `QDialog` → `QDockWidget` | 三个子窗口改为 QWidget + 嵌入 QDockWidget |
| GDB 启动参数 | 添加 `-q -nx -ex "set confirm off"` |
| 布局持久化 | `saveState()`/`restoreState()` 保存到绝对路径 |

## 常见问题

### LED 不亮
- 检查 BOOT0/BOOT1 是否接地
- 检查 LED 引脚是否匹配（默认 D1 = PF9）

### 调试器连不上
```bash
# 手动测试 OpenOCD
openocd -f openocd_dap.cfg
# 看到 "Cortex-M4 r0p1 processor detected" 即正常
```

### `Mdelay_Lib` 卡死
`static int mdelay_time` 必须声明为 `volatile`，否则编译器会把 `while(mdelay_time)` 优化成死循环（比较寄存器而非内存值）。

### 变量被优化掉（optimized out / not available）
- Debug 构建用 `-O0`（不做优化），变量即使未使用也保留
- 但 `--gc-sections` 链接器仍会删除未引用的符号：加一行 `var++` 即可
- 或声明为 `volatile`

### LinkScope 变量值为 0
- 确认「高速模式」已勾选
- 确认变量是 `static` 或全局（局部变量在栈上，地址不固定）
- 查看日志窗口确认地址解析（`解析 xxx → 0xADDR`）
- 4 字节变量全为 0：检查是否误用了旧版 LinkScope（ARM64 `1u << 32` UB 已修复）

### LinkScope 新增变量不刷新
- VSCode Watch 加变量后等 2 秒（Cortex 同步周期），无需断连重连
- 日志窗口查看 `解析 xxx → 0xADDR` 确认地址已解析

### 采样率低
- 确认高速模式已勾选（解除 `sampleFreq` 的 100Hz 限制）
- 日志窗口每 500 帧输出一次实际采样率
- 首次连接后采样率从 ~3kHz 开始属于正常（地址解析是在连接时一次性完成的）

## VSCode 快捷键

| 操作 | 快捷键 |
|------|--------|
| 编译 | `Cmd+Shift+B` |
| 调试 | `F5` |
| 继续运行 | `F5` |
| 单步跳过 | `F10` |
| 单步进入 | `F11` |
| 烧录 | `Cmd+Shift+P` → Tasks: Run Task → Flash |

## 修改 HSE 晶振

如果目标板晶振不是 8MHz，修改两处：

**1.** `Makefile`（框架 `Makefile.common` 中子项目覆盖）：
```makefile
HSE_VALUE = 25000000
include ../stm32f407-gcc-framework/Makefile.common
```

**2.** `.vscode/c_cpp_properties.json`（IntelliSense）：
```json
"defines": ["HSE_VALUE=25000000U", ...]
```
