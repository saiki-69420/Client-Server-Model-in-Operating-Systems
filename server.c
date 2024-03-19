#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "task.h"

#define SERVER_KEY 12345
#define MAX_MESSAGE_SIZE 256

struct message {
    long mtype;
    char mtext[MAX_MESSAGE_SIZE];
};
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}
void handle_client_request(int read_pipe, int write_pipe) {
    char request[MAX_MESSAGE_SIZE];
    char response[MAX_MESSAGE_SIZE];
    // Read the request from the client
    if (read(read_pipe, request, sizeof(request)) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    int len=strlen(request);
    printf("request: %s\n",request);
    printf("request len: %d\n", len);
    int type=request[strlen(request)-1];
    printf("type: %d\n",type);
    char msg[256];
    strncpy(msg,request,len-1);
    msg[len-1]='\0';
    printf("msg: %s\n",msg);
    int tt=type-48;
    printf("tt: %d\n",tt);
    switch (tt) {
        case 1: // Ping
            printf("ping\n");
            strcpy(response, "hello");
            break;
        case 2: // File Search
            {
                // Use execlp to run grep command
                close(STDOUT_FILENO); // Close standard output
                dup2(write_pipe, STDOUT_FILENO); // Redirect output to the pipe
                execlp("ls", "ls", "|","grep","-c", msg, NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            }
            break;
        case 3: // File Word Count
            {
                // Use execlp to run wc command
                close(STDOUT_FILENO); // Close standard output
                dup2(write_pipe, STDOUT_FILENO); // Redirect output to the pipe
                execlp("wc", "wc", "-w", msg, NULL);
                perror("execlp");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            exit(EXIT_FAILURE);
    }
    if (write(write_pipe, response, sizeof(response)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

int main() {
    printf("sun raha hai\n");
    int msqid;
    struct message msg;
    printf("sun raha hai\n");
    // Create a message queue
    msqid = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("sun raha hai\n");
        // Receive a request from the client via message queue
        if (msgrcv(msqid, &msg, sizeof(msg.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        printf("mtext: %s\n",msg.mtext);
        struct Task task1;
        memcpy(&task1, msg.mtext,sizeof(task1));
        printf("info: %s\n", task1.info);
        char* ttype;
        tostring(ttype, task1.taskType);
        strcat(task1.info, ttype);
        int pipe_to_child[2]; // Pipe from parent to child
        int pipe_to_parent[2]; // Pipe from child to parent

        if (pipe(pipe_to_child) == -1 || pipe(pipe_to_parent) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (child_pid == 0) { // Child process
            close(pipe_to_child[1]); // Close write end
            close(pipe_to_parent[0]); // Close read end
            handle_client_request(pipe_to_child[0], pipe_to_parent[1]);
            exit(EXIT_SUCCESS);
        } else { // Parent process
            close(pipe_to_child[0]); // Close read end
            close(pipe_to_parent[1]); // Close write end

            // Send the request to the child
            if (write(pipe_to_child[1], task1.info, sizeof(task1.info)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            // Receive the response from the child
            char response[MAX_MESSAGE_SIZE];
            ssize_t bytes_read = read(pipe_to_parent[0], response, sizeof(response));
            if (bytes_read == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            } else {
                response[bytes_read] = '\0';
                struct message resp;
                strcpy(resp.mtext, response);
                resp.mtype=msg.mtype;
                printf("Server Response:\n%s\n", response);
                msgsnd(msqid, &resp, sizeof(msg.mtext), 0);
            }

            // Close pipes
            close(pipe_to_child[1]);
            close(pipe_to_parent[0]);

            // Wait for the child to complete
            int status;
            waitpid(child_pid, &status, 0);
        }
    }

    return 0;
}
