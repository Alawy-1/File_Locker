# File Locker

A file archiving tool written in C for managing and organizing files into a single archive.

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

## Compilation
### Compilation and linking object file:
```bash
gcc -c main.c archive.c && gcc main.o archive.o -o a2
```
### One-step compilation:
```bash
gcc main.c archive.c -o archive
```



