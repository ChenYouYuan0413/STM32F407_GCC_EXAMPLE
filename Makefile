# STM32F407VGTx Makefile - ARM EABI GCC
# BUILD=debug (default) | BUILD=release
BUILD   ?= debug
PROJECT  = STM32F407_Template

# Toolchain
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc -x assembler-with-cpp
CP      = arm-none-eabi-objcopy
SZ      = arm-none-eabi-size
HEX     = $(CP) -O ihex
BIN     = $(CP) -O binary -S

# CPU
CPU       = -mcpu=cortex-m4
FPU       = -mfpu=fpv4-sp-d16
FLOAT-ABI = -mfloat-abi=hard
MCU       = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# Directories
ROOT    = .
CORE    = $(ROOT)/CORE
FWLIB   = $(ROOT)/FWLIB
USER    = $(ROOT)/USER/Template/src

# Include paths
INC = \
  -I$(CORE) \
  -I$(FWLIB)/inc \
  -I$(USER)

# Defines
DEFS = \
  -DSTM32F40_41xxx \
  -DUSE_STDPERIPH_DRIVER \
  -DHSE_VALUE=8000000U

# Source files
ASM_SRC = \
  $(CORE)/startup_stm32f407xx_gcc.s

C_SRC = \
  $(USER)/main.c \
  $(USER)/stm32f4xx_it.c \
  $(USER)/system_stm32f4xx.c \
  $(USER)/syscalls.c \
  $(USER)/delay.c \
  $(USER)/led.c \
  $(USER)/uart.c \
  $(FWLIB)/src/misc.c \
  $(FWLIB)/src/stm32f4xx_adc.c \
  $(FWLIB)/src/stm32f4xx_can.c \
  $(FWLIB)/src/stm32f4xx_crc.c \
  $(FWLIB)/src/stm32f4xx_cryp.c \
  $(FWLIB)/src/stm32f4xx_cryp_aes.c \
  $(FWLIB)/src/stm32f4xx_cryp_des.c \
  $(FWLIB)/src/stm32f4xx_cryp_tdes.c \
  $(FWLIB)/src/stm32f4xx_dac.c \
  $(FWLIB)/src/stm32f4xx_dbgmcu.c \
  $(FWLIB)/src/stm32f4xx_dcmi.c \
  $(FWLIB)/src/stm32f4xx_dma.c \
  $(FWLIB)/src/stm32f4xx_exti.c \
  $(FWLIB)/src/stm32f4xx_flash.c \
  $(FWLIB)/src/stm32f4xx_fsmc.c \
  $(FWLIB)/src/stm32f4xx_gpio.c \
  $(FWLIB)/src/stm32f4xx_hash.c \
  $(FWLIB)/src/stm32f4xx_hash_md5.c \
  $(FWLIB)/src/stm32f4xx_hash_sha1.c \
  $(FWLIB)/src/stm32f4xx_i2c.c \
  $(FWLIB)/src/stm32f4xx_iwdg.c \
  $(FWLIB)/src/stm32f4xx_pwr.c \
  $(FWLIB)/src/stm32f4xx_rcc.c \
  $(FWLIB)/src/stm32f4xx_rng.c \
  $(FWLIB)/src/stm32f4xx_rtc.c \
  $(FWLIB)/src/stm32f4xx_sdio.c \
  $(FWLIB)/src/stm32f4xx_spi.c \
  $(FWLIB)/src/stm32f4xx_syscfg.c \
  $(FWLIB)/src/stm32f4xx_tim.c \
  $(FWLIB)/src/stm32f4xx_usart.c \
  $(FWLIB)/src/stm32f4xx_wwdg.c

# Object files
OBJS       = $(ASM_SRC:.s=.o) $(C_SRC:.c=.o)
BUILD_DIR  = build/$(BUILD)
OBJS_BUILD = $(addprefix $(BUILD_DIR)/, $(notdir $(OBJS)))

# Linker
LDSCRIPT = $(ROOT)/STM32F407VGTX_FLASH.ld

# Build-type flags
ifeq ($(BUILD),debug)
  OPT_FLAGS  = -Og -g3
  DBG_DEFS   = -DDEBUG
else
  OPT_FLAGS  = -Os -g0
  DBG_DEFS   = -DNDEBUG
endif

CFLAGS  = $(MCU) $(DEFS) $(DBG_DEFS) $(INC) \
  -Wall -Wextra -Wshadow \
  $(OPT_FLAGS) \
  -ffunction-sections -fdata-sections \
  -fno-common \
  -ffreestanding \
  -std=gnu11

ASFLAGS = $(MCU) $(DEFS) $(DBG_DEFS) $(INC) \
  -Wall -g3 \
  -Wa,--gdwarf-2

LDFLAGS = $(MCU) -T$(LDSCRIPT) \
  -nostdlib \
  -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map,--cref \
  -Wl,--gc-sections \
  -lgcc

# Default target
all: $(BUILD_DIR)/$(PROJECT).elf
	@echo ""
	@echo "=== Build: $(BUILD) ==="
	$(SZ) $<
	@echo "Output: $<"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build .elf
$(BUILD_DIR)/$(PROJECT).elf: $(BUILD_DIR) $(OBJS_BUILD)
	$(CC) $(LDFLAGS) $(OBJS_BUILD) -o $@

# C source files
$(BUILD_DIR)/%.o: $(USER)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(FWLIB)/src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(CORE)/%.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

# Output formats
hex: $(BUILD_DIR)/$(PROJECT).hex
bin: $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

# Phony targets
.PHONY: all clean debug release flash info

debug: BUILD=debug
debug: all

release: BUILD=release
release: all

clean:
	rm -rf build

# OpenOCD flash
flash: all $(BUILD_DIR)/$(PROJECT).hex
	openocd -f openocd.cfg -c "program $(BUILD_DIR)/$(PROJECT).elf verify reset exit"

# OpenOCD debug server
debug-server:
	openocd -f openocd.cfg

info:
	@echo "BUILD    : $(BUILD)"
	@echo "CC       : $(CC)"
	@echo "MCU      : $(MCU)"
	@echo "DEFS     : $(DEFS)"
	@echo "OPT      : $(OPT_FLAGS)"
	@echo "INC      : $(INC)"
	@echo "OBJS     : $(OBJS_BUILD)"
