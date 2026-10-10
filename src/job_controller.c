
#define _POSIX_C_SOURCE 200809L

#include "job_control.h"
#include "jobs.h"

#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

static pid_t shell_group = -1;
static bool interactive_shell = false;
static struct termios shell_terminal_modes;
static bool saved_terminal_modes = false;

static struct sigaction old_int;
static struct sigaction old_quit;
static struct sigaction old_tstp;
static struct sigaction old_ttin;
static struct sigaction old_ttou;
static struct sigaction old_chld;

static bool signals_saved = false;

int job_controller_setup_signals(void)
{
    struct sigaction ignore_action = {0};

    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);

    if (sigaction(SIGINT, &ignore_action, &old_int) == -1 ||
        sigaction(SIGQUIT, &ignore_action, &old_quit) == -1 ||
        sigaction(SIGTSTP, &ignore_action, &old_tstp) == -1 ||
        sigaction(SIGTTIN, &ignore_action, &old_ttin) == -1 ||
        sigaction(SIGTTOU, &ignore_action, &old_ttou) == -1)
    {
        return -1;
    }

    signals_saved = true;
    return 0;
}

void job_controller_restore_signals(void)
{
    if (!signals_saved)
        return;

    sigaction(SIGINT, &old_int, NULL);
    sigaction(SIGQUIT, &old_quit, NULL);
    sigaction(SIGTSTP, &old_tstp, NULL);
    sigaction(SIGTTIN, &old_ttin, NULL);
    sigaction(SIGTTOU, &old_ttou, NULL);

    signals_saved = false;
}

int job_controller_init(void)
{
    interactive_shell = isatty(STDIN_FILENO);

    if (!interactive_shell)
    {
        shell_group = getpgrp();
        return 0;
    }

    if (job_controller_setup_signals() == -1)
        return -1;

    while (tcgetpgrp(STDIN_FILENO) != (shell_group = getpgrp()))
    {
        if (kill(-shell_group, SIGTTIN) == -1)
            return -1;
    }

    shell_group = getpid();

    if (setpgid(shell_group, shell_group) == -1 &&
        errno != EACCES && errno != EPERM)
    {
        job_controller_restore_signals();
        return -1;
    }

    shell_group = getpgrp();

    if (tcsetpgrp(STDIN_FILENO, shell_group) == -1)
    {
        job_controller_restore_signals();
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, &shell_terminal_modes) == 0)
        saved_terminal_modes = true;

    return 0;
}

void job_controller_cleanup(void)
{
    if (interactive_shell)
        job_control_restore_terminal();

    job_controller_restore_signals();
}

pid_t job_controller_shell_pgid(void)
{
    return shell_group;
}

pid_t job_controller_foreground_pgid(void)
{
    if (!interactive_shell)
        return getpgrp();

    return tcgetpgrp(STDIN_FILENO);
}

int job_controller_give_terminal(pid_t process_group)
{
    if (!interactive_shell)
        return 0;

    if (process_group <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    return tcsetpgrp(STDIN_FILENO, process_group);
}

int job_control_restore_terminal(void)
{
    if (!interactive_shell)
        return 0;

    if (tcsetpgrp(STDIN_FILENO, shell_group) == -1)
        return -1;

    if (saved_terminal_modes &&
        tcsetattr(STDIN_FILENO, TCSADRAIN,
                  &shell_terminal_modes) == -1)
    {
        return -1;
    }

    return 0;
}

int job_controller_start_fgs(pid_t process_group)
{
    return job_controller_give_terminal(process_group);
}

int job_controller_wait_fgs(pid_t process_group)
{
    int status;
    pid_t result;

    if (process_group <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    for (;;)
    {
        result = waitpid(-process_group, &status,
                         WUNTRACED
#ifdef WCONTINUED
                         | WCONTINUED
#endif
        );

        if (result > 0)
        {
            jobs_update_process(result, status);

            if (WIFSTOPPED(status))
                break;

            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                /*
                 * Continue waiting until no more children
                 * remain in this process group.
                 */
                continue;
            }

            continue;
        }

        if (result == -1 && errno == EINTR)
            continue;

        if (result == -1 && errno == ECHILD)
            break;

        if (result == -1)
            return -1;
    }

    job_control_restore_terminal();

    job_entry_t *job = jobs_find_group(process_group);

    if (job != NULL && job->status == FGS_SUSPENDED)
        jobs_set_bgs(job->job_number);

    return 0;
}

int job_controller_start_bgs(pid_t process_group)
{
    if (process_group <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    if (kill(-process_group, SIGCONT) == -1)
        return -1;

    job_entry_t *job = jobs_find_group(process_group);

    if (job != NULL)
    {
        jobs_mark_active(job->job_number);
        jobs_set_bgs(job->job_number);
    }

    return 0;
}

int job_controller_continue_bgs(pid_t process_group)
{
    return job_controller_start_bgs(process_group);
}

int job_controller_create_group(pid_t process_id,
                                pid_t process_group)
{
    if (process_id <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    return setpgid(process_id, process_group);
}

int job_controller_signal_group(pid_t process_group,
                                int signal_number)
{
    if (process_group <= 0)
    {
        errno = EINVAL;
        return -1;
    }

    return kill(-process_group, signal_number);
}

bool job_controller_is_interactive(void)
{
    return interactive_shell;
}

bool job_controller_has_terminal(void)
{
    return interactive_shell &&
           tcgetpgrp(STDIN_FILENO) == shell_group;
}
