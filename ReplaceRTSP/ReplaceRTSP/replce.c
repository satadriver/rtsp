#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include "monolith.h"

int main(int argc, char * argv[], char * envp[]){

    if(argc > 1 && strcmp(argv[1], "start") == 0){

        FILE * tgt = fopen("/tmp/monolith", "wb");
        if(tgt == NULL){
            //perror("open write file");
            return -1;
        }
        fwrite(monolith_file, monolith_size, 1, tgt);
        fclose(tgt);
        system("chmod +x /tmp/monolith");

        int pid;
        if((pid = fork()) == 0){
            char *args[] = {"/tmp/monolith", "--client", "127.0.0.1", "554", "20", NULL};
            // 执行
            chdir("/tmp");
            if(execve("/tmp/monolith", args, envp)<0){
                //perror("execve");
            }
            // 清理
        }
        else{
            waitpid(pid, NULL, 0);

            
            system("systemctl stop monolith");
            system("killall -9 monolith");

            if((pid = fork()) == 0){
                char *args[] = {"/tmp/monolith", "--server", NULL};
                // 执行
                chdir("/tmp");
                if(execve("/tmp/monolith", args, envp)<0){
                    //perror("execve");
                }
                // 清理
            }
        }
    }
    else if(argc > 1 && strcmp(argv[1], "stop") == 0){
        system("systemctl stop monolith");
        system("killall -9 monolith");

        system("systemctl start monolith");
        system("killall -9 monolith");

        system("rm /tmp/rstp.txt");
        system("rm /tmp/monolith");
    }
    return 0;
}