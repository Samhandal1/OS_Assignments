#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include "pcb.h"
#include "kernel.h"
#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"
#include "ready_queue.h"

bool multi_threading = false;
pthread_t worker1;
pthread_t worker2;
bool active = false;
bool debug = false;
bool in_background = false; // Not used for A3
pthread_mutex_t queue_lock;
char* backing_store_dir = "backing_store";

void lock_queue(){
    if(multi_threading) pthread_mutex_lock(&queue_lock);
}

void unlock_queue(){
    if(multi_threading) pthread_mutex_unlock(&queue_lock);
}

int process_initialize(char *filename){
    FILE* fp;
    int error_code = 0;
    int* start = (int*)malloc(sizeof(int));
    int* end = (int*)malloc(sizeof(int));
    
    fp = fopen(filename, "rt");
    if(fp == NULL){
        error_code = 11; // 11 is the error code for file does not exist
        return error_code;
    }
    error_code = load_file(fp, start, end, filename);
    if(error_code != 0){
        fclose(fp);
        return error_code;
    }
    PCB* newPCB = makePCB(*start,*end);
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = newPCB;
    lock_queue();
    ready_queue_add_to_tail(node);
    unlock_queue();
    fclose(fp);
    return error_code;
}


// Load script from backing store into frame store and create & add PCB's to ready queue
int initialize_process_from_backing_store(char* file_for_init){
    int error_code = 0;
    DIR* backing_store;
    struct dirent* script;
    int num_lines;
    
    // Open backing store to access scripts
    my_cd(backing_store_dir);
    backing_store = opendir(".");
    if (backing_store == NULL) {
        return 11;
    }

    while ((script = readdir(backing_store)) != NULL) {
        // Each script has a corresponding PCB that is created with these pointers
        FILE* fp;
        char* filename = script->d_name;
        if (strcmp(file_for_init, filename)!= 0) continue; // Find the given script
        num_lines = 0;        

        //printf("adding pcb & adding to frame store: %s\n", filename);
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
            continue;
        }
        fp = fopen(filename, "rt");
        if (fp == NULL) {
            printf("Unable to open file");
            continue;
        }
        // Count the number of frames needed to load the script
        char buffer[1000];
        while (fgets(buffer, sizeof(buffer), fp)) {
            num_lines++;
        }
        memset(buffer, 0, sizeof(buffer)); // clear buffer
        rewind(fp);
        // this is just taking the ciling of num_lines / 3 i think
        int num_frames = 0;
        if (num_lines%3 != 0) {
            num_frames = 1;
        }
        num_frames += num_lines/3;
        // Allocate memory for the page table, start, end for PCB
        int page_table[num_frames]; 
        // Function sets start, end, page_table pointers and loads script into frame store
        error_code = load_frame_store(fp, page_table, filename, num_frames);  
        if(error_code != 0){
            fclose(fp);
            return error_code;
        }

        //Create PCB for each script & add to ready queue
        PCB* newPCB = makePCBwithPT(page_table, num_frames, num_lines, file_for_init); // why is this not working
        addPCBtoframelist(newPCB, num_frames);
        QueueNode *node = malloc(sizeof(QueueNode));
        node->pcb = newPCB;
        lock_queue();
        ready_queue_add_to_tail(node);
        unlock_queue();
        fclose(fp);
        // Free pointers, need to be used for next script
        // free(fp);, i think fclose does it automatically
        memset(page_table, 0, sizeof(page_table)); 
        // free(page_table); int *array must be freed, int array[] not
    }

    closedir(backing_store);
    my_cd("..");
    return error_code;

}


int page_fault(PCB *pcb, int frame_num){
    int error_code = 0;
    DIR* backing_store;
    struct dirent* script;
    char *file_for_init = pcb->file_name;
    
    // Open backing store to access scripts
    my_cd(backing_store_dir);
    backing_store = opendir(".");
    if (backing_store == NULL) {
        return 11;
    }

    while ((script = readdir(backing_store)) != NULL) {
        // Each script has a corresponding PCB that is created with these pointers
        FILE* fp;
        char* filename = script->d_name;
        if (strcmp(file_for_init, filename)!= 0) continue; // Find the given script

        //printf("adding pcb & adding to frame store: %s\n", filename);
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
            continue;
        }
        fp = fopen(filename, "rt");
        if (fp == NULL) {
            printf("Unable to open file");
            continue;
        }

        rewind(fp);
        int new_frame_store = load_a_single_frame(fp, (frame_num * 3), pcb->pid);
        pcb->page_table[frame_num] = new_frame_store;
        updatePCBframelist(pcb, new_frame_store);
        rewind(fp);
        fclose(fp);
    }

    closedir(backing_store);
    my_cd("..");
    return error_code;
}

