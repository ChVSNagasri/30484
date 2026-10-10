
#ifndef INODE_INFO_H
#define INODE_INFO_H

#include <sys/types.h>
#include <sys/stat.h>

const char *get_file_type(mode_t mode);
void get_permissions(mode_t mode, char *permissions);
int display_file_info(const char *filename, struct stat *file_stat);
int run_inode_info(int argc, char *argv[]);

#endif

