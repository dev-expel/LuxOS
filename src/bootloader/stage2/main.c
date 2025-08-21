#include "stdint.h"
#include "stdio.h"
#include "disk.h"

void _cdecl cstart_(uint16_t bootDrive) {
    print("Loaded: Bootloader Stage 2\r\n");

    for(;;);
}