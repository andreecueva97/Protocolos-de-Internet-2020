#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int consumer()
{
  struct my_msgbuf buf;

    int msqid;
    key_t key;

    if ((key = ftok("producer.cpp", 'B')) == -1) {  /* same key as kirk.c */
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644)) == -1) { /* connect to the queue */
        perror("msgget");
        exit(1);
    }

    while(1) 
    { 
        if (msgrcv(msqid, (struct my_msgbuf *)&buf, sizeof(buf), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        printf("Leido de la cola de mensajes: %s", buf.mtext);
    }
  
} 


int main()
{ 
  int fds;
  pid_t pid;

  consumer ();
  return 0;
} 


