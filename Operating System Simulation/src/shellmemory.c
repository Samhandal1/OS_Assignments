#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include "pcb.h"
#include "ready_queue.h"

// IGNOR THE ERRORS :) IT COMPILES
int FRAME_MEM_SIZE = (int) FRAMEMEMSIZE;	// number of lines in the frame store
int VAR_MEM_SIZE = (int) VARMEMSIZE; 		// number of lines in the variable store
int SHELL_MEM_LENGTH = (int) (FRAMEMEMSIZE + VARMEMSIZE);
int NUM_FRAMES = ((int) FRAMEMEMSIZE) / 3;

struct memory_struct {
	char *var;
	char *value;
};

struct frame {
    bool in_use;  		// if frame is currently occupied
    int age;            // referance timestamp
    PCB *pcb;           // associated PCB
};

struct memory_struct shellmemory[FRAMEMEMSIZE + VARMEMSIZE];
struct frame framelist[(int) (FRAMEMEMSIZE / 3)];

// Helper functions
int match(char *model, char *var) {
	int i, len=strlen(var), matchCount=0;
	for(i=0;i<len;i++)
		if (*(model+i) == *(var+i)) matchCount++;
	if (matchCount == len)
		return 1;
	else
		return 0;
}

char *extract(char *model) {
	char token='=';    // look for this to find value
	char value[1000];  // stores the extract value
	int i,j, len=strlen(model);
	for(i=0;i<len && *(model+i)!=token;i++); // loop till we get there
	// extract the value
	for(i=i+1,j=0;i<len;i++,j++) value[j]=*(model+i);
	value[j]='\0';
	return strdup(value);
}


// Shell memory functions

