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
#include <signal.h>
#include <fcntl.h>

#include "quash.h"

#include "deque.h"

static int globalPipes[2][2];
static int prevPipe = -1;
static int nextPipe = 0;

IMPLEMENT_DEQUE(pidQue, pid_t)
IMPLEMENT_DEQUE(jobQue, QuashJob)

#include <string.h>
#include <sys/wait.h>

bool jobQueInitialized = false;

jobQue myJobQue;

pidQue myPIDs;

int jobCount = 0;

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME() \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

#define BSIZE 1024

char *get_current_directory(bool *should_free)
{
  // TODO: Get the current working directory. This will fix the prompt path.
  // Change this to true if necessary
  *should_free = false;
  return getcwd(NULL, BSIZE);
}

// Returns the value of an environment variable env_var
const char *lookup_env(const char *env_var)
{
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  const char *value = getenv(env_var);
  return value;
}

// Check the status of background jobs
void check_jobs_bg_status()
{
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  QuashJob currJob;

  if (jobQueInitialized)
  {
    int lengthQue = length_jobQue(&myJobQue);

    for (int i = 0; i < lengthQue; ++i)
    {

      currJob = pop_front_jobQue(&myJobQue);
      pid_t currPID;

      bool jobDone = true;

      for (int i = 0; i < length_pidQue(&currJob.pids); ++i)
      {
        int status;
        currPID = pop_front_pidQue(&currJob.pids);
        pid_t check_pid = waitpid(currPID, &status, WNOHANG);

        if (check_pid < 0)
        {
          /*
            Process terminated with an error.
          */
        }
        else if (check_pid == 0)
        {
          /*
            The child process is still running.
          */
          jobDone = false;
          push_back_pidQue(&currJob.pids, currPID);
        }
        else
        {
          /*
            The child process is done.
          */
          push_back_pidQue(&currJob.pids, check_pid); // add back to pidQue to track finished processes
          // This might result in an error, as I'm unsure about what happens if you wait on a child
          // process that has already terminated...
        }
      }

      if (jobDone)
      {
        print_job_bg_complete(currJob.jobID, currPID, currJob.cmd);
      }
      else
      {
        push_back_jobQue(&myJobQue, currJob);
      }
    }
  }
  else
  {
    printf("Job Que is Empty! Remove this line!");
  }
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char *cmd)
{
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char *cmd)
{
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char *cmd)
{
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd)
{
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char *exec = cmd.args[0];
  char **args = cmd.args;
  execvp(exec, args);

  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd)
{
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char **str = cmd.args;

  // TODO: Remove warning silencers
  for (int i = 0; str[i] != NULL; ++i)
  {
    printf("%s ", str[i]);
  }
  printf("\n");

  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd)
{
  // Write an environment variable
  const char *env_var = cmd.env_var;
  const char *val = cmd.val;

  if (setenv(env_var, val, 1))
  {
    perror("ERROR: Failed to set environment variable");
  }
}

// Changes the current working directory
void run_cd(CDCommand cmd)
{
  // Get the directory name
  const char *dir = cmd.dir;

  // Check if the directory is valid
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

  const char *tmp = getenv("PWD");

  if (setenv("OLD_PWD", tmp, 1) != 0)
  {
    return;
  }
  if (setenv("PWD", dir, 1) != 0)
  {
    return;
  }

  // TODO: Change directory
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd)
{
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  // (void)signal; // Silence unused variable warning
  // (void)job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  // IMPLEMENT_ME();
  for (int i = 0; i < length_jobQue(&myJobQue); ++i)
  {                                                 // Iterate through the job list
    QuashJob currJob = pop_front_jobQue(&myJobQue); // get the current QuashJob
    if (currJob.jobID == job_id)
    { // if the currJob is the job we would like to kill

      for (int i = 0; i < length_pidQue(&currJob.pids); ++i)
      { // iterate through the job's pidQue
        pid_t currentPID = pop_back_pidQue(&currJob.pids);
        kill(currentPID, signal); // kills each process in the job
        push_front_pidQue(&currJob.pids, currentPID);
      }
      push_back_jobQue(&myJobQue, currJob);
    }
    else
    { // if the currJob isn't the job we want to kill
      push_back_jobQue(&myJobQue, currJob);
    }
  }
  check_jobs_bg_status();
}

