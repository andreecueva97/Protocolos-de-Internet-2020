#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>

#define FILE_LENGTH 0x3200C

typedef char mm_message[200]; 

struct mm_segment
{
   int first, last, count;
   mm_message messages[1024];
};

int segment_id;
struct mm_segment *shared_memory;

int flag = 1;
void Free_SHM (int signal_number)
{
  /* Release the memory (unnecessary since the program exits).  */
  munmap (shared_memory, FILE_LENGTH);
  flag = 0;
}

void producer()
{ 
// Registrar funcion que limpie SHM segment si Ctrl+C
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &Free_SHM;
  sigaction (SIGINT, &sa, NULL);

  int fd;

  struct mm_segment ls_aux;
  /* Prepare a file large enough to hold an unsigned integer.  */
  fd = open ("mmapmem", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  write (fd, &ls_aux, sizeof(struct mm_segment));
  lseek (fd, 0, SEEK_SET);

  /* Create the memory-mapping.  */
  shared_memory = (struct mm_segment*) mmap (0, FILE_LENGTH, PROT_WRITE, MAP_SHARED, fd, 0);
  close (fd);

  shared_memory->first = 0;
  shared_memory->count = 0;
  shared_memory->last = 1;

      while(flag)
      {
        if ( shared_memory->count < 200 )
        {
          shared_memory->first = ( shared_memory->first + 1 ) % 200;
          int lv_aux = shared_memory->first;
          fgets(shared_memory->messages[lv_aux], 200, stdin);
          shared_memory->count++;
        }
//  sprintf (shared_memory->messages[0], "Hello, world.");
      }
} 


int main()
{ 
  int fds;
  pid_t pid;

  producer();

  return 0;
} 

