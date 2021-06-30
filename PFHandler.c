#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERSIZE 10

char scope[8];
int M;
int num_processes;

typedef struct process {
	int pID;
	int num_faults;
	int (*ptptr)[2];
} PROCESS;

PROCESS *Process_new(int id, int length, int page_table[][M][2], int index) { 
	PROCESS* p = malloc(sizeof(PROCESS));
	p->pID = id;
	p->num_faults = 0;
	p->ptptr = page_table[index];
	return p;
}

//comparator function adapted from https://stackoverflow.com/questions/27284185/how-does-the-compare-function-in-qsort-work
int comparator (const void * a, const void * b) {
	return (*(int*)a - *(int*)b);
}

void main(){
	scanf("%s", scope);
	scanf("%d", &M);
	int success = 1;	
	int j = 0;
	
	fflush(stdin);
	
	int num_inputs = 0;
	char *text = calloc(1,1);
	char buffer[BUFFERSIZE];
	while(fgets(buffer, BUFFERSIZE , stdin)) 
	{
		if(strcmp(buffer, "\n") == 0){
			break;
		} 
		
		text = realloc(text, strlen(text)+1+strlen(buffer));
		strcat(text, buffer); 
		num_inputs++;		
	}

	int inputs[num_inputs][2];	
	int input_number = 0;
	int index = 0;	
	char *token;
	const char delimit[2] = " \n";

	token = strtok(text, delimit);

	while( token != NULL) {		
		inputs[input_number][index] = atoi(token);		
		token = strtok(NULL, delimit);		
		
		if(index == 0){
			index = 1;
		}		
		else{
			input_number++;
			index = 0;
		}		
	}
	
	int* process_ids = calloc(1, sizeof(int));
	int found = 0;
	int num_ids = 0;
	for(int i = 0; i < num_inputs; i++){
		found = 0;
		for(int j = 0; j < num_inputs; j++){
			if(inputs[i][0] == process_ids[j]){
				found = 1;
				break;
			}
		}
		if(found == 0){
			process_ids = realloc( process_ids, sizeof(process_ids)+sizeof(int) );
			process_ids[num_ids] = inputs[i][0];
			num_ids++;
		}
	}	
	
	num_processes = num_ids;
	//create array of pointers to processes...
	
	PROCESS *processes[num_processes];	//array of pointers to all process objects
	
	int page_table[num_processes][M][2];
	memset(page_table, -1, num_processes*M*2*sizeof(int));
	
	for(int i = 0; i < num_processes; i++){
		processes[i] = Process_new(process_ids[i], M, page_table, i);
	}
	
	char str_global[8] = "GLOBAL";
	char str_local[8] = "LOCAL";
	
	if(strcmp(str_global, scope) == 0){
		
		int ram[M];		//initially set to -1, otherwise holds ID of process using given frame
		memset(ram, -1, M*sizeof(int));
		
		int current_process;
		int current_page;
		int index_of_process;
		
		int page_found;
		int next_replacement = -1;	//-1 until all frames are full then cycles from 1 to M over and over
		int num_empty_frames = M;
		
		for(int current_input = 0; current_input < num_inputs; current_input++){
			current_process = inputs[current_input][0];
			current_page = inputs[current_input][1];
			
			//determine index of process in processes array...
			for(int i = 0; i<num_ids; i++){
				if(process_ids[i] == current_process){
					index_of_process = i;
					break;
				}
			}
			
			//check if current_page already has an entry in the page table
			page_found = 0;
			for(int j = 0; j < M; j++){
				if(processes[index_of_process]->ptptr[j][0] == current_page){
					page_found = 1;
					break;
				}					
			}
			
			//if not in the page table, add it by replacing whichever frame in ram is next up for replacement
			if(page_found == 0){
				
				if(num_empty_frames != 0){
					int frame_to_fill = M-num_empty_frames;
					ram[frame_to_fill] = current_process;
					num_empty_frames--;
					processes[index_of_process]->ptptr[frame_to_fill][0] = current_page;
					processes[index_of_process]->ptptr[frame_to_fill][1] = 1;
					
					if(num_empty_frames == 0){
						next_replacement = 0;
					}
				}
				
				else{	//no unallocated frames, must actually perform a replacement
					int target_process = ram[next_replacement];

					//determine index of process in processes array...
					int index_of_target;
					for(int i = 0; i<num_ids; i++){
						if(process_ids[i] == target_process){
							index_of_target = i;
							break;
						}
					}

					processes[index_of_target]->ptptr[next_replacement][1] = 0;
					processes[index_of_process]->ptptr[next_replacement][0] = current_page;
					processes[index_of_process]->ptptr[next_replacement][1] = 1;
					ram[next_replacement] = current_process;
					
					if(next_replacement == (M-1)){
						next_replacement = 0;
					}
					
					else{
						next_replacement++;
					}
				}
				processes[index_of_process]->num_faults++;
			}
			
			
			
			//if already in page table, check if current_page is already valid according to page table
			if(page_found == 1){
				int valid_page;
				for(int j = 0; j < M; j++){
					if(processes[index_of_process]->ptptr[j][0] == current_page){
						if(processes[index_of_process]->ptptr[j][1] == 1){	//already valid, nothing to do
							valid_page = 1;
							break;
						}
					}					
				}
				
				//if not valid, it needs to be validated by knocking out which ever page is using the frame next up in ram that new page is to take over
				if(valid_page == 0){ //page not already valid
					int target_process = ram[next_replacement];
					
					//determine index of process in processes array...
					int index_of_target;
					for(int i = 0; i<num_ids; i++){
						if(process_ids[i] == target_process){
							index_of_target = i;
							break;
						}
					}
					processes[index_of_target]->ptptr[next_replacement][1] = 0;
					processes[index_of_process]->ptptr[next_replacement][0] = current_page;
					processes[index_of_process]->ptptr[next_replacement][1] = 1;
					ram[next_replacement] = current_process;

					processes[index_of_process]->num_faults++;
					
					if(next_replacement == (M-1)){
						next_replacement = 0;
					}
					
					else{
						next_replacement++;
					}
				}
			}
			
			
		}
		
		//print all page tables and faults
		printf("PID Page Faults\n");
		for(int i = 0; i < num_processes; i++){
			printf("%-5d%d\n", processes[i]->pID, processes[i]->num_faults);
		}
		
		int pages[M];
		int length;
		int found_valid;
		for(int i = 0; i < num_processes; i++){
			printf("Process %d page table\n", processes[i]->pID);
			printf("P# F#\n");
			
			memset(pages, -1, M*sizeof(int));
			length = 0;
			found_valid = 0;
			
			for(int j = 0; j < M; j++){
				if(processes[i]->ptptr[j][1] == 1){
					pages[length] = processes[i]->ptptr[j][0];
					found_valid = 1;
					length++;
				}
			}
			
			if(found_valid == 0){
				continue;
			}
			
			qsort(pages, M, sizeof(int), comparator);

			for(int j = 0; j < M; j++){
				if(pages[j] != -1){
					for(int k = 0; k < M; k++){
						if(processes[i]->ptptr[k][0] == pages[j]){
							printf("%-5d%d\n", processes[i]->ptptr[k][0], k);
						}
					}
				}
			}
		}
		
		
	}
	
	else if(strcmp(str_local, scope) == 0){
		int ram[M*num_processes];		//initially set to -1, otherwise holds page number currently occupying given frame
		memset(ram, -1, M*num_processes*sizeof(int));
		
		int current_process;
		int current_page;
		int index_of_process;
		int target_page;
		
		int page_found;
		int next_replacement[num_processes];	//-1 until all frames are full then cycles from 1 to M over and over
		memset(next_replacement, -1, num_processes*sizeof(int));
		
		int num_empty_frames[num_processes];

		for(int j = 0; j < num_processes; j++){
			num_empty_frames[j] = M;
		}
		
		int dirty_bits[num_processes][M];
		memset(dirty_bits, 0, M*num_processes*sizeof(int));
		
		for(int current_input = 0; current_input < num_inputs; current_input++){
			current_process = inputs[current_input][0];
			current_page = inputs[current_input][1];
			
			//determine index of process in processes array...
			for(int i = 0; i<num_ids; i++){
				if(process_ids[i] == current_process){
					index_of_process = i;
					break;
				}
			}

			//check if current_page already has an entry in the page table
			page_found = 0;
			for(int j = 0; j < M; j++){
				if(processes[index_of_process]->ptptr[j][0] == current_page){
					page_found = 1;
					break;
				}					
			}
			
			//if not in the page table, add it by replacing next frame with dirty bit not set
			if(page_found == 0){
				
				if(num_empty_frames[index_of_process] != 0){
					int frame_to_fill = (((index_of_process+1)*M))-num_empty_frames[index_of_process];							
					ram[frame_to_fill] = current_page;
					
					int pt_index_to_fill = frame_to_fill-(M*index_of_process);
					
					num_empty_frames[index_of_process]--;
					processes[index_of_process]->ptptr[pt_index_to_fill][0] = current_page;
					processes[index_of_process]->ptptr[pt_index_to_fill][1] = 1;
					dirty_bits[index_of_process][pt_index_to_fill] = 1;		//set dirty bit when new page added
					
					
					if(num_empty_frames[index_of_process] == 0){
						next_replacement[index_of_process] = 0;
					}
				}
				
				else{	//no unallocated frames, must actually perform a replacement
					int is_dirty;
					if(dirty_bits[index_of_process][next_replacement[index_of_process]] == 1){
						is_dirty = 1;
						while(is_dirty == 1){
							dirty_bits[index_of_process][next_replacement[index_of_process]] = 0;
							if(next_replacement[index_of_process] == (M-1)){
								next_replacement[index_of_process] = 0;
							}
							else{
								next_replacement[index_of_process]++;
							}
							is_dirty = dirty_bits[index_of_process][next_replacement[index_of_process]];
						}
					}
					
					processes[index_of_process]->ptptr[next_replacement[index_of_process]][0] = current_page;
					processes[index_of_process]->ptptr[next_replacement[index_of_process]][1] = 1;
					dirty_bits[index_of_process][next_replacement[index_of_process]] = 1;
					
					if(index_of_process == 0){
						ram[(index_of_process*M)+next_replacement[index_of_process]] = current_page;
					}
					else{
						ram[(index_of_process*M)+next_replacement[index_of_process]+1] = current_page;
					}
					
					if(next_replacement[index_of_process] == (M-1)){
						next_replacement[index_of_process] = 0;
					}
					else{
						next_replacement[index_of_process]++;
					}

				}
				processes[index_of_process]->num_faults++;
				
			}
			
			
			
			//if already in page table, check if current_page is already valid according to page table
			if(page_found == 1){

				int valid_page;
				for(int j = 0; j < M; j++){
					if(processes[index_of_process]->ptptr[j][0] == current_page){
						if(processes[index_of_process]->ptptr[j][1] == 1){	//already valid, nothing to do
							valid_page = 1;
							dirty_bits[index_of_process][j] = 1;
							break;
						}
					}					
				}
			}				
		}	
		
		//print all page tables and faults
		printf("PID Page Faults\n");
		for(int i = 0; i < num_processes; i++){
			printf("%-5d%d\n", processes[i]->pID, processes[i]->num_faults);
		}
		
		int local_pages[M];
		int local_length;
		int local_found_valid;
		for(int i = 0; i < num_processes; i++){
			printf("Process %d page table\n", processes[i]->pID);
			printf("P# F#\n");
			
			memset(local_pages, -1, M*sizeof(int));
			local_length = 0;
			local_found_valid = 0;
			
			for(int j = 0; j < M; j++){
				if(processes[i]->ptptr[j][1] == 1){
					local_pages[local_length] = processes[i]->ptptr[j][0];
					local_found_valid = 1;
					local_length++;
				}
			}
			
			if(local_found_valid == 0){
				continue;
			}
			
			qsort(local_pages, M, sizeof(int), comparator);

			for(int j = 0; j < M; j++){
				if(local_pages[j] != -1){
					for(int k = 0; k < M; k++){
						if(processes[i]->ptptr[k][0] == local_pages[j]){
							printf("%-5d%d\n", processes[i]->ptptr[k][0], (M*i)+k);
						}
					}
				}
			}
		}
		
	}
	
	return;

}