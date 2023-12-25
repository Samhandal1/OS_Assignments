#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
#include "pcb.h"
extern int FRAME_MEM_SIZE;
extern int VAR_MEM_SIZE;
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
int load_file(FILE* fp, int* pStart, int* pEnd, char* fileID);
char * mem_get_value_at_line(int index);
char * mem_get_var_at_line(int index);
void mem_free_lines_between(int start, int end);
void printShellMemory();
void mem_set_value_store(char *var_in, char *value_in);
int load_a_single_frame(FILE* fp, int target_line_num, int pid);
int load_frame_store(FILE* fp, int page_table[], char* filename, int pt_size);
void addPCBtoframelist(PCB *pcb, int num_frames);
void updatePCBframelist(PCB *pcb, int frame_number);
void ageFrames(int frame_number);
void toFrameInUse(int frame_number);
void toFrameNotInUse(int frame_number);
#endif