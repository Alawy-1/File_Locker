
#include "archive.h"

int main(int argc, char **argv) {

    
    if (argc < 2) {
        print_help();
        return 1;
    }

    // Create archive (-c)
    if (strcmp(argv[1], "-c") == 0) {
        if (argc != 3) {
            printf("\033[1;33mUsage: %s -c <archive_name>\033[0m\n", argv[0]);
            return 1;
        }
        return create_archive(argv[2]);
    }

    // Insert file (-i)
    else if (strcmp(argv[1], "-i") == 0) {
        if (argc != 4) {
            printf("\033[1;33mUsage: %s -i <archive_name> <filename>\033[0m\n", argv[0]);
            return 1;
        }
        return insert_file(argv[2], argv[3]);
    }

    // Delete file (-d)
    else if (strcmp(argv[1], "-d") == 0) {
        if (argc != 4) {
            printf("\033[1;33mUsage: %s -d <archive_name> <filename>\033[0m\n", argv[0]);
            return 1;
        }
        return delete_file(argv[2], argv[3]);
    }

    // List files (-l)
    else if (strcmp(argv[1], "-l") == 0) {
        if (argc != 3) {
            printf("\033[1;33mUsage: %s -l <archive_name>\033[0m\n", argv[0]);
            return 1;
        }
        list_archive(argv[2]);
        return 0;
    }

    // Extract file (-x)
    else if (strcmp(argv[1], "-x") == 0) {
        if (argc != 4) {
            printf("\033[1;33mUsage: %s -x <archive_name> <filename>\033[0m\n", argv[0]);
            return 1;
        }
        return extract_file(argv[2], argv[3]);
    }

    // new count files (-n)
    else if (strcmp(argv[1], "-n") == 0) {
        if (argc != 3) {
            printf("Usage: %s -n <archive_name>\n", argv[0]);
            return 1;
        }
        count_files(argv[2]);
    }

    // NEW: Replace file (-r)
    else if (strcmp(argv[1], "-r") == 0) {
        if (argc != 5) {
            printf("Usage: %s -r <archive_name> <old_filename> <new_filename>\n", argv[0]);
            return 1;
        }
        replace_file(argv[2], argv[3], argv[4]);
    }

    // Help (-h)
    else if (strcmp(argv[1], "-h") == 0) {
        if (argc != 2) {
            printf("\033[1;33mUsage: %s -h\033[0m\n", argv[0]);
            return 1;
        }
        print_help();
    }

    else {
        printf("\033[1;31mUnknown command: %s\033[0m\n", argv[1]);
        print_help();
        return 1;
    }

    return 0;
}