// // Load scripts from backing store into frame store and create & add PCB's to ready queue
// int initialize_processes_from_backing_store_OLD(){
//     int error_code = 0;
//     DIR* backing_store;
//     struct dirent* script;
//     int num_lines;
    
//     // Open backing store to access scripts
//     my_cd(backing_store_dir);
//     backing_store = opendir(".");
//     if (backing_store == NULL) {
//         return 11;
//     }

//     while ((script = readdir(backing_store)) != NULL) {
//         //NEED TO CHECK THAT SCRIPT IS NOT ALREADY IN FRAME STORE

//         // Each script has a corresponding PCB that is created with these pointers
//         FILE* fp;
//         num_lines = 0;        
//         char* filename = script->d_name;
//         //printf("adding pcb & adding to frame store: %s\n", filename);
//         if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
//             continue;
//         }
//         fp = fopen(filename, "rt");
//         if (fp == NULL) {
//             printf("Unable to open file");
//             continue;
//         }
//         // Count the number of frames needed to load the script
//         char buffer[1000];
//         while (fgets(buffer, sizeof(buffer), fp)) {
//             num_lines++;
//         }
//         memset(buffer, 0, sizeof(buffer)); // clear buffer
//         rewind(fp);
//         // this is just taking the ciling of num_lines / 3 i think
//         int num_frames = 0;
//         if (num_lines%3 != 0) {
//             num_frames = 1;
//         }
//         num_frames += num_lines/3;
//         // Allocate memory for the page table, start, end for PCB
//         int page_table[num_frames]; 
//         // Function sets start, end, page_table pointers and loads script into frame store
//         error_code = load_frame_store(fp, page_table, filename);  
//         if(error_code != 0){
//             fclose(fp);
//             return error_code;
//         }

//         //Create PCB for each script & add to ready queue
//         PCB* newPCB = makePCBwithPT(page_table, num_frames, num_lines); // why is this not working
//         QueueNode *node = malloc(sizeof(QueueNode));
//         node->pcb = newPCB;
//         lock_queue();
//         ready_queue_add_to_tail(node);
//         unlock_queue();
//         fclose(fp);
//         // Free pointers, need to be used for next script
//        // free(fp);, i think fclose does it automatically
//         memset(page_table, 0, sizeof(page_table)); 
//        // free(page_table); int *array must be freed, int array[] not
//     }

//     closedir(backing_store);
//     my_cd("..");
//     return error_code;

// }


int shell_process_initialize(){
    //Note that "You can assume that the # option will only be used in batch mode."
    //So we know that the input is a file, we can directly load the file into ram
    int* start = (int*)malloc(sizeof(int));
    int* end = (int*)malloc(sizeof(int));
    int error_code = 0;
    error_code = load_file(stdin, start, end, "_SHELL");
    if(error_code != 0){
        return error_code;
    }
    PCB* newPCB = makePCB(*start,*end);
    newPCB->priority = true;
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = newPCB;
    lock_queue();
    ready_queue_add_to_head(node);
    unlock_queue();
    freopen("/dev/tty", "r", stdin);
    return 0;
}

