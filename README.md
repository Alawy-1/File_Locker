<div align="center">
  
  <!-- Icon at top -->
  <img height="64" alt="Archive-icon-" src="https://github.com/user-attachments/assets/3617b792-9fe0-4291-9553-62f8d3d58d2e"  
       alt="Archive Manager Logo." 
       width="128">
  
  <h1>📦 ARCHIVE MANAGER</h1>
  
  <!-- Badges -->
  ![Version](https://img.shields.io/badge/version-1.0-blue)
  ![C](https://img.shields.io/badge/language-C-green)
  ![License](https://img.shields.io/badge/license-MIT-red)
  
  <h3>A lightweight file archiving tool written in C</h3>
  
  <!-- ASCII Art Alternative -->
  <pre>
╔══════════════════════════════════════════╗
║     ┌─────────┐                          ║
║     │  ARCH   │  Archive Manager v1.0    ║
║     │  v1.0   │                          ║
║     └─────────┘                          ║
║     [#########]                           ║
╚══════════════════════════════════════════╝
  </pre>
  
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



