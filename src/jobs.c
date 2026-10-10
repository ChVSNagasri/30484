
#define _POSIX_C_SOURCE 200809L

#include "jobs.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

static job_entry_t job_table[FGS_MAX_JOBS];
static int next_job_number = 1;
static bool initialized = false;

static job_entry_t *find_free_slot(void)
{
    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        if (job_table[i].job_number == 0)
            return &job_table[i];
    }

    return NULL;
}

void jobs_initialize(void)
{
    memset(job_table, 0, sizeof(job_table));
    next_job_number = 1;
    initialized = true;
}

void jobs_shutdown(void)
{
    memset(job_table, 0, sizeof(job_table));
    initialized = false;
}

int jobs_register(pid_t group_id,
                  const char *cmd,
                  job_execution_t execution)
{
    if (!initialized)
        jobs_initialize();

    if (group_id <= 0 || cmd == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    job_entry_t *job = find_free_slot();

    if (job == NULL)
    {
        errno = ENOSPC;
        return -1;
    }

    memset(job, 0, sizeof(*job));

    job->job_number = next_job_number++;

    if (next_job_number <= 0)
        next_job_number = 1;

    job->group_id = group_id;
    job->status = FGS_ACTIVE;
    job->execution = execution;
    job->created_time = time(NULL);
    job->notification_pending = false;

    snprintf(job->cmd, sizeof(job->cmd), "%s", cmd);

    return job->job_number;
}

int jobs_unregister(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    memset(job, 0, sizeof(*job));
    return 0;
}

int jobs_unregister_group(pid_t group_id)
{
    job_entry_t *job = jobs_find_group(group_id);

    if (job == NULL)
        return -1;

    return jobs_unregister(job->job_number);
}

void jobs_remove_finished(void)
{
    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        if (job_table[i].job_number != 0 &&
            job_table[i].status == FGS_FINISHED &&
            !job_table[i].notification_pending)
        {
            memset(&job_table[i], 0, sizeof(job_table[i]));
        }
    }
}

job_entry_t *jobs_find(int job_number)
{
    if (!initialized)
        return NULL;

    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        if (job_table[i].job_number == job_number)
            return &job_table[i];
    }

    return NULL;
}

job_entry_t *jobs_find_group(pid_t group_id)
{
    if (!initialized)
        return NULL;

    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        if (job_table[i].job_number != 0 &&
            job_table[i].group_id == group_id)
        {
            return &job_table[i];
        }
    }

    return NULL;
}

job_entry_t *jobs_find_process(pid_t process_id)
{
    if (!initialized)
        return NULL;

    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        job_entry_t *job = &job_table[i];

        if (job->job_number == 0)
            continue;

        for (size_t j = 0; j < job->process_count; j++)
        {
            if (job->process_list[j].process_id == process_id)
                return job;
        }
    }

    return NULL;
}

int jobs_set_fgs(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    job->execution = FGS_FOREGROUND;
    return 0;
}

int jobs_set_bgs(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    job->execution = BGS_BACKGROUND;
    return 0;
}

int jobs_mark_active(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    job->status = FGS_ACTIVE;
    return 0;
}

int jobs_mark_suspended(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    job->status = FGS_SUSPENDED;
    job->notification_pending = true;
    return 0;
}

int jobs_mark_finished(int job_number, int exit_code)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    job->status = FGS_FINISHED;
    job->notification_pending = true;

    for (size_t i = 0; i < job->process_count; i++)
    {
        if (!job->process_list[i].completed)
        {
            job->process_list[i].completed = true;
            job->process_list[i].exit_code = exit_code;
        }
    }

    return 0;
}

int jobs_add_process(int job_number, pid_t process_id)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL || process_id <= 0)
        return -1;

    if (job->process_count >= FGS_MAX_PROCESSES)
    {
        errno = ENOSPC;
        return -1;
    }

    for (size_t i = 0; i < job->process_count; i++)
    {
        if (job->process_list[i].process_id == process_id)
            return 0;
    }

    job_process_t *process =
        &job->process_list[job->process_count++];

    memset(process, 0, sizeof(*process));
    process->process_id = process_id;

    return 0;
}

int jobs_update_process(pid_t process_id, int wait_status)
{
    job_entry_t *job = jobs_find_process(process_id);

    if (job == NULL)
        return -1;

    job_process_t *process = NULL;

    for (size_t i = 0; i < job->process_count; i++)
    {
        if (job->process_list[i].process_id == process_id)
        {
            process = &job->process_list[i];
            break;
        }
    }

    if (process == NULL)
        return -1;

    if (WIFSTOPPED(wait_status))
    {
        job->status = FGS_SUSPENDED;
        job->notification_pending = true;
        return 0;
    }

#ifdef WIFCONTINUED
    if (WIFCONTINUED(wait_status))
    {
        job->status = FGS_ACTIVE;
        return 0;
    }
#endif

    if (WIFEXITED(wait_status))
    {
        process->completed = true;
        process->exit_code = WEXITSTATUS(wait_status);
    }
    else if (WIFSIGNALED(wait_status))
    {
        process->completed = true;
        process->exit_code = 128 + WTERMSIG(wait_status);
    }
    else
    {
        return 0;
    }

    size_t finished = 0;

    for (size_t i = 0; i < job->process_count; i++)
    {
        if (job->process_list[i].completed)
            finished++;
    }

    if (job->process_count > 0 &&
        finished == job->process_count)
    {
        job->status = FGS_FINISHED;
        job->notification_pending = true;
    }

    return 0;
}

void jobs_show_all(void)
{
    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        job_entry_t *job = &job_table[i];

        if (job->job_number == 0)
            continue;

        printf("[%d] %s\t%s\n",
               job->job_number,
               jobs_status_string(job->status),
               job->cmd);
    }
}

void jobs_show_fgs(void)
{
    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        job_entry_t *job = &job_table[i];

        if (job->job_number != 0 &&
            job->execution == FGS_FOREGROUND)
        {
            printf("[%d] %s\t%s\n",
                   job->job_number,
                   jobs_status_string(job->status),
                   job->cmd);
        }
    }
}

void jobs_show_bgs(void)
{
    for (int i = 0; i < FGS_MAX_JOBS; i++)
    {
        job_entry_t *job = &job_table[i];

        if (job->job_number != 0 &&
            job->execution == BGS_BACKGROUND)
        {
            printf("[%d] %s\t%s\n",
                   job->job_number,
                   jobs_status_string(job->status),
                   job->cmd);
        }
    }
}

int jobs_get_exit_code(int job_number)
{
    job_entry_t *job = jobs_find(job_number);

    if (job == NULL)
        return -1;

    for (size_t i = job->process_count; i > 0; i--)
    {
        if (job->process_list[i - 1].completed)
            return job->process_list[i - 1].exit_code;
    }

    return -1;
}

const char *jobs_status_string(fgs_status_t status)
{
    switch (status)
    {
        case FGS_ACTIVE:
            return "Running";

        case FGS_SUSPENDED:
            return "Stopped";

        case FGS_FINISHED:
            return "Done";

        default:
            return "Unknown";
    }
}
