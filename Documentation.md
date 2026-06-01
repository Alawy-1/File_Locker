# Archive Application Documentation

## Overview

A command-line file archiving tool written in C that bundles multiple files into a single binary archive format.

## File Format Specification  

| MAGIC NUMBER | 4 bytes (0x41524348 = "ARCH")  

| VERSION | 2 bytes (0x0001)  

| FILE COUNT | 2 bytes  


| FILE HEADER 1 |  

| - Name Length | 1 byte  

| - File Name | variable  

| - File Size | 4 bytes  

| FILE DATA 1 | variable  

| FILE HEADER 2... |  

## Data Structures

### ArchiveHeader

```c
typedef struct {
    uint32_t magic;      // "ARCH"
    uint16_t version;    // 1
    uint16_t file_count;
} ArchiveHeader;
```

### File Header
```c
typedef struct {
    uint8_t name_len;
    char name[256];
    uint32_t size;
} FileHeader;
```

## Function API

### create_archive()
```c
int create_archive(const char *archive_name);
```
Creates an empty archive file.

Parameters:
- archive_name - Name of the archive to create

Returns:
- 0 on success
- -1 on error

### insert_file()  
```c
int insert_file(const char *archive_name, const char *filename);
```

Adds a file to an existing archive.

Parameters:
- archive_name - Path to existing archive
- filename - Name of file to insert

Returns:
- 0 on success
- -1 on error

### delete_file()
```c
int delete_file(const char *archive_name, const char *filename);
```

Removes a file from the archive.

Parameters:
- archive_name - Path to existing archive
- filename - Name of file to delete

Returns:
- 0 on success
- -1 on error

### list_archive()
```c
void list_archive(const char *archive_name);
```
Displays all files stored in the archive.
Parameters:
- archive_name - Path to existing archive

Returns: (void)
