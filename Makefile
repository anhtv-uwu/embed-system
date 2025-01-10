# Toolchain
CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Project name
TARGET = stm32f429

# Paths
SRC_DIR     = ./ub_lib ./pacman_lib ./cmsis_lib/source ./cmsis_boot ./stdio ./syscalls
INCLUDE_DIR = ./ub_lib ./pacman_lib ./cmsis_lib/include ./cmsis_boot ./stdio ./syscalls

# Source files
SRCS = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))

# Include paths
INCLUDES = $(foreach dir, $(INCLUDE_DIR), -I$(dir))

# Output directory
BUILD_DIR = ./build

# Flags
CFLAGS  = -mcpu=cortex-m4 -mthumb -g -O0 -ffunction-sections -fdata-sections
CFLAGS += -DUSE_STDPERIPH_DRIVER -DSTM32F429_439xx -DSTM32F4XX -D__ASSEMBLY__
LDFLAGS = -T cmsis_boot/startup/stm32f4xx_flash.ld -Wl,--gc-sections

# Output files
ELF = $(BUILD_DIR)/$(TARGET).elf
BIN = $(BUILD_DIR)/$(TARGET).bin
HEX = $(BUILD_DIR)/$(TARGET).hex

# Rules
all: $(BIN) $(HEX)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile and link
$(ELF): $(BUILD_DIR) $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(ELF) $(LDFLAGS)
	@echo "ELF file created: $(ELF)"

# Convert ELF to BIN
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	@echo "BIN file created: $(BIN)"

# Convert ELF to HEX
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $(ELF) $(HEX)
	@echo "HEX file created: $(HEX)"

# Display size information
size: $(ELF)
	$(SIZE) $(ELF)

# Clean build
clean:
	rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned."

.PHONY: all clean size
