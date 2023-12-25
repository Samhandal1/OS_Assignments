#ifndef KERNEL
#define KERNEL
#include "pcb.h"
int process_initialize(char *filename);
int schedule_by_policy(char* policy, bool mt);
int shell_process_initialize();
void ready_queue_destroy();
void threads_terminate();
int backing_store_init();
int backing_store_delete();
int copy_to_backing_store(char* filename);
int initialize_process_from_backing_store(char* file_for_init);
int delete_backing_store_files();
#endif