#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_LENGTH 0x3200C

typedef char mm_message[200]; 

struct mm_segment
{
   int first, last, count;
   mm_message messages[1024];
};


void consumer()
{
  struct mm_segment *shared_memory;

  int fd;
  void* file_memory;
  int integer;

  /* Open the file.  */
  fd = open ("mmapmem", O_RDWR, S_IRUSR | S_IWUSR);
  /* Create the memory-mapping.  */
  shared_memory = (struct mm_segment*) mmap (0, FILE_LENGTH, PROT_READ | PROT_WRITE,
		      MAP_SHARED, fd, 0);
  close (fd);

  while (1)
  {
    if ( shared_memory->count > 0 )
    {
      int lv_aux = shared_memory->last;
      
      printf("Leido de shm: %s\n",shared_memory->messages[lv_aux]);
      shared_memory->last = ( lv_aux + 1 ) % 200;
      shared_memory->count--;
    }else sleep(10);
  }
  /* Release the memory (unnecessary since the program exits).  */
  munmap (file_memory, FILE_LENGTH);
  
} 


int main()
{ 
  int fds;
  pid_t pid;

  consumer ();
  return 0;
} 