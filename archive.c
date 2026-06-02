# include "archive.h"

// Create a new archive file
int create_archive(const char *archive_name) {
    FILE *archive = fopen(archive_name, "wb");
    if (archive == NULL) {
        printf("\033[1;31mError: Cannot create archive '%s'\033[0m\n", archive_name);
        return -1;
    }
    
    // Create and write empty archive header
    ArchiveHeader header;
    header.magic = MAGIC_NUMBER;
    header.version = VERSION;
    header.file_count = 0;
    
    fwrite(&header, sizeof(ArchiveHeader), 1, archive);
    fclose(archive);
    
    printf("\033[1;32mArchive '%s' created successfully!\033[0m\n", archive_name);
    return 0;
}

// Insert a file into the archive
int insert_file(const char *archive_name, const char *filename) {
    // Open original file to read
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        printf("\033[1;31mError: File '%s' not found\033[0m\n", filename);
        return -1;
    }
    
    // Get input file size
    fseek(input, 0, SEEK_END);
    uint32_t file_size = ftell(input);
    rewind(input);
    
    // Open archive for reading and writing (read first to get existing content)
    FILE *archive = fopen(archive_name, "rb+");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        fclose(input);
        return -1;
    }
    
    // Read existing header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    // Check magic number
    if (header.magic != MAGIC_NUMBER) {
        printf("\033[1;31mError: Not a valid archive file\033[0m\n");
        fclose(archive);
        fclose(input);
        return -1;
    }
    
    // Check if file already exists in archive
    // (We'll read through all file headers to check)
    long data_start = sizeof(ArchiveHeader);
    int file_exists = 0;
    
    for (int i = 0; i < header.file_count; i++) {
        FileHeader fh;
        fread(&fh.name_len, 1, 1, archive);
        fread(fh.name, 1, fh.name_len, archive);
        fh.name[fh.name_len] = '\0';
        fread(&fh.size, sizeof(uint32_t), 1, archive);
        
        if (strcmp(fh.name, filename) == 0) {
            file_exists = 1;
            break;
        }
        
        // Skip the file data
        fseek(archive, fh.size, SEEK_CUR);
        data_start = ftell(archive);
    }
    
    if (file_exists) {
        printf("\033[1;31mError: File '%s' already exists in archive\033[0m\n", filename);
        fclose(archive);
        fclose(input);
        return -1;
    }
    
    // Go to end of archive to append new file
    fseek(archive, 0, SEEK_END);
    
    // Create and write new file header
    FileHeader fh;
    fh.name_len = strlen(filename);
    strcpy(fh.name, filename);
    fh.size = file_size;
    
    fwrite(&fh.name_len, 1, 1, archive);
    fwrite(fh.name, 1, fh.name_len, archive);
    fwrite(&fh.size, sizeof(uint32_t), 1, archive);
    
    // Write file content
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        fwrite(buffer, 1, bytes_read, archive);
    }
    
    // Update header with new file count
    header.file_count++;
    fseek(archive, 0, SEEK_SET);
    fwrite(&header, sizeof(ArchiveHeader), 1, archive);
    
    fclose(archive);
    fclose(input);
    
    printf("File '%s' inserted into archive '%s'\n", filename, archive_name);
    return 0;
}

