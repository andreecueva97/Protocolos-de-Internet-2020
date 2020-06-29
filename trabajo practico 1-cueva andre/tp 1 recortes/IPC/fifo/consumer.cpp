#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 

void consumer(FILE* stream)
{
  char buffer[1024];
  printf("dentro de consumer");
  while (!feof (stream) && !ferror (stream) && fgets (buffer, sizeof (buffer), stream) != NULL)
    printf("Leido de pipe: %s\n",buffer);
//    fputs (buffer, stdout);
  
} 


int main()
{ 
  int fds;
  pid_t pid;

    FILE* stream;

    /* Convert the read file descriptor to a FILE object, and read
       from it.  */
    stream = fopen ("fifo", "r");
    consumer (stream);
    fclose(stream);
  return 0;
} 

