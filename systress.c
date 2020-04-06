#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

volatile unsigned long duration = 30;
volatile unsigned long count = 0;
volatile unsigned long it = 0;
volatile int tag = 0;

#define RATE 5

void
handle_alarm (int sig)
{
  printf ("%d %lu %lu\n", tag, it, count);
  if (it++ >= duration)
    {
      _exit (0);
    }

  alarm (RATE);
}

void
handle_usr1 (int sig)
{
  tag++;
}

void
stress_openat (char *path)
{
  while (1)
    {
      int fd = open (path, O_CREAT | O_RDWR);
      close (fd);
      count++;
    }
}

int
main (int argc, char **argv)
{
  char *syscall = NULL, *arg = NULL;
  int c;

  while ((c = getopt (argc, argv, "s:c:a:")) != -1)
    switch (c)
      {
      case 's':
	syscall = strdup (optarg);
	break;
      case 'c':
	count = atoi (optarg) / RATE;
	break;
      case 'a':
	arg = strdup (optarg);
	break;
      default:
	exit (-1);
      }

  if (syscall == NULL)
    {
      printf ("Usage: %s -s <syscall>\n", argv[0]);
      exit (-1);
    }

  pid_t pid = getpid ();
  printf ("Started with pid: %d for %lu sec\n", pid, duration);

  signal (SIGALRM, handle_alarm);
  signal (SIGUSR1, handle_usr1);

  alarm (RATE);

  if (strcmp (syscall, "openat") == 0)
    {
      if (arg == NULL)
	{
	  printf ("Usage: %s -s openat -a <path>\n", argv[0]);
	  exit (-1);
	}
      stress_openat (arg);
    }
}