// Prints the current working directory to stdout
void run_pwd()
{
  // TODO: Print the current working directory
  // IMPLEMENT_ME();

  bool should_free;
  char *currentDirectory = get_current_directory(&should_free);
  printf("%s\n", currentDirectory);
  if (should_free)
  {
    free(currentDirectory);
  }

  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs()
{
  // TODO: Print background jobs
  // IMPLEMENT_ME();

  QuashJob currJob;
  for (int i = 0; i < length_jobQue(&myJobQue); ++i)
  {
    currJob = pop_front_jobQue(&myJobQue);
    pidQue *pidList = &currJob.pids;
    pid_t firstPID = peek_front_pidQue(pidList);
    print_job(currJob.jobID, firstPID, currJob.cmd);
    push_back_jobQue(&myJobQue, currJob);
  }

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
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
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
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
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, pidQue *parentPidQue)
{
  // Read the flags field from the parser
  bool p_in = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
  (void)p_in;  // Silence unused variable warning
  (void)p_out; // Silence unused variable warning
  (void)r_in;  // Silence unused variable warning
  (void)r_out; // Silence unused variable warning
  (void)r_app; // Silence unused variable warning

  // TODO: Setup pipes, redirects, and new process
  // IMPLEMENT_ME();

  // parent_run_command(holder.cmd); // This should be done in the parent branch of
  //  a fork
  // child_run_command(holder.cmd); // This should be done in the child branch of a fork

  if (p_out)
  {
    pipe(globalPipes[nextPipe]);
  }
  pid_t pid = fork();
  int fd;

  if (pid == 0)
  { // child
    if (p_out)
    {
      dup2(globalPipes[nextPipe][1], STDOUT_FILENO);
      close(globalPipes[nextPipe][0]);
    }
    if (p_in)
    {
      dup2(globalPipes[prevPipe][0], STDIN_FILENO);
      close(globalPipes[prevPipe][1]);
    }
    if (r_in)
    {
      fd = open(holder.redirect_in, O_RDONLY);
      dup2(fd, STDIN_FILENO);
      close(fd);
    }
    if (r_out)
    {
      if (r_app)
      {
        fd = open(holder.redirect_out, O_WRONLY | O_CREAT | O_APPEND, 0666);
      }
      else
      {
        fd = open(holder.redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    child_run_command(holder.cmd);
    exit(0);
  }
  else if (pid > 0)
  { // parent
    int status;
    // waitpid(-1, &status, 0); //wait for child to finish
    if (p_out || p_in)
    {
      waitpid(-1, &status, 0);
    }
    if (p_in)
    {
      close(globalPipes[prevPipe][0]);
    }
    if (p_out)
    {
      close(globalPipes[nextPipe][1]);
    }
    nextPipe = (nextPipe + 1) % 2; // switch pipes - after child writes to nextPipe,  parent process switches nextPipe to point to the other pipe
    prevPipe = (prevPipe + 1) % 2; // switch pipes
    parent_run_command(holder.cmd);
    push_back_pidQue(parentPidQue, pid);
    return;
  }
  else
  {
    fprintf(stderr, "Fork failed");
    exit(1);
  }
}

// Run a list of commands
void run_script(CommandHolder *holders)
{
  if (!jobQueInitialized)
  {
    myJobQue = new_jobQue(100);
    jobQueInitialized = true;
  }

  // Test Code
  //  pidQue myPidQue = new_pidQue(100);
  //  push_back_pidQue(&myPidQue, 1);
  //  QuashJob currentJob = {.jobID = 1, .pids = &myPidQue, .cmd = "Hello World!"};
  //  push_back_jobQue(&myJobQue, currentJob);
  //  QuashJob myJob;
  //  myJob = pop_front_jobQue(&myJobQue);
  //  print_job(myJob.jobID, 1, myJob.cmd);

  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC)
  {
    end_main_loop();
    return;
  }

  // Create an empty pidQue
  pidQue myPidQue;
  myPidQue = new_pidQue(100);

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i], &myPidQue);
  if (!(holders[0].flags & BACKGROUND))
  {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    // IMPLEMENT_ME();

    int status;
    for (int i = 0; i < length_pidQue(&myPidQue); ++i)
    {
      pid_t currPid = pop_front_pidQue(&myPidQue);
      waitpid(currPid, &status, 0);
    }
    destroy_pidQue(&myPidQue);
  }

  else
  {
    // A background job.
    // TODO: Push the new job to the job queue
    // IMPLEMENT_ME();

    // Creates a new QuashJob variabe to store the new job
    ++jobCount;
    QuashJob myJob = {.jobID = jobCount, .pids = myPidQue, .cmd = get_command_string()}; // This is incorrect, need the string for the cmd

    push_back_jobQue(&myJobQue, myJob); // Adds current job to the queue

    // TODO: Once jobs are implemented, uncomment and fill the following line
    if (!is_empty_pidQue(&myJob.pids))
      print_job_bg_start(myJob.jobID, peek_front_pidQue(&myJob.pids), myJob.cmd);
  }
}
