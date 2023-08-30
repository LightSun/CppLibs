#include "CmdRunner.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

using namespace std;

#define READ   0
#define WRITE  1

FILE* __popen2(const char* cmd, int write, int& pid){
    pid_t child_pid;
    int fd[2];
    if(pipe(fd) == -1){
        return NULL;
    }

    if((child_pid = fork()) == -1)
    {
        perror("fork");
        return NULL;
    }

    /* child process */
    if (child_pid == 0)
    {
        if (write == 0) //read
        {
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
        }
        else //write
        {
            close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0);   //Redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
        int ret = execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
        //execl(cmd, cmd, (char*)NULL);
        exit(0);
        //_exit(0);
    }
    else
    {
        if (write == 0)
        {
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else
        {
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (write == 0)
    {
        return fdopen(fd[READ], "r");
    }
    return fdopen(fd[WRITE], "w");
}

int __pclose2(FILE* fp, int pid){
    int  stat;

    //fd = fileno(fp);
    if(pid == 0)
        return (-1); //fp wasn't opened by popen()

    if(fclose(fp) == EOF)
        return (-1);
    if( kill(-pid, SIGKILL) != 0) {
        fprintf(stderr, "kill failed\n");
        perror("kill");
    }
    else {
        printf("pid = %d was killed\n", pid);
    }
    while(waitpid(pid, &stat, 0) < 0) {
        if(errno != EINTR )
            return (-1); /*error other than EINTR from waitpid()*/
    }
    return stat;
}
int __pread2(FILE* fp, char* buf, unsigned int buf_size){
    return read(fileno(fp), buf, buf_size);
    //auto ret = fgets(buf, buf_size, fp);
    //reutrn ret != NULL ? 1 : 0;
}
