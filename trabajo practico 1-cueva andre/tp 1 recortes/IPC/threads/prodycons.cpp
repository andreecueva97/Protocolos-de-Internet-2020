#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>


typedef char message[200]; 

struct queue
{
   int first, last, count;
   message messages[1024];
};

struct queue shared_queue;

int flag = 1;

void Free_Resources (int signal_number)
{
  flag = 0;
}

void* producer(void *)
{ 
  shared_queue.first = 0;
  shared_queue.count = 0;
  shared_queue.last = 1;

      while(flag)
      {
        if ( shared_queue.count < 200 )
        {
          shared_queue.first = ( shared_queue.first + 1 ) % 200;
          int lv_aux = shared_queue.first;
          fgets(shared_queue.messages[lv_aux], 200, stdin);
          shared_queue.count++;
        }
      }
} 
    
void* consumer(void *)
{
  while (flag)
  {
    if ( shared_queue.count > 0 )
    {
      int lv_aux = shared_queue.last;
      
      printf("Leido de queue: %s\n",shared_queue.messages[lv_aux]);
      shared_queue.last = ( lv_aux + 1 ) % 200;
      shared_queue.count--;
    }else sleep(10);
  }
  
} 

int main()
{
// Registrar funcion que limpie SHM segment si Ctrl+C
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &Free_Resources;
  sigaction (SIGINT, &sa, NULL);

  pthread_t thread1_id;
  pthread_t thread2_id;

  pthread_create (&thread1_id, NULL, &producer, NULL);

  pthread_create (&thread2_id, NULL, &consumer, NULL);

  /* Make sure the first thread has finished.  */
  pthread_join (thread1_id, NULL);
  /* Make sure the second thread has finished.  */
  pthread_join (thread2_id, NULL);

  /* Now we can safely return.  */
  return 0;
}