#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void producer(FILE* stream)
{ 
 char a[1024];

      while(1)
      {
        fgets(a, 1024, stdin);

        fprintf (stream, "%s", a);
        fflush (stream);
      }
} 

void consumer(FILE* stream)
{
  char buffer[1024];

  while (!feof (stream) && !ferror (stream) && fgets (buffer, sizeof (buffer), stream) != NULL)
    printf("Leido de pipe: %s\n",buffer);
//    fputs (buffer, stdout);
  
} 


int main()
{ 
  int fds[2];
  pid_t pid;

  /* Create a pipe.  File descriptors for the two ends of the pipe are
     placed in fds.  */
  pipe (fds);
  /* Fork a child process.  */
  pid = fork ();
  if (pid == (pid_t) 0) {
    FILE* stream;
    /* This is the child process.  Close our copy of the write end of
       the file descriptor.  */
    close (fds[1]);
    /* Convert the read file descriptor to a FILE object, and read
       from it.  */
    stream = fdopen (fds[0], "r");
    consumer (stream);
    close (fds[0]);
  }
  else {
    /* This is the parent process.  */
    FILE* stream;
    /* Close our copy of the read end of the file descriptor.  */
    close (fds[0]);
    /* Convert the write file descriptor to a FILE object, and write
       to it.  */
    stream = fdopen (fds[1], "w");
    producer(stream);
    close (fds[1]);
  }

  return 0;
} 