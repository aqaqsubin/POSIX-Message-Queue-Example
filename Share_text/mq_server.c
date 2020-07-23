#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>

const char* input = "input.db";
const char * slash = "/";

char * mq_name;
int* ptr_input;

int getDigit (int pid);

int main(int argc, char **argv)
{
    int fd_input;

    struct mq_attr attr;
    attr.mq_maxmsg = 10; // Size of Message Queue
    attr.mq_msgsize = 21 * sizeof(char); // Size of Message

    char value[20];
    char *pid_to_str;

    mqd_t mfd;
    
    //Shared File : Share Server PID
    if((fd_input = open(input,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
		perror("Open Error\n");
		return 1;
	}
    
    //INIT
    write(fd_input, "", sizeof(int)*1);

    //Input Server PID
	ptr_input = mmap(NULL,1*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED,fd_input,0);
    ptr_input[0]=getpid();

    //Message Queue name it Server PID
    pid_to_str = (char *)malloc(sizeof(char)*getDigit(getpid()));
    mq_name = (char *)calloc(0, sizeof(pid_to_str)+1);
    sprintf(pid_to_str, "%d", getpid());

    strncat(mq_name, slash, 1);
    strncat(mq_name, pid_to_str, sizeof(pid_to_str));

    close(fd_input);

    //Create Message Queue 
    mfd = mq_open((const char *)mq_name, O_RDWR | O_CREAT,  0666, &attr);
    if (mfd == -1)
    {
            perror("Open Error");
            exit(0);
    }

    while(1)
    {
        if((mq_receive(mfd, (char *)&value, attr.mq_msgsize,NULL)) == -1)
        {
                exit(-1);
        }
        printf("Read Data : %s\n", value);
    }

    free(mq_name);
    free(pid_to_str);

    return 0;
}
int getDigit (int pid){
    int i =0;
    while(pid > 0){
        pid = (int)(pid / 10);
        i ++;
    }
    return i;
}         