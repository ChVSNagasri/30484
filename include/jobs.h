
#ifndef SF_JOBS_H
#define SF_JOBS_H

#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define FGS_MAX_JOBS       64
#define FGS_CMD_LENGTH    512
#define FGS_MAX_PROCESSES 32

typedef enum
{
    FGS_ACTIVE = 0,
    FGS_SUSPENDED,
    FGS_FINISHED
} fgs_status_t;

typedef enum
{
    FGS_FOREGROUND = 0,
    BGS_BACKGROUND
} job_execution_t;

typedef struct
{
    pid_t process_id;
    int exit_code;
    bool completed;
} job_process_t;

typedef struct
{
    int job_number;
    pid_t group_id;

    fgs_status_t status;
    job_execution_t execution;

    char cmd[FGS_CMD_LENGTH];

    job_process_t process_list[FGS_MAX_PROCESSES];
    size_t process_count;

    time_t created_time;
    bool notification_pending;
} job_entry_t;

/* Job table management */
void jobs_initialize(void);
void jobs_shutdown(void);

/* Register and remove jobs */
int jobs_register(pid_t group_id,
                  const char *cmd,
                  job_execution_t execution);

int jobs_unregister(int job_number);
void jobs_remove_finished(void);

/* Job lookup */
job_entry_t *jobs_find(int job_number);
job_entry_t *jobs_find_group(pid_t group_id);
job_entry_t *jobs_find_process(pid_t process_id);

/* Foreground and background management */
int jobs_set_fgs(int job_number);
int jobs_set_bgs(int job_number);

/* Job state management */
int jobs_mark_active(int job_number);
int jobs_mark_suspended(int job_number);
int jobs_mark_finished(int job_number, int exit_code);

/* Process tracking */
int jobs_add_process(int job_number, pid_t process_id);
int jobs_update_process(pid_t process_id, int wait_status);

/* Display commands */
void jobs_show_all(void);
void jobs_show_fgs(void);
void jobs_show_bgs(void);

/* Job-control utilities */
int jobs_get_exit_code(int job_number);
const char *jobs_status_string(fgs_status_t status);

#endif /* SF_JOBS_H */
