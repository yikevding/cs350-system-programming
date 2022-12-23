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

// declarations

Memory *address = NULL;
int queueID;
int smemoryID;
int semaphoreID;
int row_index;
void shutdown();
int is_perfect(int);
int is_checked(int *, int);
void change_bit(int *, int);
int direct_signals()
{
    signal(SIGINT, shutdown);
    signal(SIGQUIT, shutdown);
    signal(SIGHUP, shutdown);
    return 0;
}
// main functions
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Argument usage error.\n");
        perror("error: ");
        exit(1);
    }
    int start = atoi(argv[1]);

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

    printf("Compute process ready to register.\n");
    // construct a message and ask manage for registration number, which is process row index
    Message message;
    message.type = 1;
    int mypid = getpid();
    message.data = mypid;
    int s = msgsnd(queueID, &message, sizeof(message.data), 0);
    if (s == -1)
    {
        printf("Failed to send registration message.\n");
        perror("error:");
        exit(1);
    }
    else if (s == 0)
        printf("Sent compute registration message to manage.\n");

    // use semaphore to lock operation, reserve spot in other words
    struct sembuf buffer;
    buffer.sem_flg = 0;
    buffer.sem_num = 0;
    buffer.sem_op = -1; // lock
    int lock = semop(semaphoreID, &buffer, 1);
    if (lock == -1)
    {
        printf("Failed to lock semaphore.\n");
        perror("error: ");
        exit(1);
    }

    // get process index from manage, only get the message with type matching its pid
    Message r_msg;
    msgrcv(queueID, &r_msg, sizeof(r_msg.data), mypid, 0);
    row_index = r_msg.data;
    // need to rework this part

    if (row_index == -1) // if already 20 compute processes
    {
        exit(0);
    }
    address->rows[row_index].pid = getpid();

    // to calculate perfect number
    for (int i = start; i < BITMAP_SIZE * sizeof(int) * 8; i++)
    {
        if (is_checked(address->bitmap, i))
        {
            address->rows[row_index].skipped++;
        }
        else
        {
            change_bit(address->bitmap, i);
            address->rows[row_index].tested++;
            if (is_perfect(i))
            {
                address->rows[row_index].found++;
                Message report;
                report.type = 2; // type 2 means report perfect number
                report.data = i;
                msgsnd(queueID, &report, sizeof(report.data), 0);
            }
        }
    }
    // after get to the end, go all the way to 0 and stop at the starting point, if reached call execlp and shut down

    for (int i = 0; i < start; i++)
    {
        if (is_checked(address->bitmap, i))
        {
            address->rows[row_index].skipped++;
        }
        else
        {
            change_bit(address->bitmap, i);
            address->rows[row_index].tested++;
            if (is_perfect(i))
            {
                address->rows[row_index].found++;
                Message report;
                report.type = 2; // type 2 means report perfect number
                report.data = i;
                msgsnd(queueID, &report, sizeof(report.data), 0);
            }
        }
    }

    // after testing everything call report -k
    execl("report", "report", "-k", NULL);
}

// implement how compute shut down everything it has
void shutdown()
{
    address->rows[row_index].pid = 0; // delete entry from shared memory
    // memory address, detached
    int addressid = shmdt(address);
    if (addressid == -1)
    {
        printf("Failed to deallocate address.\n");
        perror("error: ");
        exit(1);
    }
    exit(0);
}

// implement how to check perfect number
int is_perfect(int target)
{
    if (target <= 0)
        return 0;
    int sum = 0;
    for (int i = 1; i < target; i++)
    {
        if (target % i == 0)
        {
            sum += i;
        }
    }
    if (sum == target)
        return 1;
    else
        return 0;
}

// implement how to check individual bit
int is_checked(int *map, int i)
{
    int block_index = i / 32;
    int bit_index = i % 32;
    return map[block_index] & (1 << bit_index);
}

// implement how to change bit
void change_bit(int *map, int i)
{
    int block_index = i / 32;
    int bit_index = i % 32;
    map[block_index] |= (1 << bit_index);
}