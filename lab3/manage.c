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
int semaphoreID;
void shutdown();
int direct_signals()
{
    signal(SIGINT, shutdown);
    signal(SIGQUIT, shutdown);
    signal(SIGHUP, shutdown);
    return 0;
}
int main(int argc, char *argv[])
{

    // handle signals
    int status = direct_signals();
    if (status != 0)
    {
        printf("Failed to handle signals.\n");
        perror("error: ");
        exit(1);
    }

    // create message queue
    queueID = msgget(KEY, IPC_CREAT | 0660);
    if (queueID == -1)
    {
        printf("Failed to get message queue.\n");
        perror("error: ");
        exit(1);
    }

    // create semaphore
    semaphoreID = semget(KEY, 1, IPC_CREAT | 0660);
    if (semaphoreID == -1)
    {
        printf("Failed to get semaphore.\n");
        perror("error: ");
        exit(1);
    }

    // create shared memory and map into address space
    size_t memory_size = sizeof(Memory);
    smemoryID = shmget(KEY, memory_size, IPC_CREAT | 0660);
    if (smemoryID == -1)
    {
        printf("Failed to get shared memory segment.\n");
        perror("error: ");
        exit(1);
    }
    address = shmat(smemoryID, 0, 0);
    if (address == (Memory *)-1)
    {
        printf("Failed to map address.\n");
        perror("error: ");
        exit(1);
    }

    // initialize shard memory ,bitmap, perfect number array, process rows
    memset(address, 0, sizeof(Memory));
    for (int i = 0; i < BITMAP_SIZE; i++)
        address->bitmap[i] = 0;
    for (int i = 0; i < MAX_PERFECT_NUM; i++)
        address->array[i] = 0;
    for (int i = 0; i < MAX_PROCESS; i++)
        address->rows[i].pid = 0;

    // setup semaphore
    // compute blocks a semaphore, after initialize, unlock for future use
    struct sembuf buffer;
    buffer.sem_flg = 0;
    buffer.sem_num = 0;
    buffer.sem_op = 1; // unlock

    // read messages
    Message message;
    int current = 0; // current process row index
    int total = 0;   // total perfect numbers in perfect array

    printf("Manage finishes set up, waiting for messages.\n");
    while (1)
    {
        status = msgrcv(queueID, &message, sizeof(message.data), -3, 0);
        if (status == -1)
        {
            printf("Failed to get messages in manage.\n");
            perror("error:");
            exit(1);
        }
        if (message.type == 3) // signal to terminate everything
        {
            printf("Manage shut down everything.\n");
            shutdown();
            break;
        }
        else if (message.type == 1) // register new process
        {
            Message s_msg;
            s_msg.type = message.data;
            printf("Manage got register message.\n");
            while (current < 20)
            {
                if (address->rows[current].pid == 0) // empty slot
                {
                    s_msg.data = current;
                    break;
                }
                else
                {
                    current++;
                }
            }
            if (current >= 20)
            {
                s_msg.data = -1; // this tells the compute process to terminate, because no more than 20 allowed
            }
            else
            {
                address->rows[current].pid = message.data;
                address->rows[current].tested = 0;
                address->rows[current].skipped = 0;
                address->rows[current].found = 0;
                current++; // increase current empty slot index
            }
            msgsnd(queueID, &s_msg, sizeof(s_msg.data), 0);
            int unlock = semop(semaphoreID, &buffer, 1);
            if (unlock == -1)
            {
                printf("Failed to unlock semaphore.\n");
                perror("error: ");
                exit(1);
            }
        }
        else if (message.type == 2) // find correct spot of perfect number array
        {
            int repeated = 0;
            int candidate = message.data;
            printf("Manage got perfect number %i\n", candidate);
            if (address->array[0] == 0) // if the array is empty
            {
                address->array[0] = candidate;
                total++;
                continue;
            }
            for (int i = 0; i < 20; i++) // check if the array already contains candidate perfect number
            {
                if (address->array[i] == 0)
                    break;
                if (address->array[i] == candidate)
                {
                    repeated = 1;
                    break;
                }
            }

            if (repeated == 0) // if not we will put it in the correct spot, sorted in ascending order
            {
                int j = total - 1;
                while (j >= 0)
                {
                    if (candidate > address->array[j])
                        break;
                    else
                    {
                        address->array[j + 1] = address->array[j];
                        j--;
                    }
                }
                address->array[j + 1] = candidate;
                total++;
            }
        }
    }
    exit(0);
}

// shutdown all process
void shutdown()
{
    // send kill signals
    for (int i = 0; i < MAX_PROCESS; i++)
    {
        if (address->rows[i].pid == 0)
            continue;
        else
        {
            pid_t pid = address->rows[i].pid;
            int kill_status = kill(pid, SIGINT);
            if (kill_status != 0)
            {
                printf("Failed to send kill signal.\n");
                perror("error: ");
                exit(1);
            }
        }
    }

    // sleep
    sleep(5);

    // clean up, deallocate resource

    // message queue
    int mid = msgctl(queueID, IPC_RMID, 0);
    if (mid == -1)
    {
        printf("Failed to deallocate message queue.\n");
        perror("error: ");
        exit(1);
    }

    // semaphore
    int semid = semctl(semaphoreID, 0, IPC_RMID, 0);
    if (semid == -1)
    {
        printf("Failed to deallocate semaphore.\n");
        perror("error: ");
        exit(1);
    }

    // memory address, detached
    int addressid = shmdt(address);
    if (addressid == -1)
    {
        printf("Failed to deallocate address.\n");
        perror("error: ");
        exit(1);
    }

    // remove shared segment
    int segid = shmctl(smemoryID, IPC_RMID, 0);
    if (segid == -1)
    {
        printf("Failed to deallocate shared segment.\n");
        perror("error: ");
        exit(1);
    }
}