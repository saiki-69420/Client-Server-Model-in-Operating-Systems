#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>

#define SERVER_KEY 12345
#define MAX_MESSAGE_SIZE 256

struct message {
    long mtype;
    char mtext[MAX_MESSAGE_SIZE];
};

int main() {
    // Create a message queue
    int msqid = msgget(SERVER_KEY, 0);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Do you want the server to terminate? Press Y for Yes and N for No: ");
        char choice;
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y') {
            // Inform the main server to terminate
            struct message msg;
            msg.mtype = 1; // Use mtype 1 for termination message
            strcpy(msg.mtext, "terminate");
            if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            printf("Termination request sent to the main server.\n");
            break; // Exit the cleanup process
        } else if (choice == 'N' || choice == 'n') {
            // Continue running as usual
            continue;
        } else {
            printf("Invalid choice. Please enter Y or N.\n");
        }
    }

    return 0;
}
