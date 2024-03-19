#ifndef TASK_H
#define TASK_H


#define MAX_info_SIZE 25
struct Task{
    int clientID;
    int taskType;
    char info[MAX_info_SIZE];
};

#endif 