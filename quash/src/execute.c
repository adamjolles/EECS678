/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"
#include <stdio.h>
#include "task.h"
#include <sys/wait.h>
#include "quash.h"
#include <fcntl.h>
#include <signal.h>

static int globalPipes[2][2];
static int previousPipe = -1;
static int nextPipe = 0;

int t_id = 1;
background_job_queue_t bq;

char *get_current_directory(bool *should_free)
{
  char *cwd = getcwd(NULL, 0);
  *should_free = true;
  return cwd;
}

const char *lookup_env(const char *env_var)
{
  const char *value = getenv(env_var);
  return (value != NULL) ? value : "null";
}

void check_jobs_bg_status()
{
  if (is_empty_background_job_queue_t(&bq))
  {
    return;
  }

  int bq_size = length_background_job_queue_t(&bq);
  Task j;
  int process_status, pid;
  pid_t return_pid;

  for (int i = 0; i < bq_size; i++)
  {
    j = pop_front_background_job_queue_t(&bq);
    job_process_queue_t job_process_queue = j.process_queue;

    int job_process_queue_size = length_job_process_queue_t(&job_process_queue);
    bool job_run = false;

    for (int k = 0; k < job_process_queue_size; k++)
    {
      pid = pop_front_job_process_queue_t(&job_process_queue);
      return_pid = waitpid(pid, &process_status, WNOHANG);

      if (return_pid == 0)
      {
        job_run = true;
      }
      push_back_job_process_queue_t(&job_process_queue, pid);

      if (job_run)
      {
        break; // Break early if job is still running
      }
    }

    if (job_run)
    {
      push_back_background_job_queue_t(&bq, j);
    }
    else
    {
      print_job_bg_complete(j.t_id, peek_back_job_process_queue_t(&j.process_queue), j.cmd);
      destroy_task(&j);
    }
  }
}

void print_job(int t_id, pid_t pid, const char *cmd)
{
  printf("[%d]\t%8d\t%s\n", t_id, pid, cmd);
  fflush(stdout);
}

void print_job_bg_start(int t_id, pid_t pid, const char *cmd)
{
  printf("Background job started: ");
  print_job(t_id, pid, cmd);
}

void print_job_bg_complete(int t_id, pid_t pid, const char *cmd)
{
  printf("Completed: \t");
  print_job(t_id, pid, cmd);
}

void run_generic(GenericCommand cmd)
{
  char *exec = cmd.args[0];
  char **args = cmd.args;
  execvp(exec, args);

  perror("ERROR: Failed to execute program");
}

void run_echo(EchoCommand cmd)
{
  char **str = cmd.args;

  for (int i = 0; NULL != str[i]; i++)
  {
    printf("%s ", str[i]);
  }

  printf("\n");
  fflush(stdout);
}

void run_export(ExportCommand cmd)
{
  const char *env_var = cmd.env_var;
  const char *val = cmd.val;
  setenv(env_var, val, 1);
}

void run_cd(CDCommand cmd)
{
  const char *dir = cmd.dir;

  if (dir == NULL)
  {
    perror("ERROR: Failed to resolve path");
    return;
  }

  if (chdir(dir) != 0)
  {
    perror("ERROR: Failed to change directory");
    return;
  }

  char *cwd = getcwd(NULL, 0);
  if (cwd == NULL)
  {
    perror("ERROR: Failed to get current working directory");
    return;
  }

  setenv("OLD_PWD", getenv("PWD"), 1);
  setenv("PWD", cwd, 1);
  free(cwd);
}

void run_kill(KillCommand cmd)
{
  int signal = cmd.sig;
  int t_id = cmd.job;

  int bq_size = length_background_job_queue_t(&bq);
  for (int i = 0; i < bq_size; i++)
  {
    Task j = pop_front_background_job_queue_t(&bq);
    if (t_id == j.t_id)
    {
      job_process_queue_t queue = j.process_queue;
      int process_queue_length = length_job_process_queue_t(&queue);

      for (int k = 0; k < process_queue_length; k++)
      {
        int pid = pop_front_job_process_queue_t(&queue);
        kill(pid, signal);
        push_back_job_process_queue_t(&queue, pid);
      }
      push_back_background_job_queue_t(&bq, j);
      break; // Break early if the job is found and signal is sent
    }
    else
    {
      push_back_background_job_queue_t(&bq, j);
    }
  }
}

