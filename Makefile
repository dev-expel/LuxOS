ASM=nasm
CC=gcc
CC16=wcc
ld16=wlink

SRC_DIR=src
TOOLS_DIR = tools
BUILD_DIR=build

.PHONY: all system kernel boot clean always tools_fat

all: system tools_fat

#
# Floppy Image
#
system: $(BUILD_DIR)/system.img 

$(BUILD_DIR)/system.img: boot kernel
	dd if=/dev/zero of=$(BUILD_DIR)/system.img bs=512 count=2880
	mkfs.fat -F 12 -n "NBOS" $(BUILD_DIR)/system.img
	dd if=$(BUILD_DIR)/boot_s1.bin of=$(BUILD_DIR)/system.img conv=notrunc
	mcopy -i $(BUILD_DIR)/system.img $(BUILD_DIR)/boot_s2.bin "::boot_s2.bin"
	mcopy -i $(BUILD_DIR)/system.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD_DIR)/system.img test.txt "::test.txt"

#
# Bootloader
#
boot: stage1 stage2

stage1: $(BUILD_DIR)/boot_s1.bin

$(BUILD_DIR)/boot_s1.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/boot_s2.bin

$(BUILD_DIR)/boot_s2.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always $(TOOLS_DIR)/fat/fat.c
	mkdir -p $(BUILD_DIR)/tools
	$(CC) -g -o $(BUILD_DIR)/tools/fat $(TOOLS_DIR)/fat/fat.c

#
# Always
#
always:
	mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	rm -rf $(BUILD_DIR)/*