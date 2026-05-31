#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdlib.h> // free
#include <stdint.h> //uint32/16_t
#include <stdio.h> // FILE 
#include <string.h> //strcmp, strcpy

#define MAGIC_NUMBER 0x41524348  // "ARCH" in hex
#define VERSION 1
#define MAX_FILENAME 256

typedef struct {
    uint32_t magic;      // Magic number (0x41524348)
    uint16_t version;    // Version number
    uint16_t file_count; // Number of files in archive
} ArchiveHeader;

// File header structure (stored before each file)
typedef struct {
    uint8_t name_len;    // Length of filename
    char name[MAX_FILENAME]; // File name
    uint32_t size;       // File size in bytes
} FileHeader;

// Function prototypes (DECLARATIONS)
int create_archive(const char *archive_name);
int insert_file(const char *archive_name, const char *filename);
int delete_file(const char *archive_name, const char *filename);
void list_archive(const char *archive_name);
int extract_file(const char *archive_name, const char *filename);
void print_help(void);

#endif