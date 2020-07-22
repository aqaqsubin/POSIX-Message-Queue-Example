#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>

const char* input = "input.db";
int* ptr_input;

void server(void *params);

struct Paramter
{
    int thread_num;
};

int main(int argc, char **argv) {
    pthread_t pt[200];
    int  i;
    int status;
    int fd_input;
    struct Paramter *parms;
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("File Open Error\n");
		return 1;
	}
    write(fd_input, "", sizeof(int)*200);
    ptr_input = mmap(NULL, 200*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_input,0);

    // create thread
	for (i=0; i<200; i++){
        struct Paramter *params;
        params = (struct Paramter *)malloc(sizeof(struct Paramter));
        params->thread_num = i;

        pthread_create(&pt[i], NULL, (void*)&server, (void *)params);
    }
    for (i=0; i<200; i++){
        pthread_join(pt[i], (void*)&status);
    }
    close(fd_input);
    return 0;
}

void server(void *params){

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 4;
    int value;
    
    mqd_t mfd;

    struct Paramter *my_params = (struct Paramter *)params;
    ptr_input[my_params->thread_num] = getpid();

    char *mq_name;
    mq_name = (char *)malloc(sizeof(char)*getDigit(getpid()));
    sprintf(mq_name, "%d", ptr_input[my_params->thread_num]);

    mfd = mq_open((const char *)mq_name, O_RDWR | O_CREAT, 0666, &attr);
    if (mfd == -1)
    { 
            perror("MQ Open Error");
            exit(0);
    } 

    while(1) {
        if((mq_receive(mfd, (char *)&value, attr.mq_msgsize, NULL)) == -1){
            exit(-1);
        }
        // printf("Read Data %d\n", value);
    }
    return;
}

int getDigit(int pid){
    int i = 0;
    while(pid > 0){
        pid = (int)(pid / 10);
        i ++;
    }
    return i;
}