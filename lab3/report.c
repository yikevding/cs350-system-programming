// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <setjmp.h>
#include <string.h>
#include "defs.h"

Memory *address = NULL;
int queueID;
int smemoryID;
int main(int argc, char *argv[])
{
    int kflag = 0;
    int option;
    while ((option = getopt(argc, argv, "k")) != -1)
    {
        switch (option)
        {
        case 'k':
        {
            kflag++;
            break;
        }
        }
    }
    // create message queue
    queueID = msgget(KEY, IPC_CREAT | 0660);
    if (queueID == -1)
    {
        printf("Failed to get message queue.\n");
        perror("error: ");
        exit(1);
    }

    // create shared memory and map into address space
    size_t memory_size = sizeof(Memory);
    smemoryID = shmget(KEY, memory_size, IPC_CREAT | 0660);
    if (smemoryID == -1)
    {
        printf("Failed to get shared memory segment.\n");
        perror("error");
        exit(1);
    }
    address = shmat(smemoryID, 0, 0);
    if (address == (Memory *)-1)
    {
        printf("Failed to map address.\n");
        perror("error: ");
        exit(1);
    }
    // print out current statistics
    printf("Perfect Number Found: ");
    for (int i = 0; i < 20; i++)
    {
        if (address->array[i] == 0)
            break;
        else
            printf("%d ", address->array[i]);
    }
    printf("\n");
    int total_found = 0;
    int total_tested = 0;
    int total_skipped = 0;
    for (int i = 0; i < 20; i++)
    {
        if (address->rows[i].pid == 0)
            break;
        else
        {
            pid_t pid = address->rows[i].pid;
            int found = address->rows[i].found;
            int tested = address->rows[i].tested;
            int skipped = address->rows[i].skipped;
            printf("pid(%d): found: %d, tested: %d, skipped: %d ", pid, found, tested, skipped);
            total_found += found;
            total_tested += tested;
            total_skipped += skipped;
            printf("\n");
        }
    }
    printf("Statistics:\n");
    printf("Total found: %d\n", total_found);
    printf("Total tested: %d\n", total_tested);
    printf("Total skipped: %d\n", total_skipped);

    // k argument present, shutdown everything
    if (kflag == 1)
    {
        Message close_message;
        close_message.type = 3;
        close_message.data = -1;
        msgsnd(queueID, &close_message, sizeof(close_message.data), 0);

        // memory address, detached
        int addressid = shmdt(address);
        if (addressid == -1)
        {
            printf("Failed to deallocate address.\n");
            perror("error: ");
            exit(1);
        }
    }

    exit(0);
}