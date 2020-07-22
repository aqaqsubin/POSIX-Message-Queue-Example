#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <string.h>

const char* input = "input.db";

int server_pid;
int* ptr_input;

sem_t* inputClient;

int main(int argc, char **argv)
{
    int fd_input;

    struct mq_attr attr;
    attr.mq_maxmsg = 11;
    attr.mq_msgsize = 12*sizeof(char);
    char value[10];
    mqd_t mfd;

    sem_unlink("inputClient");


    if((inputClient = sem_open("inputClient", O_CREAT,0777,1))==NULL){

		perror("Sem Open Error");
		return 1;
	}
   

    //Shared File : Get Client PID
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("Open Error\n");
		return 1;
	}

    //Input Server PID
	ptr_input = mmap(NULL,2*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED,fd_input,0);
    ptr_input[1] = getpid();

    mfd = mq_open("/my_mq", O_WRONLY, 0666, &attr);
    if(mfd == -1)
    {
        perror("open error");
        exit(0);
    }
    int err;
    while(1)
    {
        printf("Client input\n");
        scanf("%s", &value);
        printf("%d, %d\n",sizeof(value), attr.mq_msgsize);
        err = mq_send(mfd, (char *)&value, sizeof(value)+1, 1);
        if(err == -1)
        {
            perror("send error");
            exit(-1);
        }else printf("Send Message\n");
        fflush(stdin);
        printf("Client post\n");
        sem_post(inputClient);

    }
    close(fd_input);
    return 0;
}