# include "archive.h"

// Create a new archive file
int create_archive(const char *archive_name) {
    FILE *archive = fopen(archive_name, "wb");
    if (archive == NULL) {
        printf("Error: Cannot create archive '%s'\n", archive_name);
        return -1;
    }
    
    // Create and write empty archive header
    ArchiveHeader header;
    header.magic = MAGIC_NUMBER;
    header.version = VERSION;
    header.file_count = 0;
    
    fwrite(&header, sizeof(ArchiveHeader), 1, archive);
    fclose(archive);
    
    printf("Archive '%s' created successfully!\n", archive_name);
    return 0;
}

// Insert a file into the archive
int insert_file(const char *archive_name, const char *filename) {
    // Open original file to read
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        printf("Error: File '%s' not found\n", filename);
        return -1;
    }
    
    // Get input file size
    fseek(input, 0, SEEK_END);
    uint32_t file_size = ftell(input);
    rewind(input);
    
    // Open archive for reading and writing (read first to get existing content)
    FILE *archive = fopen(archive_name, "rb+");
    if (archive == NULL) {
        printf("Error: Archive '%s' not found\n", archive_name);
        fclose(input);
        return -1;
    }
    
    // Read existing header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    // Check magic number
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
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
        printf("Error: File '%s' already exists in archive\n", filename);
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
        printf("Error: Archive '%s' not found\n", archive_name);
        return -1;
    }
    
    // Read header
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
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
        printf("Error: Cannot create temporary file\n");
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
    
    printf("File '%s' deleted from archive '%s'\n", filename, archive_name);
    return 0;
}

// List all files in the archive
void list_archive(const char *archive_name) {
    FILE *archive = fopen(archive_name, "rb");
    if (archive == NULL) {
        printf("Error: Archive '%s' not found\n", archive_name);
        return;
    }
    
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
        fclose(archive);
        return;
    }
    
    printf("\n=== Archive: %s ===\n", archive_name);
    printf("Version: %d\n", header.version);
    printf("Files in archive: %d\n\n", header.file_count);
    
    if (header.file_count == 0) {
        printf("  (empty archive)\n");
    }
    
    for (int i = 0; i < header.file_count; i++) {
        FileHeader fh;
        fread(&fh.name_len, 1, 1, archive);
        fread(fh.name, 1, fh.name_len, archive);
        fh.name[fh.name_len] = '\0';
        fread(&fh.size, sizeof(uint32_t), 1, archive);
        
        printf("  %2d. %s (%u bytes)\n", i + 1, fh.name, fh.size);
        
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
        printf("Error: Archive '%s' not found\n", archive_name);
        return -1;
    }
    
    ArchiveHeader header;
    fread(&header, sizeof(ArchiveHeader), 1, archive);
    
    if (header.magic != MAGIC_NUMBER) {
        printf("Error: Not a valid archive file\n");
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
                printf("Error: Cannot create output file '%s'\n", filename);
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
            printf("File '%s' extracted from archive\n", filename);
            fclose(archive);
            return 0;
        }
        
        // Skip to next file
        fseek(archive, fh.size, SEEK_CUR);
    }
    
    printf("Error: File '%s' not found in archive\n", filename);
    fclose(archive);
    return -1;
}

void print_help(void) {
    printf("\n=== Archive Application ===\n");
    printf("Usage:\n");
    printf("  Create archive:  %s -c <archive_name>\n", "./archive");
    printf("  Insert file:     %s -i <archive_name> <filename>\n", "./archive");
    printf("  Delete file:     %s -d <archive_name> <filename>\n", "./archive");
    printf("  List files:      %s -l <archive_name>\n", "./archive");
    printf("  Extract file:    %s -x <archive_name> <filename>\n", "./archive");
    printf("\nExamples:\n");
    printf("  %s -c myarchive.bin\n", "./archive");
    printf("  %s -i myarchive.bin document.txt\n", "./archive");
    printf("  %s -d myarchive.bin document.txt\n", "./archive");
    printf("  %s -l myarchive.bin\n", "./archive");
    printf("  %s -x myarchive.bin document.txt\n", "./archive");
}