// Delete a file from the archive
int delete_file(const char *archive_name, const char *filename) {
    // Open archive for reading
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        return -1;
    }
    
    // Read header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("\033[1;31mError: Not a valid archive file\033[0m\n");
        fclose(archive);
        return -1;
    }
    
    // Read all file headers and data to memory (simplified approach)
    // For large files, we'd need a more complex approach
    FileHeader *headers = (FileHeader*)malloc(header.file_count * sizeof(FileHeader));
    long *data_offsets = (long*)malloc(header.file_count * sizeof(long));
    int *to_keep = (int*)malloc(header.file_count * sizeof(int));
    
    // Read all headers and track data positions
    long current_pos = sizeof(ArchiveHeader);
    int file_index = 0;
    
    for (int i = 0; i < header.file_count; i++) {
        fread(&headers[i].name_len, 1, 1, archive);
        fread(headers[i].name, 1, headers[i].name_len, archive);
        headers[i].name[headers[i].name_len] = '\0';
        fread(&headers[i].size, sizeof(uint32_t), 1, archive);
        
        data_offsets[i] = ftell(archive);
        to_keep[i] = 1;
        
        // Check if this is the file to delete
        if (strcmp(headers[i].name, filename) == 0) {
            to_keep[i] = 0;
        }
        
        // Skip to next header
        fseek(archive, headers[i].size, SEEK_CUR);
    }
    
    fclose(archive);
    
    // Create temporary archive
    char temp_archive[512];
    sprintf(temp_archive, "%s.temp", archive_name);
    FILE *temp = fopen(temp_archive, "wb");
    if (temp == NULL) {
        printf("\033[1;31mError: Cannot create temporary file\033[0m\n");
        free(headers);
        free(data_offsets);
        free(to_keep);
        return -1;
    }
    
    // Write new header
    ArchiveHeader new_header;
    new_header.magic = MAGIC_NUMBER;
    new_header.version = VERSION;
    new_header.file_count = 0;
    fwrite(&new_header, sizeof(ArchiveHeader), 1, temp);
    
    // Write all files except the deleted one
    long new_file_count = 0;
    for (int i = 0; i < header.file_count; i++) {
        if (to_keep[i]) {
            // Write header
            fwrite(&headers[i].name_len, 1, 1, temp);
            fwrite(headers[i].name, 1, headers[i].name_len, temp);
            fwrite(&headers[i].size, sizeof(uint32_t), 1, temp);
            
            // Write data from original archive
            archive = fopen(archive_name, "rb");
            fseek(archive, data_offsets[i], SEEK_SET);
            
            char buffer[1024];
            size_t bytes_read;
            long remaining = headers[i].size;
            while (remaining > 0) {
                size_t to_read = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
                fread(buffer, 1, to_read, archive);
                fwrite(buffer, 1, to_read, temp);
                remaining -= to_read;
            }
            fclose(archive);
            new_file_count++;
        }
    }
    
    // Update header with new file count
    new_header.file_count = new_file_count;
    fseek(temp, 0, SEEK_SET);
    fwrite(&new_header, sizeof(ArchiveHeader), 1, temp);
    
    fclose(temp);
    
    // Replace original archive with temp
    remove(archive_name);
    rename(temp_archive, archive_name);
    
    free(headers);
    free(data_offsets);
    free(to_keep);
    
    printf("\033[1;32mFile '%s' deleted from archive '%s'\033[0m\n", filename, archive_name);
    return 0;
}

// List all files in the archive
void list_archive(const char *archive_name) {
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        return;
    }
    
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("\033[1;31mError: Not a valid archive file\033[0m\n");
        fclose(archive);
        return;
    }
    
    printf("\n\033[1;36m=== Archive: %s ===\033[0m\n", archive_name);
    printf("\033[1;36mVersion: %d\033[0m\n", header.version);
    // printf("Files in archive: %d\n\n", header.file_count);
    
    if (header.file_count == 0) {
        printf("  (empty archive)\n");
    }
    
    for (int i = 0; i < header.file_count; i++) {
        FileHeader fh;
        fread(&fh.name_len, 1, 1, archive);
        fread(fh.name, 1, fh.name_len, archive);
        fh.name[fh.name_len] = '\0';
        fread(&fh.size, sizeof(uint32_t), 1, archive);
        
        printf("\033[1;33m  %2d. %s (%u bytes)\033[0m\n", i + 1, fh.name, fh.size);
        
        // Skip to next file header
        fseek(archive, fh.size, SEEK_CUR);
    }
    
    printf("\n");
    fclose(archive);
}

