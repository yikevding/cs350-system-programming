// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#ifndef defs_h
#define KEY 56857 // key for shared memory
#define MAX_PERFECT_NUM 20
#define MAX_PROCESS 20
#define BITMAP_SIZE 1048576 // this is 2^25 /32

typedef struct proc
{
    pid_t pid;
    int tested;
    int skipped;
    int found;
} Process;

typedef struct message
{
    long type;
    int data;
} Message;

typedef struct memory
{
    int bitmap[BITMAP_SIZE];
    int array[MAX_PERFECT_NUM];
    Process rows[MAX_PROCESS];
} Memory;

#endif