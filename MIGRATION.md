# STM32F407 项目迁移：Keil MDK → ARM GCC + OpenOCD + VSCode Cortex-Debug

## 项目背景

- **原始项目**：Keil MDK-ARM 工程（STM32F4xx 标准外设库 V1.8.0）
- **目标芯片**：STM32F407VGTx（1024KB Flash / 128KB SRAM）
- **HSE 晶振**：8MHz，PLL 倍频至 168MHz
- **迁移日期**：2026-06-08

## 环境

| 工具 | 版本 | 安装方式 |
|------|------|----------|
| `arm-none-eabi-gcc` | 16.1.0 | `brew install arm-none-eabi-gcc` |
| `arm-none-eabi-gdb` | 17.2 | 随 gcc 安装 |
| `OpenOCD` | 0.12.0 | `brew install openocd` |
| `make` | 3.81 | macOS 自带 |
| VSCode Cortex-Debug | 1.12.1 | VSCode 扩展 `marus25.cortex-debug` |

> **注意**：Homebrew 的 `arm-none-eabi-gcc` 不含 newlib（C 标准库）。本项目使用 `-ffreestanding` 模式，依赖 GCC 内置头文件编译，无需额外安装 C 库。

## 新增文件总览

```
STM32F407/
├── Makefile                          # 构建系统（debug/release）
├── openocd.cfg                       # OpenOCD 配置（DAPLink/CMSIS-DAP）
├── STM32F407VGTX_FLASH.ld            # 链接脚本
├── STM32F407.svd                     # SVD 外设描述文件（35KB）
├── .gitignore
├── .vscode/
│   ├── launch.json                   # Cortex-Debug 调试配置
│   ├── tasks.json                    # 构建任务
│   ├── settings.json                 # 工作区设置
│   └── c_cpp_properties.json        # IntelliSense 配置
├── CORE/
│   └── startup_stm32f407xx_gcc.s     # GCC 格式启动文件（新增）
└── USER/Template/src/
    └── syscalls.c                    # 最小 freestanding 运行时桩（新增）
```

## 关键步骤

### 1. 启动文件转换

[`CORE/startup_stm32f407xx_gcc.s`](CORE/startup_stm32f407xx_gcc.s) 从原 Keil 语法 `startup_stm32f40xx.s` 转换而来。

主要语法对照：

| Keil ARMASM | GNU AS |
|-------------|--------|
| `AREA \|.text\|, CODE` | `.section .text` |
| `EXPORT symbol` | `.global symbol` |
| `IMPORT symbol` | 自动链接（或 `.extern`） |
| `DCD value` | `.word value` |
| `SPACE size` | `.space size` |
| `PROC … ENDP` | `.type func, %function … .size` |
| `EQU` | `.equ` |
| `PRESERVE8; THUMB` | `.syntax unified; .thumb` |
| `IF :DEF:__MICROLIB … ENDIF` | 移除（链接脚本管理堆栈） |

关键修改点：
- 中断向量表放在 `.isr_vector` 段，链接脚本将其放在 Flash 起始处（0x08000000）
- 默认中断处理函数使用 `.weak` + `.thumb_set` 实现可覆盖弱引用
- 启动后在 `Reset_Handler` 中完成 `.data` 复制、`.bss` 清零，调用 `SystemInit()` 后跳转 `main()`
- 去掉 `__libc_init_array`（freestanding 模式无静态构造函数）

### 2. 链接脚本

[`STM32F407VGTX_FLASH.ld`](STM32F407VGTX_FLASH.ld)：

- **FLASH**：0x08000000，1024KB（存放代码、只读数据、初始化值）
- **RAM**：0x20000000，128KB（存放 `.data`、`.bss`、堆栈）
- **堆大小**：`_Min_Heap_Size = 0x200`（512字节）
- **栈大小**：`_Min_Stack_Size = 0x400`（1KB）
- 导出符号：`_sidata`、`_sdata`、`_edata`、`_sbss`、`_ebss`、`_estack`

### 3. 构建系统

[`Makefile`](Makefile) 核心设计：

```makefile
# 调试构建（默认）
make BUILD=debug all       # -Og -g3

# 发布构建
make BUILD=release all     # -Os -g0

# 其他
make clean                 # 删除 build/
make flash                 # 编译 + 烧录
make debug-server          # 仅启动 OpenOCD
```

