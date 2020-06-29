#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <sys/shm.h>
#include <sys/stat.h>

typedef char shm_message[200]; //tamano de cada mensaje

struct shm_segment
{
   int first, last, count;
   shm_message messages[1024];//hay espacio hasta 1024 mensajes
};

void consumer()
{
  printf("----------------consumer - espia----------\n");
  printf("ESPIA DE LA CONVERSACION ENTRE CLIENTE Y SERVIRDOR\n");
  int segment_id;
  struct shm_segment *shared_memory;
  struct shmid_ds shmbuffer;
  int segment_size;
  const int shared_segment_size = 0x3200C;//toma tamano en hexadecimal
  key_t lv_key;//llave, es un identificador que tiene que ser igual de los dos lados

  lv_key = ftok("producer.cpp",'B');//toma el nombre y genera la llave

  /* Allocate a shared memory segment.  */
  segment_id = shmget (lv_key, shared_segment_size,
		       IPC_EXCL | S_IRUSR | S_IWUSR);//flags asignados

  /* Attach the shared memory segment.  */
  shared_memory = (struct shm_segment*) shmat (segment_id, 0, 0);
  printf ("shared memory attached at address %p\n", shared_memory);

  while (1)
  {
    if ( shared_memory->count > 0 )//va revisando si hay mensajes y los va descontando
    {
      int lv_aux = shared_memory->last;//toma hasta el ultimo caracter del sharememory
      printf("Leido de shm: %s\n",shared_memory->messages[lv_aux]);
      shared_memory->last = ( lv_aux + 1 ) % 200;//va al leer el sigueinte y asi sucesivamente
      shared_memory->count--;
    }else sleep(10);//espera un rato, si no hay nada para leer
  }
} 


int main()
{ 
  int fds;
  pid_t pid;

  consumer ();
  return 0;
} 