// Extract a file from the archive
int extract_file(const char *archive_name, const char *filename) {
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        return -1;
    }
    
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("\033[1;31mError: Not a valid archive file\033[0m\n");
        fclose(archive);
        return -1;
    }
    
    // Search for the file
    for (int i = 0; i < header.file_count; i++) {
        FileHeader fh;
        fread(&fh.name_len, 1, 1, archive);
        fread(fh.name, 1, fh.name_len, archive);
        fh.name[fh.name_len] = '\0';
        fread(&fh.size, sizeof(uint32_t), 1, archive);
        
        if (strcmp(fh.name, filename) == 0) {
            // Found the file - extract it
            FILE *output = fopen(filename, "wb");
            if (output == NULL) {
                printf("\033[1;31mError: Cannot create output file '%s'\033[0m\n", filename);
                fclose(archive);
                return -1;
            }
            
            char buffer[1024];
            size_t bytes_read;
            long remaining = fh.size;
            while (remaining > 0) {
                size_t to_read = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
                bytes_read = fread(buffer, 1, to_read, archive);
                fwrite(buffer, 1, bytes_read, output);
                remaining -= bytes_read;
            }
            
            fclose(output);
            printf("\033[1;32mFile '%s' extracted from archive\033[0m\n", filename);
            fclose(archive);
            return 0;
        }
        
        // Skip to next file
        fseek(archive, fh.size, SEEK_CUR);
    }
    
    printf("\033[1;31mError: File '%s' not found in archive\033[0m\n", filename);
    fclose(archive);
    return -1;
}

void count_files(const char *archive_name) {
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        //return -1;
    }
    
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
        fclose(archive);
        //return -1;
    }
    
    printf("Archive '%s' contains %d file(s)\n", archive_name, header.file_count);
    
    fclose(archive);
    //return header.file_count;
}
/*
int replace_file(const char *archive_name, const char *old_filename, const char *new_filename) {
    // First, check if new file exists
    FILE *new_file = fopen(new_filename, "rb");
    if (new_file == NULL) {
        printf("\033[1;31mError: New file '%s' not found\033[0m\n", new_filename);
        return -1;
    }
    
    // Get new file size
    fseek(new_file, 0, SEEK_END);
    uint32_t new_file_size = ftell(new_file);
    rewind(new_file);
    
    // Open archive for reading
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("\033[1;31mError: Archive '%s' not found\033[0m\n", archive_name);
        fclose(new_file);
        return -1;
    }
    
    // Read header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("\033[1;31mError: Not a valid archive file\033[0m\n");
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Read all file headers and track data positions
    FileHeader *headers = (FileHeader*)malloc(header.file_count * sizeof(FileHeader));
    long *data_offsets = (long*)malloc(header.file_count * sizeof(long));
    int found_index = -1;
    
    for (int i = 0; i < header.file_count; i++) {
        fread(&headers[i].name_len, 1, 1, archive);
        fread(headers[i].name, 1, headers[i].name_len, archive);
        headers[i].name[headers[i].name_len] = '\0';
        fread(&headers[i].size, sizeof(uint32_t), 1, archive);
        
        data_offsets[i] = ftell(archive);
        
        if (strcmp(headers[i].name, old_filename) == 0) {
            found_index = i;
        }
        
        // Skip to next header
        fseek(archive, headers[i].size, SEEK_CUR);
    }
    
    if (found_index == -1) {
        printf("Error: File '%s' not found in archive\n", old_filename);
        free(headers);
        free(data_offsets);
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Create temporary archive
    char temp_archive[512];
    sprintf(temp_archive, "%s.temp", archive_name);
    FILE *temp = fopen(temp_archive, "wb");
    if (temp == NULL) {
        printf("Error: Cannot create temporary file\n");
        free(headers);
        free(data_offsets);
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Write new header
    ArchiveHeader new_header;
    new_header.magic = MAGIC_NUMBER;
    new_header.version = VERSION;
    new_header.file_count = header.file_count;  // Count stays the same
    fwrite(&new_header, sizeof(ArchiveHeader), 1, temp);
    
    // Write all files, replacing the target file
    for (int i = 0; i < header.file_count; i++) {
        if (i == found_index) {
            // Write new file header
            uint8_t new_name_len = strlen(new_filename);
            fwrite(&new_name_len, 1, 1, temp);
            fwrite(new_filename, 1, new_name_len, temp);
            fwrite(&new_file_size, sizeof(uint32_t), 1, temp);
            
            // Write new file data
            rewind(new_file);
            char buffer[1024];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), new_file)) > 0) {
                fwrite(buffer, 1, bytes_read, temp);
            }
        } else {
            // Write original file header
            fwrite(&headers[i].name_len, 1, 1, temp);
            fwrite(headers[i].name, 1, headers[i].name_len, temp);
            fwrite(&headers[i].size, sizeof(uint32_t), 1, temp);
            
            // Write original file data
            archive = fopen(archive_name, "rb");
            fseek(archive, data_offsets[i], SEEK_SET);
            
            char buffer[1024];
            size_t bytes_read;
            long remaining = headers[i].size;
            while (remaining > 0) {
                size_t to_read = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
                fread(buffer, 1, to_read, archive);
                fwrite(buffer, 1, to_read, temp);
                remaining -= to_read;
            }
            fclose(archive);
        }
    }
    
    fclose(temp);
    fclose(archive);
    fclose(new_file);
    
    // Replace original with temporary archive
    remove(archive_name);
    rename(temp_archive, archive_name);
    
    free(headers);
    free(data_offsets);
    
    printf("\033[1;32mFile '%s' replaced with '%s' in archive '%s'\033[0m\n", old_filename, new_filename, archive_name);
    return 0;
}
*/

