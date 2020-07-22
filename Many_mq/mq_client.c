#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const char* input = "input.db";
int* ptr_input;

void client(void * params);

struct Paramter
{
    int server_pid;
};

int main(int argc, char **argv) {

    pthread_t pt[200];
    int  i;
    int status;
    int fd_input;

    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("File Open Error\n");
		return 1;
	}
    ptr_input = mmap(NULL, 200*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_input,0);

    // create thread
	for (i=0; i<200; i++){
        struct Paramter *params;
        params = (struct Paramter *)malloc(sizeof(struct Paramter));
        params->server_pid = ptr_input[i];
        
        pthread_create(&pt[i], NULL, (void*)&client, (void *)params);
    }
    for (i=0; i<200; i++){
        pthread_join(pt[i], (void*)&status);
    }
    close(fd_input);
    return 0;
}

void client(void * params) {
    
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 4;
    int value = 0;

    mqd_t mfd;

    struct Paramter *my_params = (struct Paramter *)params;
    char mq_name[10];
    sprintf(mq_name, "%d", ptr_input[my_params->server_pid]);
    mfd = mq_open((const char *)mq_name, O_WRONLY, 0666, &attr);
    if (mfd == -1)
    { 
            perror("MQ Open Error");
            exit(0);
    } 

    while(1) {
        if((mq_send(mfd, (char *)&value, attr.mq_msgsize, 1)) == -1){
            perror("Send Error");
            exit(-1);
        }
        sleep(1);
        value ++;
        // printf("Send Data %d\n", value);
    }
    return;
}