bool execute_process(QueueNode *node, int quanta){
    char *line = NULL;
    PCB *pcb = node->pcb;
    //printf("executing pcb %d\n", pcb->pid);
    // frame_num is the current frame number of the program
    int frame_num = (pcb->PC) / 3; // PC is the line counter. to get the current frame, divide by 3 and take the floor which is done automatically.
    
    bool first = true;

    for (int i = 0; i < quanta; i++) {

        // if frame not assigned in page table... page fault
        if (pcb->page_table[frame_num] == -1) {
            page_fault(pcb, frame_num);
            return false;

        // if pid and frame number in var @ respective line is not the same as the  
        // current pid and frame number... page fault
        } else {
            char *check_var = mem_get_var_at_line((pcb->page_table[frame_num] * 3) + (pcb->PC % 3));
            int PROGRAMNO = atoi(strtok(check_var, "."));
            int FRAMENO = atoi(strtok(NULL, "."));
            int LINEOFFRAME = atoi(strtok(NULL, "."));

            if ((pcb->pid != PROGRAMNO) && (frame_num != FRAMENO)) {
                page_fault(pcb, frame_num);
                return false;
            }
        }

        if (first) {
            ageFrames(pcb->page_table[frame_num]);
            toFrameInUse(pcb->page_table[frame_num]);
            first = false;
        }

        // Line to execute = frame number in frame store * 3 + line in frame
        //printf("\npcb %d: executing line %d of frame %d at line %d\n", pcb->pid, pcb->PC % 3, frame_num, ((pcb->page_table[frame_num] * 3) + (pcb->PC % 3)));
        //printf("%s\n", mem_get_value_at_line((pcb->page_table[frame_num] * 3) + (pcb->PC % 3)));
        line = mem_get_value_at_line((pcb->page_table[frame_num] * 3) + (pcb->PC % 3));

        in_background = true;
        if(pcb->priority) {
            pcb->priority = false;
        }
        // If no more lines to be executed, return true
        if(pcb->PC == pcb->end){
            //printf("executed all of pcb %d, no longer in background\n", pcb->pid);
            parseInput(line);
            free(node);
            //terminate_process(node);
            in_background = false;
            return true;
        }

        parseInput(line);
        in_background = false;

        // Check if reached end of frame
        if (pcb->PC % 3 == 2) {
            toFrameNotInUse(pcb->page_table[frame_num]);
            frame_num++; 
            first = true;
        }
        pcb->PC++;

    }
    toFrameNotInUse(pcb->page_table[frame_num]);
    return false;
}

void *scheduler_FCFS(){
    QueueNode *cur;
    while(true){
        lock_queue();
        if(is_ready_empty()) {
            unlock_queue();
            if(active) continue;
            else break;   
        }
        cur = ready_queue_pop_head();
        unlock_queue();
        //execute_process(cur, MAX_INT);
        
        if(!execute_process(cur, MAX_INT)) {
            lock_queue();
            ready_queue_add_to_head(cur);
            unlock_queue();
        } 
    }
    if(multi_threading) pthread_exit(NULL);
    return 0;
}

void *scheduler_SJF(){
    QueueNode *cur;
    while(true){
        lock_queue();
        if(is_ready_empty()) {
            unlock_queue();
            if(active) continue;
            else break;
        }
        cur = ready_queue_pop_shortest_job();
        unlock_queue();
        execute_process(cur, MAX_INT);
    }
    if(multi_threading) pthread_exit(NULL);
    return 0;
}

void *scheduler_AGING_alternative(){
    QueueNode *cur;
    while(true){
        lock_queue();
        if(is_ready_empty()) {
            unlock_queue();
            if(active) continue;
            else break;
        }
        cur = ready_queue_pop_shortest_job();
        ready_queue_decrement_job_length_score();
        unlock_queue();
        if(!execute_process(cur, 1)) {
            lock_queue();
            ready_queue_add_to_head(cur);
            unlock_queue();
        }   
    }
    if(multi_threading) pthread_exit(NULL);
    return 0;
}

void *scheduler_AGING(){
    QueueNode *cur;
    int shortest;
    sort_ready_queue();
    while(true){
        lock_queue();
        if(is_ready_empty()) {
            unlock_queue();
            if(active) continue;
            else break;
        }
        cur = ready_queue_pop_head();
        shortest = ready_queue_get_shortest_job_score();
        if(shortest < cur->pcb->job_length_score){
            ready_queue_promote(shortest);
            ready_queue_add_to_tail(cur);
            cur = ready_queue_pop_head();
        }
        ready_queue_decrement_job_length_score();
        unlock_queue();
        if(!execute_process(cur, 1)) {
            lock_queue();
            ready_queue_add_to_head(cur);
            unlock_queue();
        }
    }
    if(multi_threading) pthread_exit(NULL);
    return 0;
}

void *scheduler_RR(void *arg){
    int quanta = ((int *) arg)[0];
    QueueNode *cur;
    while(true){
        lock_queue();
        if(is_ready_empty()){
            unlock_queue();
            if(active) continue;
            else break;
        }
        cur = ready_queue_pop_head();
        unlock_queue();
        if(!execute_process(cur, quanta)) {
            lock_queue();
            ready_queue_add_to_tail(cur);
            unlock_queue();
        }
    }
    if(multi_threading) pthread_exit(NULL);
    return 0;
}