int replace_file(const char *archive_name, const char *old_filename, const char *new_filename) {
    // First, check if new file exists
    FILE *new_file = fopen(new_filename, "rb");
    if (new_file == NULL) {
        printf("Error: New file '%s' not found\n", new_filename);
        return -1;
    }
    
    // Get new file size
    fseek(new_file, 0, SEEK_END);
    uint32_t new_file_size = ftell(new_file);
    rewind(new_file);
    
    // Open archive for reading
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("Error: Archive '%s' not found\n", archive_name);
        fclose(new_file);
        return -1;
    }
    
    // Read header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Read all file headers and track data positions
    FileHeader *headers = (FileHeader*)malloc(header.file_count * sizeof(FileHeader));
    long *data_offsets = (long*)malloc(header.file_count * sizeof(long));
    int found_index = -1;
    
    for (int i = 0; i < header.file_count; i++) {
        fread(&headers[i].name_len, 1, 1, archive);
        fread(headers[i].name, 1, headers[i].name_len, archive);
        headers[i].name[headers[i].name_len] = '\0';
        fread(&headers[i].size, sizeof(uint32_t), 1, archive);
        
        data_offsets[i] = ftell(archive);
        
        if (strcmp(headers[i].name, old_filename) == 0) {
            found_index = i;
        }
        
        // Skip to next header
        fseek(archive, headers[i].size, SEEK_CUR);
    }
    
    if (found_index == -1) {
        printf("Error: File '%s' not found in archive\n", old_filename);
        free(headers);
        free(data_offsets);
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Create temporary archive
    char temp_archive[512];
    sprintf(temp_archive, "%s.temp", archive_name);
    FILE *temp = fopen(temp_archive, "wb");
    if (temp == NULL) {
        printf("Error: Cannot create temporary file\n");
        free(headers);
        free(data_offsets);
        fclose(archive);
        fclose(new_file);
        return -1;
    }
    
    // Write new header
    ArchiveHeader new_header;
    new_header.magic = MAGIC_NUMBER;
    new_header.version = VERSION;
    new_header.file_count = header.file_count;
    fwrite(&new_header, sizeof(ArchiveHeader), 1, temp);
    
    // Write all files, replacing the target file
    for (int i = 0; i < header.file_count; i++) {
        if (i == found_index) {
            // Write new file header
            uint8_t new_name_len = strlen(new_filename);
            fwrite(&new_name_len, 1, 1, temp);
            fwrite(new_filename, 1, new_name_len, temp);
            fwrite(&new_file_size, sizeof(uint32_t), 1, temp);
            
            // Write new file data
            rewind(new_file);
            char buffer[1024];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), new_file)) > 0) {
                fwrite(buffer, 1, bytes_read, temp);
            }
        } else {
            // Write original file header
            fwrite(&headers[i].name_len, 1, 1, temp);
            fwrite(headers[i].name, 1, headers[i].name_len, temp);
            fwrite(&headers[i].size, sizeof(uint32_t), 1, temp);
            
            // Write original file data - FIXED: Don't reopen, use the already open archive
            fseek(archive, data_offsets[i], SEEK_SET);
            
            char buffer[1024];
            size_t bytes_read;
            long remaining = headers[i].size;
            while (remaining > 0) {
                size_t to_read = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
                bytes_read = fread(buffer, 1, to_read, archive);
                fwrite(buffer, 1, bytes_read, temp);
                remaining -= bytes_read;
            }
        }
    }
    
    fclose(temp);
    fclose(archive);
    fclose(new_file);
    
    // Replace original with temporary archive
    if (remove(archive_name) != 0) {
        printf("Error: Could not remove original archive\n");
        return -1;
    }
    
    if (rename(temp_archive, archive_name) != 0) {
        printf("Error: Could not rename temp archive\n");
        return -1;
    }
    
    free(headers);
    free(data_offsets);
    
    printf("File '%s' replaced with '%s' in archive '%s'\n", old_filename, new_filename, archive_name);
    return 0;
}

