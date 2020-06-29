#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>

typedef char shm_message[200]; //tamano de cada mensaje

struct shm_segment
{
   int first, last, count;
   shm_message messages[1024];//hay espacio hasta 1024 mensajes
};

int segment_id;
struct shm_segment *shared_memory;

int flag = 1;
void Free_SHM (int signal_number)//permite liberar la sharememory
{
//  shmdt (shared_memory);
  /* Deallocate the shared memory segment.  */
  shmctl (segment_id, IPC_RMID, 0);
  flag = 0;
}

void producer()
{ 
  printf("----------------producer - servidor----------\n");
// Registrar funcion que limpie SHM segment si Ctrl+C--------------
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &Free_SHM;
  sigaction (SIGINT, &sa, NULL);
//-----------------------------------------------------------------
  struct shmid_ds shmbuffer;
  int segment_size;
  const int shared_segment_size = 0x3200C;
  key_t lv_key;//llave, es un identificador que tiene que tiene que ser igual de los dos lados

  lv_key = ftok("producer.cpp",'B');//toma el nombre y genera la llave
  /* Allocate a shared memory segment.  */
  segment_id = shmget (lv_key, shared_segment_size,//variable que usamos para leer y escribir el mensaje dentro del segmento
		       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  //IPC_CREAT crea el segmento
  //IPC_EXCL asegura que el segmento se cree
  //S_IRUSR lectura
  //S_IWUSR escritura  
  /* Attach the shared memory segment.  */
  shared_memory = (struct shm_segment*) shmat (segment_id, 0, 0);//coloca el mensaje dentro del segmento
  printf ("shared memory attached at address %p\n", shared_memory);//imprime a direccion de memoria compatida

  shared_memory->first = 0;
  shared_memory->count = 0;
  shared_memory->last = 1;

      while(flag)
      {
        if ( shared_memory->count < 200 )//deberia ser 1024 porque es la cantidad de mensajes que puede tomar
        {
          shared_memory->first = ( shared_memory->first + 1 ) % 200;//hace el movimiento necesario para pasar al otro mensaje [0]->[201]->[401]
          int lv_aux = shared_memory->first;//ahora que se movio al siguiente mensaje lo guardo en un aux.
          printf("ingrese texto a enviar:");
          fgets(shared_memory->messages[lv_aux], 200, stdin);//toma mensaje por entrada de teclado
          shared_memory->count++;//mensaje agregado entonces suma +1
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