void mem_init(){
	int i;
	for (i=0; i<SHELL_MEM_LENGTH; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
	int i;
	for (i=0; i<SHELL_MEM_LENGTH; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<SHELL_MEM_LENGTH; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	return;

}

/**A3 : start from top of shell memory**/
//get value based on input key
char *mem_get_value(char *var_in) {
	int i;
	for (i=(SHELL_MEM_LENGTH-1); i>=FRAME_MEM_SIZE; i--){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return NULL;

}


void printShellMemory(){
	int count_empty = 0;
	for (int i = 0; i < SHELL_MEM_LENGTH; i++){
		if(strcmp(shellmemory[i].var,"none") == 0){
			count_empty++;
		}
		else{
			printf("\nline %d: key: %s\t\tvalue: %s\n", i, shellmemory[i].var, shellmemory[i].value);
		}
    }
	printf("\n\t%d lines in total, %d lines in use, %d lines free\n\n", SHELL_MEM_LENGTH, SHELL_MEM_LENGTH-count_empty, count_empty);
}


/*
 * Function:  addFileToMem 
 * 	Added in A2
 * --------------------
 * Load the source code of the file fp into the shell memory:
 * 		Loading format - var stores fileID, value stores a line
 *		Note that the first 100 lines are for set command, the rests are for run and exec command
 *
 *  pStart: This function will store the first line of the loaded file 
 * 			in shell memory in here
 *	pEnd: This function will store the last line of the loaded file 
 			in shell memory in here
 *  fileID: Input that need to provide when calling the function, 
 			stores the ID of the file
 * 
 * returns: error code, 21: no space left
 */
int load_file(FILE* fp, int* pStart, int* pEnd, char* filename)
{
	char *line;
    size_t i;
    int error_code = 0;
	bool hasSpaceLeft = false;
	bool flag = true;
	i=101;
	size_t candidate;
	while(flag){
		flag = false;
		for (i; i < SHELL_MEM_LENGTH; i++){
			if(strcmp(shellmemory[i].var,"none") == 0){
				*pStart = (int)i;
				hasSpaceLeft = true;
				break;
			}
		}
		candidate = i;
		for(i; i < SHELL_MEM_LENGTH; i++){
			if(strcmp(shellmemory[i].var,"none") != 0){
				flag = true;
				break;
			}
		}
	}
	i = candidate;
	//shell memory is full
	if(hasSpaceLeft == 0){
		error_code = 21;
		return error_code;
	}
    
    for (size_t j = i; j < SHELL_MEM_LENGTH; j++){
        if(feof(fp))
        {
            *pEnd = (int)j-1;
            break;
        }else{
			line = calloc(1, SHELL_MEM_LENGTH);
            fgets(line, 999, fp);
			shellmemory[j].var = strdup(filename);
            shellmemory[j].value = strndup(line, strlen(line));
			free(line);
        }
    }

	//no space left to load the entire file into shell memory
	if(!feof(fp)){
		error_code = 21;
		//clean up the file in memory
		for(int j = 1; i <= SHELL_MEM_LENGTH; i ++){
			shellmemory[j].var = "none";
			shellmemory[j].value = "none";
    	}
		return error_code;
	}
	//printShellMemory();
    return error_code;
}



char * mem_get_value_at_line(int index){
	if(index<0 || index > SHELL_MEM_LENGTH) return NULL; 
	return shellmemory[index].value;
}

char * mem_get_var_at_line(int index){
	if(index<0 || index > SHELL_MEM_LENGTH) return NULL; 
	char *return_var = strdup(shellmemory[index].var);
	return return_var;
}

void mem_free_lines_between(int start, int end){
	for (int i=start; i<=end && i<SHELL_MEM_LENGTH; i++){
		if(shellmemory[i].var != NULL){
			free(shellmemory[i].var);
		}	
		if(shellmemory[i].value != NULL){
			free(shellmemory[i].value);
		}	
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}


/*
* A3 Memory 
* Heap and stack style memory
* Frames of size 3 are stored in "heap", i.e. in shellmemory[0], shellmemory[2], and increasing by 3.
* Variables are stored in "stack", i.e. in shellmemory[SHELL_MEM_LENGTH-1], and descending.
* Sizes are set dynamically. (static now but later)
* Keys in shell memory are <PROGRAMNO>.<FRAMENO>.<LINEOFFRAME>, where FRAMENO is the frame number of that program, and LINEOFFRAME = {0,1,2}
* * so if the first program is loaded into an empty frame store and we want to get the last line of the first frame, use the key 1.0.2
*/

// Set key value pair
void mem_set_value_store(char *var_in, char *value_in) {
	int i;
	for (i=(SHELL_MEM_LENGTH-1); i>=FRAME_MEM_SIZE; i--){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=(SHELL_MEM_LENGTH-1); i>=FRAME_MEM_SIZE; i--){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	return;

}

// Load file into frame store by finding as many three consecutive free spaces as needed to load all of file into frames
// Initialize start, end, pagetable pointers according to the script
int load_frame_store(FILE* fp, int page_table[], char* filename, int pt_size) {
    int error_code = 0;
	bool hasSpaceLeft = false;
	bool flag = true;
	int i = 0;
	int page_table_idx = 0;
	int frame_start;
	int frame_end;
	int frame_counter = 0;
	bool found_frame = true;
	int curpid = get_pid_counter(); // need the pid of PCB but PCB for this script isn't created. not good practice but it works
    if (page_table == NULL) {
        error_code = 21;
		printf("page table null");
        return error_code;
    }

	// Load script into frames
	while (flag) {

		found_frame = true;
		frame_start = i;
		frame_end = i+3;
		for (i; i < frame_end; i++){
			if(strcmp(shellmemory[i].var,"none") != 0){  // Skip these three spots and start again from j
				frame_counter++; // next frame
				i = frame_end;
				found_frame = false;
				break;
			}
		}

		// Found three consecutive spots: save frame number to page table and lines of script to memory
		if (found_frame) {
			//rewind i			
			i = frame_start;

			for (i; i < frame_end; i++){
				if (feof(fp)) {
					flag = false;
					break;
				}
				else {

					//if (*start == -1) *start = i; // if not initialized yet
					// save line to be put as value in memory

					char line[999];
					fgets(line, 999, fp);

					// save key to be put as var in memory
					char key[50];  //this might give a stack smash not sure how long it has to be
					sprintf(key, "%d.%d.%d", curpid, page_table_idx, (i-frame_start));

					shellmemory[i].var = malloc(strlen(key) + 1);
					strcpy(shellmemory[i].var, key);

					shellmemory[i].value = malloc(strlen(line) + 1);
					strcpy(shellmemory[i].value, line);
					
					// memset(key, 0, sizeof(key));
				}
			}

			page_table[page_table_idx] = frame_counter;  
			page_table_idx++;
			frame_counter++;

			// Frame store is full
			if (i > FRAME_MEM_SIZE) {
				flag = false;
				error_code = 21;
				return error_code;
			}
		}

		// only load first 2 pages into memory, other pages not in mem will initiate to -1 to indicate its not in mem 
		if (page_table_idx == 2) {
			// int pt_size = (int) (sizeof(page_table) / sizeof(page_table[0]));
			for(int ctr = 2; ctr < pt_size; ctr++) {
				page_table[ctr] = -1;
			}
			flag = false;
		}
	}

	//printShellMemory();
	//no space left to load the entire file into shell memory
	if (!feof(fp) && page_table_idx < 2) {
		error_code = 21;
		//should clean up the file in memory
		return error_code;
	}
    return error_code;

}

void addPCBtoframelist(PCB *pcb, int num_frames) {
	for (int i = 0; i < num_frames; i++) {
		int frame_number = pcb->page_table[i];
		if (frame_number != -1) {
			framelist[frame_number].in_use = false;
			framelist[frame_number].age = 0;
			framelist[frame_number].pcb = pcb;
		}
	}
}

void updatePCBframelist(PCB *pcb, int frame_number) {
	framelist[frame_number].in_use = false;
	framelist[frame_number].age = 0;
	framelist[frame_number].pcb = pcb;
}

void ageFrames(int frame_number) {
	framelist[frame_number].age = 0;
	for (int i = 0; i < NUM_FRAMES; i++) {
		if (framelist[i].in_use == false && i != frame_number) {
			framelist[i].age += 1;
		}
	}
}

void toFrameInUse(int frame_number) {
	framelist[frame_number].in_use = true;
}

void toFrameNotInUse(int frame_number) {
	framelist[frame_number].in_use = false;
}

int findLRU() {
	int lru_frame = -1;
	int lru_age = -1;
	for (int i = 0; i < NUM_FRAMES; i++) {
		int cur_age = framelist[i].age;
		if (cur_age >= lru_age) {
			lru_age = cur_age;
			lru_frame = i;
		}
	}

	return lru_frame;
}

int load_a_single_frame(FILE* fp, int target_line_num, int pid) {
	int i = 0;
	int page_table_idx = target_line_num / 3;
	int frame_start;
	int frame_end;
	int frame_counter = 0;
	bool found_frame = false;
	int curpid = pid; // need the pid of PCB but PCB for this script isn't created. not good practice but it works
	int ctr = 0;

	// find first free spot
	while (ctr <= (FRAME_MEM_SIZE - 3)) {
		frame_start = ctr;
		frame_end = ctr+3;
		found_frame = true;
		for (i = frame_start; i < frame_end; i++) {
			if (strcmp(shellmemory[i].var,"none") != 0) { 
				found_frame = false;
				ctr = frame_end;
				break;
			}
		}
		if (found_frame) break;
	}

	// Found three consecutive spots: save frame number to page table and lines of script to memory
	if (!found_frame) {

		// get a random frame to evict
		// srand(time(NULL)); //use current time as seed for random generator
    	// int random_frame = rand() % NUM_FRAMES;
		// frame_start = random_frame * 3;
		frame_start = findLRU() * 3;

		// get information from  var
		char *var_to_check = strdup(shellmemory[frame_start].var);
		int pid = atoi(strtok(var_to_check, "."));
		int pt_idx = atoi(strtok(NULL, "."));
		int frame_num = atoi(strtok(NULL, "."));

		if (frame_num != 0) {
			frame_start -= frame_num;
		}

		frame_end = frame_start + 3;

		// delete frame from page table in pcb
		// QueueNode *temp = ready_queue_get_head();
		// while (temp != NULL) {
		// 	if (temp->pcb->pid == pid) {
		// 		temp->pcb->page_table[pt_idx] = -1;
		// 		break;
		// 	}
		// 	temp = temp->next;
		// }

		// print the victim
		printf("Page fault! Victim page contents:\n\n");
		for (int j = frame_start; j < frame_end; j++) {
			printf("%s", shellmemory[j].value);
		}
		printf("\nEnd of victim page contents.\n");
	}

	// go to line that needs to be transfered
	char buffer[1000];
	int num_lines = 0;
	while (num_lines < target_line_num) {
		fgets(buffer, sizeof(buffer), fp);
		num_lines++;
	}
	memset(buffer, 0, sizeof(buffer)); // clear buffer

	//rewind i			
	i = frame_start;
	int eof_ctr = 0;

	for (i; i < frame_end; i++)  {

		char line[999];
		fgets(line, 999, fp);

		// save key to be put as var in memory
		char key[50];  //this might give a stack smash not sure how long it has to be
		sprintf(key, "%d.%d.%d", curpid, page_table_idx, (i-frame_start));

		shellmemory[i].var = malloc(strlen(key) + 1);
		strcpy(shellmemory[i].var, key);

		if (eof_ctr >= 1) {
			shellmemory[i].value = "none";
		} else {
			shellmemory[i].value = malloc(strlen(line) + 1);
			strcpy(shellmemory[i].value, line);
		}

		if (feof(fp)) {
			eof_ctr++;
		}

		// shellmemory[i].value = malloc(strlen(line) + 1);
		// strcpy(shellmemory[i].value, line);
	}

	// return the frame #
	return (frame_start / 3);
}