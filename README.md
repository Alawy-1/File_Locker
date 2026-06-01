# File Locker

A lightweight file archiving tool written in C that allows you to bundle multiple files into a single binary archive, with the ability to insert, delete, list, and extract files.

## Features

- **Create** a new archive file
- **Insert** files into an existing archive
- **Delete** files from an archive
- **List** contents of an archive
- **Extract** files from an archive
- Simple binary format with metadata headers

## Archive Format

The archive file consists of:
- **Global Header**: Magic number (ARCH), version number, file count
- **File Headers**: For each file - name length, filename, file size
- **File Data**: Raw file content stored sequentially

## Requirements

- GCC compiler (MinGW on Windows)
- Standard C libraries only (no external dependencies)

## Compilation
```bash
# One-step compilation:
gcc main.c archive.c -o archive



