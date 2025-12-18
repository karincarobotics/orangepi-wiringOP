/*
 * gpio_daemon.c:
 *	Copyright (c) 2025 Karinca Robotics
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <wpiExtensions.h>

#include <gertboard.h>
#include <piFace.h>

#include "../version.h"

#define MAX_SIZE 255
#define QUEUE_NAME "/gpio_daemon_queue"

extern int wiringPiDebug ;

#define	PI_USB_POWER_CONTROL	38

int wpMode ;

/* Signal handler */
static int terminate = 0;
void handle_signal(int sig)
{
  if (sig == SIGINT)
  {
    write(STDOUT_FILENO, "Caught SIGINT (Ctrl+C)\n", 23);
  } else if (sig == SIGTERM)
  {
    write(STDOUT_FILENO, "Caught SIGTERM\n", 15);
  }

  terminate = 1;
}

static int parse_message(char* buf, ssize_t buf_size)
{
  ssize_t i = 0;
  while (buf[i++] != ' ')
  {
    if (i >= buf_size)
      return -1;
  }
  buf[i-1] = '\0';
  int pin = atoi(buf);
  int val = atoi(&buf[i]);
  printf("(%d, %d)\n", pin, val);
  if (val == 0)
    digitalWrite(pin, LOW);
  else
    digitalWrite(pin, HIGH);
  return 0;
}

/* Daemonize the current process */
int daemonize(void)
{
  pid_t pid;

  /* 1. Fork and exit parent */
  pid = fork();
  if (pid < 0) {
      return -1;
  }
  if (pid > 0) {
      exit(EXIT_SUCCESS);  // Parent exits
  }

  /* 2. Create new session */
  if (setsid() < 0) {
      return -1;
  }

  /* 3. Ignore SIGHUP */
  signal(SIGHUP, SIG_IGN);

  /* 4. Second fork to prevent acquiring a terminal */
  pid = fork();
  if (pid < 0) {
      return -1;
  }
  if (pid > 0) {
      exit(EXIT_SUCCESS);
  }

  /* 5. Set file permissions mask */
  umask(0);

  /* 6. Change working directory */
  if (chdir("/") < 0) {
      return -1;
  }

  /* 7. Close standard file descriptors */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* 8. Redirect stdio to /dev/null */
  open("/dev/null", O_RDONLY); // stdin
  open("/dev/null", O_RDWR);   // stdout
  open("/dev/null", O_RDWR);   // stderr

  return 0;
}

static int setup_dev()
{
  wiringPiSetup () ;
  wpMode = WPI_MODE_PINS ;
  return 0;
}

/*
 * main:
 *	Start here
 *********************************************************************************
 */
int main (int argc, char *argv [])
{
  if (daemonize() < 0)
  {
    exit(EXIT_FAILURE);
  }

  int i ;
  struct sigaction sa;

  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;    // Or SA_RESTART if desired

  /* Register signal handlers */
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);

  if (getenv ("WIRINGPI_DEBUG") != NULL)
  {
    printf ("gpio: wiringPi debug mode enabled\n") ;
    wiringPiDebug = TRUE ;
  }

  setup_dev();

  mqd_t mq;
  struct mq_attr attr;
  char buffer[MAX_SIZE + 1];
  
  /* Queue attributes */
  attr.mq_flags = 0;                // Blocking mode
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;
  
  /* Open the message queue */
  mq = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT, 0666, &attr);
  if (mq == (mqd_t) -1)
  {
      perror("mq_open");
      return 1;
  }
  printf("Waiting for messages...\n");

  while (!terminate)
  {
    /* Receive messages */
    ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
    if (bytes_read >= 0)
    {
      buffer[bytes_read] = '\0';
      printf("Received: %s\n", buffer);
      int result = parse_message(buffer, bytes_read);
      if (result)
      {
	fprintf(stderr, "Could not parse message");
      }
    }
    else
    {
      perror("mq_receive");
    }
  }

  mq_close(mq);
  mq_unlink(QUEUE_NAME);  // Remove queue when done
  
  return 0;
}
