#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define NEW 0
#define READY 1
#define RAN 2
#define BLOCKED 3
#define EXITED 4

typedef struct Process {
	int id;
	int status;
	int requests[512];
	int allocated[512];

} process;

void processRequestInput(char str[], int);
void processAllocationInput(char str[], int);
void unAllocatedInput(char str[]);
process *create_process(int pid);
void run_processes();
int initial_check();
void display_ran_processes();
void block_processes();
void block_process(process *proc);
void display_new_processes();
void display_current_blocked_processes();


int UnAllocatedArray[512];
process *processTable[512];
process* ran_processes[512];
int processCount;
int resourceCount;
int num_processes_ran;
int num_runnable_processes;
int num_processes_gone;

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);

	int x;
	for(x=0;x<512;x++){
		UnAllocatedArray[x] = -1;
	}

	processCount = 1;
	num_processes_ran = 0;
	num_processes_gone = 0;

	int count = 0;
	int num_processes;
	int num_resources;
	num_runnable_processes = 0;

	char str[128];
	while (fgets(str, sizeof(str), stdin) != NULL) {
		if (strcmp(str, "\n\0") != 0) {
			if (count == 0){
				const char delims[] = " \n\t\r";
				num_processes = atoi(strtok(str, delims));
				num_resources = atoi(strtok(NULL, delims));
				processCount = num_processes;
				resourceCount = num_resources;
				int p;
				for(p = 0;p<processCount;p++){
					process *temp_proc;
					temp_proc = create_process(p+1);
					processTable[p] = temp_proc;
				}
			}
			else if (count <= num_processes){
				processRequestInput(str, count-1);
			}
			else if (count <= 2*num_processes){
				processAllocationInput(str, (count-processCount)-1);
			}
			else{
				unAllocatedInput(str);
				int can_run = initial_check();
				run_processes();
				break;
			}
			count = count+1;
		}

	}

	display_ran_processes();

	return EXIT_SUCCESS;
}


process *create_process(int pid){
	process *proc;
	proc = (process*) malloc(sizeof(process));
	proc->id = pid;
	proc->status = 0;
	int i;
	for(i=0;i<resourceCount;i++){
		proc->allocated[i] = -1;
		proc->requests[i] = -1;
	}

	return proc;
}

void processRequestInput(char str[], int row_num){
	//printf("\n");
	int x;
	int tempnum;
	int count = 0;
	for(x = 0;x<(resourceCount*2)-1;x++){
		if(str[x] != 32){
			tempnum = str[x];
			tempnum = tempnum - 48;
			processTable[row_num]->requests[count] = tempnum;
			count = count+1;
		}
	}
}

void processAllocationInput(char str[], int row_num){
	int x;
	int tempnum;
	int count = 0;
	for(x = 0;x<(resourceCount*2)-1;x++){
		if(str[x] != 32){
			tempnum = str[x];
			tempnum = tempnum - 48;
			processTable[row_num]->allocated[count] = tempnum;
			count = count+1;
		}
	}
}

void unAllocatedInput(char str[]){
	int x;
	int tempnum;
	int count = 0;
	int num;
	for(x = 0;x<(resourceCount*2)-1;x++){
		if(str[x] != 32){
			tempnum = str[x];
			tempnum = tempnum - 48;
			num = tempnum*1;
			UnAllocatedArray[count] = num;
			count = count+1;
		}
	}
}


int initial_check(){
	int x;
	int i;
	int unalloc_num;
	int required_num;
	int avail_count = 0;
	int a_count = 0;
	for(x=0;x<processCount;x++){
		avail_count = 0;
		process *temp_proc = processTable[x];
		if(temp_proc->status == 0){
			for(i=0;i<resourceCount;i++){
				unalloc_num = UnAllocatedArray[i];
				required_num = temp_proc->requests[i];
				if (required_num <= unalloc_num){
					avail_count = avail_count+1;
				}
			}
		}
		if (avail_count == resourceCount){
			a_count = a_count+1;
			temp_proc->status = 1;
		}
	}
	num_runnable_processes = a_count;
	if (a_count > 0){
		return 1;
	}
	else{
		return 0;
	}
}

void run_processes(){
	int x;
	int i;
	int unalloc_num;
	int allocated_num;
	int avail_count = 0;
	int can_anything_run =1;

	int num_processes_ran = 0;
	int deadlock = 0;
	process *temp_proc;

	while(num_processes_gone < processCount){
		if (num_runnable_processes == 0){
			deadlock = 1;
			display_new_processes();
			block_processes();
		}
		else{
			for(x=0;x<processCount;x++){
				temp_proc = processTable[x];
				if (temp_proc->status == 1){
					for(i=0;i<resourceCount;i++){
						allocated_num = temp_proc->allocated[i];
						UnAllocatedArray[i] = UnAllocatedArray[i]+allocated_num;
						temp_proc->requests[i] = 0;
						temp_proc->allocated[i] = 0;
					}
					temp_proc->status = 4;
					ran_processes[num_processes_ran] = temp_proc;
					num_processes_ran = num_processes_ran+1;
					num_processes_gone = num_processes_gone+1;
				}
			}
			can_anything_run = initial_check();
		}
	}

}


void block_processes(){
	int x;
	int i;
	int can_run;
	process *temp_proc;
	//Block Processes 1 by 1 and check if anything can run
	for(x=0;x<processCount;x++){
		//block Process x+1
		temp_proc = processTable[x];
		if(temp_proc->status == 0){
			block_process(temp_proc);
			can_run = initial_check();
			if (can_run == 1){
				display_current_blocked_processes();
				break;
			}
		}
	}
}


void block_process(process *proc){
	int j;
	int request_num;
	for(j=0;j<resourceCount;j++){
		request_num = proc->allocated[j];
		UnAllocatedArray[j] = UnAllocatedArray[j]+request_num;
	}
	proc->status = 3;
	num_processes_gone = num_processes_gone+1;
}


void display_ran_processes(){
	int x = 0;
	while(ran_processes[x] != NULL){
		printf("%d ", ran_processes[x]->id);
		x = x+1;
	}
}

void display_current_blocked_processes(){
	int x = 0;
	process *temp_proc;
	while(processTable[x] != NULL){
		temp_proc = processTable[x];
		if(temp_proc->status == 3){
			printf("%d ", temp_proc->id);
		}
		x = x+1;
	}
	printf("\n");
}

void display_new_processes(){
	int x = 0;
	process *temp_proc;
	while(processTable[x] != NULL){
		temp_proc = processTable[x];
		if(temp_proc->status == 0){
			printf("%d ", temp_proc->id);
		}
		x = x+1;
	}
	printf("\n");
}
