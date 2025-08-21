#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// TypeDefs
typedef uint8_t bool;
#define true 1
#define false 0

// error codes
#define INVALID_ARG_COUNT -1
#define OPEN_DISK_FAILED -2
#define READ_BOOTSECTOR_FAILED -3
#define READ_FAT_FAILED -4
#define READ_ROOT_DIR_FAILED -5
#define FIND_FILE_FAILED -6
#define READ_FILE_FAILED -7

typedef struct {
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemID[8];
} __attribute__((packed)) BootSector;

// Functions
BootSector g_BootSector;
uint8_t* g_Fat = NULL;
DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd;

bool readBootSector(FILE* disk)
{
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut) {
    bool is_ok = true;
    is_ok = is_ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0) \
            && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
    return is_ok;
}

bool readFat(FILE* disk) {
    g_Fat = (uint8_t*) malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
}

bool readRootDirectory(FILE* disk) {
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if (size % g_BootSector.BytesPerSector > 0) sectors ++;

    g_RootDirectoryEnd = lba + sectors;
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_BootSector.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}

DirectoryEntry* findFile(const char* name) {
    for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++) {
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0) return &g_RootDirectory[i];
    }

    return NULL;
}

bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer) {
    bool is_ok = true; 
    uint16_t currentCluster = fileEntry->FirstClusterLow;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        is_ok = is_ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0) currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        else currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;
    } while (is_ok && currentCluster < 0x0FF8);

    return is_ok;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return INVALID_ARG_COUNT;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot read from disk %s\n", argv[1]);
        return OPEN_DISK_FAILED;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Failed to read boot sector\n");
        return READ_BOOTSECTOR_FAILED;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT\n");
        free(g_Fat);
        return READ_FAT_FAILED;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read root directory\n");
        free(g_Fat);
        free(g_RootDirectory);
        return READ_ROOT_DIR_FAILED;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find the file %s\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(fileEntry);
        return FIND_FILE_FAILED;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read the file %s\n", argv[2]);
        free(buffer);
        free(g_Fat);
        free(g_RootDirectory);
        free(fileEntry);
        return READ_FILE_FAILED;
    }

    for (size_t i = 0; i < fileEntry->Size; i++) {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(buffer);
    free(g_Fat);
    free(g_RootDirectory);
    return 0;
}