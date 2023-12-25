#ifndef PCB_H
#define PCB_H
#include <stdbool.h>
/*
 * Struct:  PCB 
 * --------------------
 * pid: process(task) id
 * PC: number of line it is executing (i.e. start at 0, move up), also current index in page_table // used to be program counter, stores the index of line that the task is executing
 * start: the first line in shell memory that belongs to this task
 * end: the last line in shell memory that belongs to this task
 * job_length_score: for EXEC AGING use only, stores the job length score
 */
typedef struct
{
    bool priority;
    int pid;
    int PC;
    int start;
    int end;
    int job_length_score;
    int *page_table; 
    char *file_name;
}PCB;

int generatePID();
int get_pid_counter();
PCB * makePCB(int start, int end);
PCB * makePCBwithPT(int page_table_info[], int page_table_size, int program_size, char* file_for_init);
#endif