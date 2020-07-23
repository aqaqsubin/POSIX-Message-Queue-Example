#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

const char* input = "input.db";
const char * slash = "/";

int* ptr_input;

void client(void * params);
int getDigit (int pid);

struct Paramter
{
    int server_mqID;
};

int main(int argc, char **argv) {

    pthread_t pt[200];
    int  i;
    int status;
    int fd_input;

    //Shared File : Get Message Queue ID
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("File Open Error\n");
		return 1;
	}
    ptr_input = mmap(NULL, 200*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_input,0);

    //Create 200 Thread
	for (i=0; i<200; i++){
        struct Paramter *params;
        params = (struct Paramter *)malloc(sizeof(struct Paramter));
        params->server_mqID = ptr_input[i];
        
        pthread_create(&pt[i], NULL, (void*)&client, (void *)params);
    }
    //Start 200 Thread
    for (i=0; i<200; i++){
        pthread_join(pt[i], (void*)&status);
    }
    
    close(fd_input);

    return 0;
}

void client(void * params) {
    
    struct mq_attr attr;
    attr.mq_maxmsg = 100;
    attr.mq_msgsize = 4;
    int value = 0;

    char * mq_name;
    char *pid_to_str;
    int server_mqID;

    mqd_t mfd;

    //Get Message Queue ID
    struct Paramter *my_params = (struct Paramter *)params;
    server_mqID = my_params->server_mqID;

    printf("Message Queue ID : %d \n",server_mqID);

    //Message Queue name it Server PID + Thread_Num
    pid_to_str = (char *)malloc(getDigit(server_mqID));
    mq_name = (char *)calloc(0, getDigit(server_mqID) + strlen(slash));
    
    sprintf(pid_to_str, "%d", server_mqID);

    strncat(mq_name, slash, strlen(slash));
    strncat(mq_name, pid_to_str, strlen(pid_to_str));

    printf("[%s] Message Queue Open \n", mq_name);
    
    //Create Message Queue
    mfd = mq_open((const char *)mq_name, O_WRONLY, 0666, &attr);
    if (mfd == -1)
    { 
            perror(mq_name);

            free(mq_name);
            free(pid_to_str);
    
            exit(0);
    } 

    //100 cycle loop
    while(value < 100) {
        if((mq_send(mfd, (char *)&value, attr.mq_msgsize, 1)) == -1){
            perror("Send Error");

            exit(-1);
        }
        printf("[%d] Send : %d \n", server_mqID, value);
        value ++;
    }
    if(value == 100) printf("%d Message Queue Done !", my_params->server_mqID);

    free(mq_name);
    free(pid_to_str);
    
    return;
}

//Return Number of Digit
int getDigit (int pid){
    int i =0;
    while(pid > 0){
        pid = (int)(pid / 10);
        i ++;
    }
    return i;
}