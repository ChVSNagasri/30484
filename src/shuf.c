#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

static void get_permissions(mode_t mode, char *permissions)
{
    permissions[0] = S_ISDIR(mode) ? 'd' : '-';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}

void shuf_date(void)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    dir = opendir(".");

    if (dir == NULL)
    {
        perror("shuf");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        /* Skip hidden files */
        if (entry->d_name[0] == '.')
            continue;

        /* Get file information */
        if (stat(entry->d_name, &file_stat) == -1)
        {
            perror("stat");
            continue;
        }

        char permissions[11];

        get_permissions(file_stat.st_mode, permissions);

        /* Get owner */
        struct passwd *owner = getpwuid(file_stat.st_uid);

        /* Get group */
        struct group *group = getgrgid(file_stat.st_gid);

        /* Format date */
        char date[50];

        struct tm *time_info = localtime(&file_stat.st_mtime);

        if (time_info == NULL)
        {
            strcpy(date, "unknown");
        }
        else
        {
            strftime(
                date,
                sizeof(date),
                "%b %e %H:%M",
                time_info
            );
        }

        /*
         * Custom order:
         *
         * DATE
         * PERMISSIONS
         * OWNER
         * GROUP
         * SIZE
         * NAME
         */

        printf(
            "%-12s %-11s %-8s %-8s %8ld %s\n",
            date,
            permissions,
            owner ? owner->pw_name : "unknown",
            group ? group->gr_name : "unknown",
            (long)file_stat.st_size,
            entry->d_name
        );
    }

    closedir(dir);
}
