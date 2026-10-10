
#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>
#include <stdbool.h>

void job_control_init(void);
void job_control_cleanup(void);

pid_t job_control_shell_pgid(void);
pid_t job_control_foreground_pgid(void);

int job_control_give_terminal(pid_t pgid);
int job_control_restore_terminal(void);

int job_control_start_fgs(pid_t pgid);
int job_control_wait_fgs(pid_t pgid);

int job_control_start_bgs(pid_t pgid);
int job_control_continue_bgs(pid_t pgid);

int job_control_create_group(pid_t pid, pid_t pgid);
int job_control_signal_group(pid_t pgid, int signal_number);

bool job_control_is_interactive(void);
bool job_control_has_terminal(void);

int job_control_setup_signals(void);
void job_control_restore_signals(void);

#endif

