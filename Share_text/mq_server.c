#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <string.h>

const char* input = "input.db";

int client_pid;
int* ptr_input;

sem_t* inputClient;


int main(int argc, char **argv)
{
    int fd_input;

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 11*sizeof(char);
    char value[10];

    mqd_t mfd;

    sem_unlink("inputClient");

    if((inputClient = sem_open("inputClient", O_CREAT,0777,0))==NULL){

		perror("Sem Open Error");
		return 1;
	}
    
    //Shared File : Get Client PID
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("Open Error\n");
		return 1;
	}
    
    //INIT
    write(fd_input, "", sizeof(int)*2);

    //Input Server PID
	ptr_input = mmap(NULL,2*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED,fd_input,0);
    ptr_input[0]=getpid();

    //Create Message Queue 
    mfd = mq_open("/my_mq", O_RDWR | O_CREAT,  0666, &attr);
    if (mfd == -1)
    {
            perror("open error");
            exit(0);
    }

    while(1)
    {
        //Wait for Client
        printf("Wait for Client input\n");
        sem_wait(inputClient);
        printf("Reading Server\n");
        if((mq_receive(mfd, (char *)&value, attr.mq_msgsize,NULL)) == -1)
        {
                exit(-1);
        }
        printf("Read Data %d\n", value);
    }
    close(fd_input);
    return 0;
}         