编译器标志：

```makefile
MCU  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
DEFS = -DSTM32F40_41xxx -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000U
             # ^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^
             # 芯片型号选择       启用标准外设库驱动        外部晶振 8MHz
```

链接器使用 `-nostdlib` + `-lgcc`（仅依赖 libgcc，不依赖 newlib）。

### 4. 运行时桩

[`USER/Template/src/syscalls.c`](USER/Template/src/syscalls.c) 提供：

- `_sbrk()` — 简单的堆分配（支持 `malloc`，从 `_end` 向上增长，遇栈顶停止）
- `errno` — 全局错误码

**无需 `printf` / `scanf`** — STM32 标准外设库本身不依赖标准 I/O。

### 5. 调试器配置

[`openocd.cfg`](openocd.cfg) 适配 DAPLink（CMSIS-DAP + SWD）：

```tcl
source [find interface/cmsis-dap.cfg]
transport select swd
source [find target/stm32f4x.cfg]
adapter speed 5000
```

> 如用 ST-Link，改 `interface/cmsis-dap.cfg` 为 `interface/stlink.cfg`，`transport select hla_swd`。

### 6. VSCode 集成

#### tasks.json

- **Build Debug**（`Cmd+Shift+B` 默认）：`make BUILD=debug all`
- **Build Release**：`make BUILD=release all`
- **Clean**：`make clean`
- **Flash**：`make flash`（先编译再烧录）

#### launch.json

- **STM32F407 Debug (DAPLink)**（`F5` 启动）：自动编译 → OpenOCD → 烧录 → 停在 `main()`
- **STM32F407 Attach (DAPLink)**：附加到运行中的目标，不打断

Live Watch 配置：

```json
"liveWatch": {
    "enabled": true,
    "refreshRate": 200,
    "sampleInterval": 50
}
```

#### settings.json

```json
"cortex-debug.armToolchainPath": "/opt/homebrew/bin",
"cortex-debug.openocdPath": "/opt/homebrew/bin/openocd",
"cortex-debug.armToolchainPrefix": "arm-none-eabi"
```

#### c_cpp_properties.json

为 IntelliSense 提供正确的 Include 路径和宏定义，消除 VSCode 中的红色波浪线。

### 7. SVD 文件

[`STM32F407.svd`](STM32F407.svd) 从 [cmsis-svd](https://github.com/cmsis-svd/cmsis-svd-data) 下载，调试时自动加载，可在 **CORTEX PERIPHERALS** 面板查看所有外设寄存器。

## DAPLink 接线

连接 DAPLink 到 STM32F407 目标板（至少 4 根线）：

| DAPLink 引脚 | STM32F407 引脚 | 说明 |
|-------------|---------------|------|
| SWDIO | PA13 | 数据线 |
| SWCLK | PA14 | 时钟线 |
| GND | GND | 共地 |
| 3.3V | VDD | 参考电平（由目标板供电） |

> DAPLink 通常不对外供电，STM32F407 目标板需要独立供电。

## 调试操作速查

| 操作 | 方式 |
|------|------|
| 编译 | `Cmd+Shift+B` |
| 开始调试 | `F5` |
| 单步执行 | `F10`（Step Over）、`F11`（Step Into） |
| 添加 Live Watch | 在 WATCH 面板点 `+`，输入全局变量名（如 `SystemCoreClock`） |
| 查看外设寄存器 | 调试启动后，左侧 **CORTEX PERIPHERALS** 面板自动出现 |
| 烧录（不调试） | `Cmd+Shift+P` → `Tasks: Run Task` → `Flash` |
| 终端编译 | `make -j8 BUILD=debug all` |
| 终端烧录 | `make flash` |

## 如需修改 HSE 晶振

如果目标板的晶振不是 8MHz（如 25MHz），修改 `Makefile`：

```makefile
DEFS = \
  -DSTM32F40_41xxx \
  -DUSE_STDPERIPH_DRIVER \
  -DHSE_VALUE=25000000U       # ← 改此处
```

同步修改 [`.vscode/c_cpp_properties.json`](.vscode/c_cpp_properties.json) 中的 `HSE_VALUE` 定义以保持 IntelliSense 一致。
