<div align="center">
  
  <!-- Icon at top -->
  <img height="64" alt="Archive-icon-" src="https://github.com/user-attachments/assets/85b1d3fc-d63f-4199-8c89-a9cd6cabd9c2"  
       alt="Archive Manager Logo." 
       width="128">

  
  <h1>File Locker</h1>
  
  <!-- Badges -->
  ![Version](https://img.shields.io/badge/version-1.0-blue)
  ![C](https://img.shields.io/badge/language-C-green)
  ![License](https://img.shields.io/badge/license-MIT-red)
  
  <h3>A lightweight file archiving tool written in C</h3>
  
</div>  

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



