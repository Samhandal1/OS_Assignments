#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"

int pid_counter = 1;

int generatePID(){
    return pid_counter++;
}

int get_pid_counter() {
    return pid_counter;
}

//In this implementation, Pid is the same as file ID 
PCB* makePCB(int start, int end){
    PCB * newPCB = malloc(sizeof(PCB));
    newPCB->pid = generatePID();
    newPCB->PC = start; //changing this for A3
    newPCB->start  = start;
    newPCB->end = end;
    newPCB->job_length_score = 1+end-start;
    newPCB->priority = false;
    return newPCB;
}

PCB* makePCBwithPT(int page_table_info[], int page_table_size, int program_size, char* file_for_init){
    // copy page_table to new array unique to this PCB
    int* page_table = malloc(page_table_size * sizeof(int));
    memcpy(page_table, page_table_info, page_table_size * sizeof(int));
    
    PCB * newPCB = malloc(sizeof(PCB));
    newPCB->pid = generatePID();
    newPCB->PC = 0;
    newPCB->start = 0;
    newPCB->end = program_size-1; // last pc position, counts 0 as 1
    newPCB->job_length_score = program_size;
    newPCB->priority = false;
    newPCB->page_table = page_table;
    newPCB->file_name = file_for_init;
    return newPCB;
}