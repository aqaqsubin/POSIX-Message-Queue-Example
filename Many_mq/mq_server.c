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
int server_pid;

void server(void *params);
int getDigit (int pid);

struct Paramter
{
    int thread_num;
};

int main(int argc, char **argv) {

    pthread_t pt[200];
    int  i;
    int status;
    int fd_input;

    //Shared File : Get Server PID
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("File Open Error\n");
		return 1;
	}
    server_pid = getpid();

    //INIT File
    write(fd_input, "", sizeof(int)*200);
    ptr_input = mmap(NULL, 200*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_input,0);

    //Create 200 Thread
	for (i=0; i<200; i++){

        //Pass Server Thread Number
        struct Paramter *params;
        params = (struct Paramter *)malloc(sizeof(struct Paramter));
        params->thread_num = i;

        pthread_create(&pt[i], NULL, (void*)&server, (void *)params);
    }

    //Start 200 Thread
    for (i=0; i<200; i++){
        pthread_join(pt[i], (void*)&status);
    }

    close(fd_input);

    return 0;
}

void server(void *params){

    struct mq_attr attr;
    attr.mq_maxmsg = 100;
    attr.mq_msgsize = 4;
    int value;

    char *mq_name;
    char *mq_id;

    char *pid_to_str;
    char *thrNum_to_str;
    
    mqd_t mfd;

    //Get Server Thread Number
    struct Paramter *my_params = (struct Paramter *)params;
    
    //Message Queue name it Server PID + Thread Num

    pid_to_str = (char *)malloc(getDigit(server_pid));
    thrNum_to_str = (char *)malloc(getDigit(my_params->thread_num));

    mq_name = (char *)calloc(0, getDigit(server_pid) + getDigit(my_params->thread_num) + strlen(slash));
    mq_id = (char *)calloc(0, getDigit(server_pid) + getDigit(my_params->thread_num));
    
    sprintf(pid_to_str, "%d", server_pid);
    sprintf(thrNum_to_str, "%d", my_params->thread_num);

    //Get Message Queue Name ex) '/dev/mqueue/' + '1525' + '100' = '/dev/mqueue/1525100'
    strncat(mq_name, slash, strlen(slash));
    strncat(mq_name, pid_to_str, strlen(pid_to_str));
    strncat(mq_name, thrNum_to_str, strlen(thrNum_to_str));

    //Get Message Queue ID ex) '1525' + '100' = '1525100'
    strncat(mq_id, pid_to_str, strlen(pid_to_str));
    strncat(mq_id, thrNum_to_str, strlen(thrNum_to_str));

    //Write Message Queue ID
    ptr_input[my_params->thread_num] = atoi(mq_id);

    printf("[%s] Message Queue ID : %d \n", mq_id, atoi(mq_id));
    printf("[%s] Message Queue Open \n", mq_name);
    //Create Message Queue
    mfd = mq_open((const char *)mq_name, O_RDWR | O_CREAT, 0666, &attr);
    if (mfd == -1)
    { 
            perror("Message Queue Open Error");
            mq_close(mfd);
            mq_unlink((const char *)mq_name);
            // exit(0);
            free(mq_name);
            free(pid_to_str);

            return;
    }

    //100 cycle loop
    while(value != 99) {
        // printf("[%s] Receive : %d \n", mq_id, value);
        if((mq_receive(mfd, (char *)&value, attr.mq_msgsize, NULL)) == -1){
            perror("Send Error");

            mq_close(mfd);
            mq_unlink((const char *)mq_name);
            
            free(mq_name);
            free(pid_to_str);
            
            exit(-1);
        }
    }
    printf("%d Thread Done !\n", my_params->thread_num);
    
    mq_close(mfd);
    mq_unlink((const char *)mq_name);
    
    free(mq_name);
    free(pid_to_str);

    return;
}

//Return Number of Digit
int getDigit(int pid){
    int i = 1;

    while(pid=(int)(pid / 10) > 0){
        pid = (int)(pid / 10);
        i ++;
    }
    return i;
}