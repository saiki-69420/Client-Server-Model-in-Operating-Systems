#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "task.h"

#define SERVER_KEY 12345
#define MAX_MESSAGE_SIZE 256


struct message {
    long mtype;
    char mtext[MAX_MESSAGE_SIZE];
};

int main() {
    int client_id;
    printf("Enter Client-ID: ");
    scanf("%d", &client_id);

    // Create a message queue
    int msqid = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct message msg;
    msg.mtype = client_id;
    struct Task task;
    task.clientID=client_id;
    while (1) {
        printf("\nMenu:\n");
        printf("1. Enter 1 to contact the Ping Server\n");
        printf("2. Enter 2 to contact the File Search Server\n");
        printf("3. Enter 3 to contact the File Word Count Server\n");
        printf("4. Enter 4 to exit\n");

        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                task.taskType=1;
                strcpy(task.info,"hi");
                memcpy(msg.mtext, &task, sizeof(task)); // '1' for Ping
                break;
            case 2:
                printf("Enter filename: ");
                task.taskType=2;
                scanf("%s", task.info);
                memcpy(msg.mtext, &task, sizeof(task));
                break;
            case 3:
                printf("Enter filename: ");
                task.taskType=3;
                scanf("%s", task.info);
                memcpy(msg.mtext, &task, sizeof(task)); // '1' for Ping
                break;
            case 4:
                printf("Exiting client %d...\n", client_id);
                msgctl(msqid, IPC_RMID, NULL); // Remove message queue
                exit(EXIT_SUCCESS);
            default:
                printf("Invalid choice. Try again.\n");
                continue;
        }

        // Send the request to the server
        if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }

        // Receive and display the server's response
        if (msgrcv(msqid, &msg, sizeof(msg.mtext), client_id, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        printf("Server Response:\n%s\n", msg.mtext);
    }

    return 0;
}
