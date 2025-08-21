BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/boot_s1.bin

$(BUILD_DIR)/boot_s1.bin:
	$(ASM) boot.asm -f bin -o $(BUILD_DIR)/boot_s1.bin

clean:
	rm -f $(BUILD_DIR)/boot_s1.bin