// shortcut but problem that the insert will add at the end of the archive
// void replace_file(const char *archive_name, const char *old_filename, const char *new_filename){
//     delete_file(archive_name, old_filename);
//     insert_file(archive_name, new_filename);
//     printf("\n%s got replaced by %s, in %s archive.\n", old_filename, new_filename, archive_name);
// }
void print_help(void) {
    // printf("\n=== Archive Application ===\n");
    // printf("Usage:\n");
    // printf("  Create archive:  %s -c <archive_name>\n", "./archive");
    // printf("  Insert file:     %s -i <archive_name> <filename>\n", "./archive");
    // printf("  Delete file:     %s -d <archive_name> <filename>\n", "./archive");
    // printf("  List files:      %s -l <archive_name>\n", "./archive");
    // printf("  Extract file:    %s -x <archive_name> <filename>\n", "./archive");
    // printf("  Count files:     %s -n <archive_name>\n", "./archive");
    // printf("  Replace file:    %s -r <archive_name> <old_file> <new_file>\n", "./archive");
    // printf("  Help:            %s -h\n", "./archive");  

    // printf("\nExamples:\n");

    // printf("  %s -c myarchive.bin\n", "./archive");
    // printf("  %s -i myarchive.bin document.txt\n", "./archive");
    // printf("  %s -d myarchive.bin document.txt\n", "./archive");
    // printf("  %s -l myarchive.bin\n", "./archive");
    // printf("  %s -x myarchive.bin document.txt\n", "./archive");
    printf("\033[1;34m\n=== Archive Application ===\033[0m\n");

    printf("\033[1;35mUsage:\033[0m\n");

    printf("\033[0;35m  Create archive:  %s -c <archive_name>\033[0m\n", "./archive");
    printf("\033[0;35m  Insert file:     %s -i <archive_name> <filename>\033[0m\n", "./archive");
    printf("\033[0;35m  Delete file:     %s -d <archive_name> <filename>\033[0m\n", "./archive");
    printf("\033[0;35m  List files:      %s -l <archive_name>\033[0m\n", "./archive");
    printf("\033[0;35m  Extract file:    %s -x <archive_name> <filename>\033[0m\n", "./archive");
    printf("\033[0;35m  Count files:     %s -n <archive_name>\033[0m\n", "./archive");
    printf("\033[0;35m  Replace file:    %s -r <archive_name> <old_file> <new_file>\033[0m\n", "./archive");
    printf("\033[0;35m  Help:            %s -h\033[0m\n", "./archive");  

    printf("\n\033[1;33mExamples:\033[0m\n");

    printf("\033[0;33m  %s -c myarchive.bin\033[0m\n", "./archive");
    printf("\033[0;33m  %s -i myarchive.bin document.txt\033[0m\n", "./archive");
    printf("\033[0;33m  %s -d myarchive.bin document.txt\033[0m\n", "./archive");
    printf("\033[0;33m  %s -l myarchive.bin\033[0m\n", "./archive");
    printf("\033[0;33m  %s -x myarchive.bin document.txt\033[0m\n", "./archive");
    printf("\n\033[1;36m============================================\n");
    printf("         File Locker\n");
    printf("         Team Name: WARNING\n");
    printf("         Copyright (c) 2026 WARNING Team\n");
    printf("         All rights reserved.\n");
    printf("============================================\033[0m\n");
}