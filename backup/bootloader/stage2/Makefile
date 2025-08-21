BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: stage2

stage2: $(BUILD_DIR)/boot_s2.bin

$(BUILD_DIR)/boot_s2.bin:
	$(ASM) main.asm -f bin -o $(BUILD_DIR)/boot_s2.bin

clean:
	rm -f $(BUILD_DIR)/boot_s2.bin
