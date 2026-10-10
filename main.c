roller.c src/jobs.c src/lexer.c src/main.c src/parser.c src/producer_consumer.c src/shuf.c src/token.c -lreadline -o shellforge
src/job_controller.c: In function ‘job_controller_cleanup’:
src/job_controller.c:107:9: warning: implicit declaration of function ‘job_controller_restore_terminal’; did you mean ‘job_controller_restore_signals’? [-Wimplicit-function-declaration]
  107 |         job_controller_restore_terminal();
      |         ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      |         job_controller_restore_signals
src/job_controller.c: At top level:
src/job_controller.c:24:25: warning: ‘old_chld’ defined but not used [-Wunused-variable]
   24 | static struct sigaction old_chld;
      |                         ^~~~~~~~
src/main.c: In function ‘main’:
src/main.c:27:6: warning: implicit declaration of function ‘job_controller_init’; did you mean ‘job_control_init’? [-Wimplicit-function-declaration]
   27 |  if (job_controller_init() == -1)
      |      ^~~~~~~~~~~~~~~~~~~
      |      job_control_init
src/main.c:195:5: warning: implicit declaration of function ‘job_controller_cleanup’ [-Wimplicit-function-declaration]
  195 |     job_controller_cleanup();
      |     ^~~~~~~~~~~~~~~~~~~~~~