void run_pwd()
{
  char *cwd = getcwd(NULL, 0);
  if (cwd == NULL)
  {
    perror("ERROR: Failed to get the current working directory");
    return;
  }
  printf("%s\n", cwd);
  free(cwd);
  fflush(stdout);
}

void run_jobs()
{
  if (is_empty_background_job_queue_t(&bq))
  {
    return;
  }

  int job_queue_length = length_background_job_queue_t(&bq);
  for (int i = 0; i < job_queue_length; i++)
  {
    Task job = pop_front_background_job_queue_t(&bq);
    print_job(job.t_id, peek_front_job_process_queue_t(&job.process_queue), job.cmd);
    push_back_background_job_queue_t(&bq, job);
  }

  fflush(stdout);
}

void child_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);

  switch (type)
  {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command of type: %d\n", type);
  }
}

void parent_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);

  switch (type)
  {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command of type: %d\n", type);
  }
}

void create_process(CommandHolder holder, Task *currJob)
{
  bool p_in = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND;

  pid_t pid;
  int fd;

  nextPipe = (nextPipe + 1) % 2;
  previousPipe = (previousPipe + 1) % 2;
  if (p_out)
  {
    pipe(globalPipes[nextPipe]);
  }
  pid = fork();
  push_process_front_to_job(currJob, pid);
  if (pid == 0)
  {
    if (p_out)
    {
      dup2(globalPipes[nextPipe][1], STDOUT_FILENO);
      close(globalPipes[nextPipe][0]);
    }
    if (p_in)
    {
      dup2(globalPipes[previousPipe][0], STDIN_FILENO);
      close(globalPipes[previousPipe][1]);
    }
    if (r_in)
    {
      char *input = holder.redirect_in;
      fd = open(input, O_RDONLY);
      dup2(fd, STDIN_FILENO);
      close(fd);
    }
    if (r_out)
    {
      FILE *file;
      char *output = holder.redirect_out;
      if (r_app)
      {
        file = fopen(output, "a");
        dup2(fileno(file), STDOUT_FILENO);
      }
      else
      {
        file = fopen(output, "w");
        dup2(fileno(file), STDOUT_FILENO);
      }
      fclose(file);
    }

    child_run_command(holder.cmd);
    exit(EXIT_SUCCESS);
  }
  else
  {
    if (p_in)
    {
      close(globalPipes[previousPipe][0]);
    }

    if (p_out)
    {
      close(globalPipes[nextPipe][1]);
    }

    parent_run_command(holder.cmd);
  }
}

void run_script(CommandHolder *holders)
{
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC)
  {
    end_main_loop();
    return;
  }

  CommandType type;
  Task currJob = new_Task();
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i], &currJob);

  if (!(holders[0].flags & BACKGROUND))
  {
    while (!is_empty_job_process_queue_t(&currJob.process_queue))
    {
      int status;
      if (waitpid(peek_back_job_process_queue_t(&currJob.process_queue), &status, 0) != -1)
      {
        pop_back_job_process_queue_t(&currJob.process_queue);
      }
    }
    destroy_task(&currJob);
  }
  else
  {
    currJob.is_background = true;
    currJob.cmd = get_command_string();
    currJob.t_id = t_id++;
    push_back_background_job_queue_t(&bq, currJob);
    print_job_bg_start(currJob.t_id, peek_front_job_process_queue_t(&currJob.process_queue), currJob.cmd);
  }
}

void construct_background_job_queue()
{
  bq = new_background_job_queue_t(1);
}

void destructor_background_job_queue()
{
  while (!(is_empty_background_job_queue_t(&bq)))
  {
    Task j = pop_front_background_job_queue_t(&bq);
    destroy_task(&j);
  // }
  destroy_background_job_queue_t(&bq);
}