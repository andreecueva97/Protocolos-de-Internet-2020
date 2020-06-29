#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
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


int main()
{ 
  int fds;
  pid_t pid;

    FILE* stream;
    stream = fopen("fifo", "w");
    producer(stream);
    close (fds);

  return 0;
} 