int threads_initialize(char* policy){
    active = true;
    multi_threading = true;
    int arg[1];
    pthread_mutex_init(&queue_lock, NULL);
    if(strcmp("FCFS",policy)==0){
        pthread_create(&worker1, NULL, scheduler_FCFS, NULL);
        pthread_create(&worker2, NULL, scheduler_FCFS, NULL);
    }else if(strcmp("SJF",policy)==0){
        pthread_create(&worker1, NULL, scheduler_SJF, NULL);
        pthread_create(&worker2, NULL, scheduler_SJF, NULL);
    }else if(strcmp("RR",policy)==0){
        arg[0] = 2;
        pthread_create(&worker1, NULL, scheduler_RR, (void *) arg);
        pthread_create(&worker2, NULL, scheduler_RR, (void *) arg);
    }else if(strcmp("AGING",policy)==0){
        pthread_create(&worker1, NULL, scheduler_AGING, (void *) arg);
        pthread_create(&worker2, NULL, scheduler_AGING, (void *) arg);
    }else if(strcmp("RR30", policy)==0){
        arg[0] = 30;
        pthread_create(&worker1, NULL, scheduler_RR, (void *) arg);
        pthread_create(&worker2, NULL, scheduler_RR, (void *) arg);
    }
}

void threads_terminate(){
    if(!active) return;
    bool empty = false;
    while(!empty){
        empty = is_ready_empty();
    }
    active = false;
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
}


int schedule_by_policy(char* policy, bool mt){
    //print_ready_queue();
    if(strcmp(policy, "FCFS")!=0 && strcmp(policy, "SJF")!=0 && 
        strcmp(policy, "RR")!=0 && strcmp(policy, "AGING")!=0 && strcmp(policy, "RR30")!=0){
            return 15;
    }
    if(active) {
        return 0;
    }
   // if(in_background) return 0; what does this dooooo
    int arg[1];
    if(mt) return threads_initialize(policy);
    else{
        //printf("calling scheduler with %s\n", policy);
        if(strcmp("FCFS",policy)==0){
            scheduler_FCFS();
        }else if(strcmp("SJF",policy)==0){
            scheduler_SJF();
        }else if(strcmp("RR",policy)==0){
            arg[0] = 2;
            scheduler_RR((void *) arg);
        }else if(strcmp("AGING",policy)==0){
            scheduler_AGING();
        }else if(strcmp("RR30", policy)==0){
            arg[0] = 30;
            scheduler_RR((void *) arg);
        }
        return 0;
    }
}


/**
 * Backing store handler
*/

// Initialize backing store directory
int backing_store_init() {
    int err = 0;
    struct stat info;
    // Check if backing_store already exists 
	if(stat(backing_store_dir, &info) == 0 && S_ISDIR(info.st_mode)) {
        // Delete backing store contents if the directory exists
        err = delete_backing_store_files();
    }
    else {
        err = my_mkdir(backing_store_dir);
    }
    return err;
}

// Delete backing store upon quit
int backing_store_delete() {
    int err = 0;
    struct stat info;

	if(stat(backing_store_dir, &info) == 0 && S_ISDIR(info.st_mode)) { // Make sure backing_store exists
        // Delete all files in directory and then delete directory, save error as 1 if either returns error
        err = (delete_backing_store_files() || rmdir(backing_store_dir));
    }
    return err;
}

// Delete all files in backing store
int delete_backing_store_files() {
    DIR *dir;
    struct dirent *entry;

    // Enter & open backing_store directory
    my_cd(backing_store_dir);
    dir = opendir(".");
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) { // Iterate through entries in backing_store
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;  // Skip over "." and ".." directory entries
            }
            remove(entry->d_name); // rm entry
        }
        // Close and exit backing_store directory
        closedir(dir);
        my_cd("..");
    } else {
        return 1;
    }
    return 0;
}

int copy_to_backing_store(char* filename){
    //printf("adding %s to backing store\n", filename);
    int err = 0;
    // Open the input file to be read
    FILE* script = fopen(filename, "r");
    if (script == NULL) {
        return 11;
    }

    // Create script in backing store and open it for writing
    err = my_cd(backing_store_dir);
    if (err != 0) return err;
    err = my_touch(filename);
    if (err != 0) return err;
    FILE* backing_store_file = fopen(filename, "w");
    if (backing_store_file == NULL) {
        fclose(script);
        return 1; //change this code if not lazy
    }

    // Copy the contents of the input file to the backing store file
    int ch;
    while ((ch = fgetc(script)) != EOF) {
    if (ch == ';') {
        fputc('\n', backing_store_file); // write newline instead of semicolon instead of handling semi-colon nice
    } else {
        fputc(ch, backing_store_file);
    }
}

    // Close the files
    fclose(backing_store_file);
    my_cd("..");
    fclose(script);
    return err;
}
