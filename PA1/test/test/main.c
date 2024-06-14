#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

//fprintf(stderr,"error");



int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <current depth> <max depth> <left-right>\n", argv[0]); //arguments
        return 1;
    }

    int curDepth = atoi(argv[1]);
    int maxDepth = atoi(argv[2]);
    int lr = atoi(argv[3]);

    int num1;
    int num2 = 1;

    for(int i = 0; i < curDepth; i++ ){ //printing part
            printf("---");
        }

    printf("> current depth: %d , lr : %d\n", curDepth, lr);

    if (curDepth == 0) {
        printf("Please enter num1 for the root: ");
            scanf("%d", &num1);
    }
    for(int i = 0; i < curDepth; i++ ){
        printf("---");
        }
    printf("> my num1 is : %d\n", num1);

    //* --------------------------------------------- Main Part -------------------------------------------------

    if(curDepth < maxDepth){

        char curDepthStr[10], maxDepthStr[10], lrStr[10];
            sprintf(curDepthStr, "%d", curDepth + 1);
            sprintf(maxDepthStr, "%d", maxDepth);
            sprintf(lrStr, "%d", 0);

        //! ********************** Pipe Part ***************

        int firstPipe[2];
        if(pipe(firstPipe)<0){
            perror("Pipe failed");
            exit(1);
        }

        int secondPipe[2];
        if(pipe(secondPipe)<0){
            perror("Pipe failed");
            exit(1);
        }


        //write(fd[1], &num1, sizeof(num1));
        //close(fd[0]);
        //dup2(fd[1],STDOUT_FILENO);
        //char num1Str[12]; // Enough to hold integer converted to string
        //sprintf(num1Str, "%d", num1);
        
        int pid = fork(); //! ****************** first Fork ********************



        if(pid == 0) { //? ------------------------ Left child ----------------------------------
            
            close(firstPipe[1]); //* first close write because we will read here
            dup2(firstPipe[0], STDIN_FILENO);
            close(firstPipe[0]);
            printf("child");
            
            
            //int receivedNum1;
            //scanf("%d", &receivedNum1);
            //fprintf(stderr,"Received num1: %d\n", receivedNum1);
            
            

            char *args[] = {"./treePipe",curDepthStr, maxDepthStr, lrStr,NULL};
            execvp(args[0], args);
            
        }
        
        else if(pid > 0) { //? ------------------------- Parent --------------------------

            close(firstPipe[0]);

            //dup2(firstPipe[1], STDOUT_FILENO);
            dprintf(firstPipe[1], "%d", num1);

            close(firstPipe[1]);

            
            wait(NULL);

            for(int i = 0;i< curDepth; i++){
                printf("---");
            }
            
            printf("> current depth: %d, lr: %d, my num1: %d, my num2: %d\n",curDepth,lr,num1,num2);

            for(int i = 0; i < curDepth; i++){
                printf("---");
            }
            printf("> my result is: %d\n",num1);


            int pid2 = fork(); //! *************** second fork ****************


            if (pid2 == 0) { //? ---------------------- Right child ---------------------------------

                


                char *args[] = {"./treePipe",curDepthStr, maxDepthStr, lrStr,NULL};
                execvp(args[0], args);
            }
            
            else {
                wait(NULL);
                if(curDepth == 0) {
                    
                    printf("The final result is: %d\n", num1);
                }
            }
        }
        else {
            
            perror("fork");
            exit(EXIT_FAILURE);
        }

    }
    else{
        for(int i = 0; i < curDepth; i++){
            printf("---");
        }
        printf("> my result is: %d\n",num1);
    }

    return 0;
}