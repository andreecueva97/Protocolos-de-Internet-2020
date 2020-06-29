#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>


struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int producer()
{ 
      struct my_msgbuf buf;

    int msqid;
    key_t key;

    if ((key = ftok("producer.cpp", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    buf.mtype = 1; /* we don't really care in this case */
      while(1)
      {
        fgets(buf.mtext, 200, stdin);
        if (msgsnd(msqid, (struct my_msgbuf *)&buf, sizeof(buf), 0) == -1)
            perror("msgsnd");

      }

    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
return 0;
} 


int main()
{ 
  int fds;
  pid_t pid;

  producer();

  return 0;
} 



