#define _DEFAULT_SOURCE
#include "inode_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Return the file type as a string */
const char *get_file_type(mode_t mode)
{
    if (S_ISREG(mode))  return "Regular File";
    if (S_ISDIR(mode))  return "Directory";
    if (S_ISLNK(mode))  return "Symbolic Link";
    if (S_ISCHR(mode))  return "Character Device";
    if (S_ISBLK(mode))  return "Block Device";
    if (S_ISFIFO(mode)) return "FIFO/Pipe";
    
    return "Unknown";
}

/* Convert permission bits to rwxrwxrwx */
void get_permissions(mode_t mode, char *perm)
{
    perm[0] = (mode & S_IRUSR) ? 'r' : '-';
    perm[1] = (mode & S_IWUSR) ? 'w' : '-';
    perm[2] = (mode & S_IXUSR) ? 'x' : '-';

    perm[3] = (mode & S_IRGRP) ? 'r' : '-';
    perm[4] = (mode & S_IWGRP) ? 'w' : '-';
    perm[5] = (mode & S_IXGRP) ? 'x' : '-';

    perm[6] = (mode & S_IROTH) ? 'r' : '-';
    perm[7] = (mode & S_IWOTH) ? 'w' : '-';
    perm[8] = (mode & S_IXOTH) ? 'x' : '-';

    perm[9] = '\0';
}

/* Display metadata using stat() */
int display_file_info(const char *filename, struct stat *file_stat)
{
    char permissions[10];

    if (stat(filename, file_stat) == -1) {
        perror(filename);
        return -1;
    }

    get_permissions(file_stat->st_mode, permissions);

    printf("\n========================================\n");
    printf("File:              %s\n", filename);
    printf("========================================\n");

    printf("Inode Number:      %lu\n",
           (unsigned long)file_stat->st_ino);

    printf("Device ID:         %lu\n",
           (unsigned long)file_stat->st_dev);

    printf("File Type:         %s\n",
           get_file_type(file_stat->st_mode));

    printf("File Size:         %lld bytes\n",
           (long long)file_stat->st_size);

    printf("Permissions:       %s\n", permissions);

    printf("Permission Octal:  %04o\n",
           (unsigned int)(file_stat->st_mode & 07777));

    printf("Owner ID (UID):    %u\n",
           (unsigned int)file_stat->st_uid);

    printf("Group ID (GID):    %u\n",
           (unsigned int)file_stat->st_gid);

    printf("Hard Link Count:   %lu\n",
           (unsigned long)file_stat->st_nlink);

    printf("Last Access:       %s", ctime(&file_stat->st_atime));
    printf("Last Modification: %s", ctime(&file_stat->st_mtime));
    printf("Last Status Change: %s", ctime(&file_stat->st_ctime));

    return 0;
}

/* Accept multiple filenames and compare their inodes */
int run_inode_info(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int count = argc - 1;

    struct stat *info = calloc((size_t)count, sizeof(*info));
    int *valid = calloc((size_t)count, sizeof(*valid));

    if (info == NULL || valid == NULL) {
        perror("calloc");
        free(info);
        free(valid);
        return EXIT_FAILURE;
    }

    int failures = 0;

    for (int i = 0; i < count; i++) {
        valid[i] = display_file_info(argv[i + 1], &info[i]) == 0;

        if (!valid[i])
            failures++;
    }

    printf("\n========== INODE COMPARISON ==========\n");

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (!valid[i] || !valid[j])
                continue;

            printf("\n%s <--> %s\n", argv[i + 1], argv[j + 1]);

            if (info[i].st_ino == info[j].st_ino &&
                info[i].st_dev == info[j].st_dev) {
                printf("Result: SAME underlying file\n");
                printf("Reason: Same device and inode.\n");
            } else {
                printf("Result: DIFFERENT files\n");
            }
        }
    }

    free(info);
    free(valid);

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    return run_inode_info(argc, argv);
}
