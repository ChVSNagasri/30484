#define _POSIX_C_SOURCE 200809L
#include "file_links.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
/* Display inode and hard-link count */
void display_info(const char *filename, int use_lstat)
{
    struct stat st;

    int result = use_lstat
               ? lstat(filename, &st)
               : stat(filename, &st);

    if (result == -1) {
        perror(filename);
        return;
    }

    printf("%-20s Inode: %-12lu Links: %-4lu Type: %s\n",
           filename,
           (unsigned long)st.st_ino,
           (unsigned long)st.st_nlink,
           S_ISLNK(st.st_mode) ? "Symbolic Link" :
           S_ISREG(st.st_mode) ? "Regular File" : "Other");
}

/* Create, inspect, and remove links */
int run_file_links(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *original = argv[1];
    const char *hard_link = "hardlink_file";
    const char *soft_link = "softlink_file";

    struct stat st;

    if (stat(original, &st) == -1) {
        perror(original);
        return EXIT_FAILURE;
    }

    /*
     * link() and symlink() will fail if these names
     * already exist. Nothing is overwritten.
     */
    if (link(original, hard_link) == -1) {
        perror("Creating hard link");
        return EXIT_FAILURE;
    }

    printf("Hard link created: %s\n", hard_link);

    if (symlink(original, soft_link) == -1) {
        perror("Creating symbolic link");

        if (unlink(hard_link) == -1)
            perror("Removing hard link after failure");

        return EXIT_FAILURE;
    }

    printf("Symbolic link created: %s\n", soft_link);

    printf("\n========== BEFORE DELETION ==========\n");

    printf("\nOriginal file:\n");
    display_info(original, 0);

    printf("\nHard link:\n");
    display_info(hard_link, 0);

    printf("\nSymbolic link itself:\n");
    display_info(soft_link, 1);

    printf("\n========== DELETING LINKS ==========\n");

    int failures = 0;

    if (unlink(hard_link) == -1) {
        perror("Removing hard link");
        failures++;
    } else {
        printf("Hard link deleted successfully.\n");
    }

    if (unlink(soft_link) == -1) {
        perror("Removing symbolic link");
        failures++;
    } else {
        printf("Symbolic link deleted successfully.\n");
    }

    printf("\n========== AFTER DELETION ==========\n");
    display_info(original, 0);

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    return run_file_links(argc, argv);
}
