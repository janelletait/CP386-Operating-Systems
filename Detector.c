#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

int num_processes = 0;
int num_resources = 0;
int terminated_processes = 0;
int num_deadlocked_processes = 0;

//the terminate function completes the termination of a given process by deallocating all its resources and adding them to the unallocated array
void terminate(int (*requests)[num_processes][num_resources], int (*allocations)[num_processes][num_resources], int (*unallocated)[num_resources], int pID){
	for(int i = 0; i <= (num_resources-1); i++){	//deallocate all of the process's resources and add them to the unallocated array so they can become available to the rest of the processes
		(*unallocated)[i] += (*allocations)[pID][i];
		(*requests)[pID][i] = 0;
		(*allocations)[pID][i] = 0;
		
	}
	
	terminated_processes++;
	return;
}

//the check_deadlock function examines the requests and allocations of all processes to determine which ones are involved in the deadlock, and sets the deadlocked_IDs array accordingly 
void check_deadlock(int (*requests)[num_processes][num_resources], int (*allocations)[num_processes][num_resources], int (*deadlock_IDs)[num_processes], int (*unallocated)[num_resources]){
	
	num_deadlocked_processes = 0;
	int have;
	int need;
	
	int requested_resources[num_resources];		//this is a matrix used to keep track of how many requests there are for each resource
	memset(requested_resources, 0, num_resources*sizeof(int));
	
	for(int i = 0; i <= (num_processes-1); i++){
		(*deadlock_IDs)[i] = 0;		//reset the array of deadlocked processes back to 0 so we can perform a recount if this is not the first deadlock in the system
		for(int j = 0; j <= (num_resources-1); j++){
			if((*requests)[i][j] > 0){
				requested_resources[j] += (*requests)[i][j];	
			}
		}
	}
	
	for(int i = 0; i <= (num_processes-1); i++){
		have = 0;
		need = 0;
		for(int j = 0; j <= (num_resources-1); j++){
			
			if(((*allocations)[i][j] > 0) && (requested_resources[j] > (*unallocated)[j])){		//if a process is allocated a resource, and there are requests for that resource that cannot be fulfilled, mark this process as "having" a desired resource
				have = j+1;				
			}
			
			if(((*requests)[i][j] > 0) && (requested_resources[j] > (*unallocated)[j])){	//if a process is requesting a resource and that request cannot be fulfilled, mark that process as being in "need" of an unavailable resource
				need = j+1;
			}
		}

		if(have >= 1 && need >= 1){		//if a process has a desired resource AND wants an unavailable resource, that means it is a part of the deadlock cycle, so it is added to the array of deadlocked processes
			(*deadlock_IDs)[i] = 1;
			num_deadlocked_processes++;
		}
	}		
}

void main(){
	scanf("%d %d", &num_processes, &num_resources);

	int requests[num_processes][num_resources];
	int allocations[num_processes][num_resources];
	int unallocated[num_resources];
	int terminated_IDs[num_processes];
	int deadlock_IDs[num_processes];
	
	memset(terminated_IDs, 0, num_processes*sizeof(int));
	
	int process_terminated;
	
	int m = 0;
	int temp_num;
	char temp_char;
	
	for(int i = 0; i <= (num_processes-1); i++){

		m = 0;
		while(m <= (num_resources - 1)){		//read the request array from input
			scanf("%d%c", &temp_num, &temp_char);
			requests[i][m] = temp_num;
			m++;
		}
	}
	
	for(int i = 0; i <= (num_processes-1); i++){

		m = 0;
		while(m <= (num_resources - 1)){		//read the allocations array from input
			scanf("%d%c", &temp_num, &temp_char);
			allocations[i][m] = temp_num;
			m++;
		}
	}
	
	m = 0;
	while(m <= (num_resources - 1)){		//read the unallocated array from input
		scanf("%d%c", &temp_num, &temp_char);
		unallocated[m] = temp_num;
		m++;
	}
	
	the_loop:
	
	process_terminated = 0;

	do{
		int process_to_finish = 1;
		for(int i = 0; i <= (num_processes-1); i++){	//loop through all processes and if a process can execute, run it
			
			process_to_finish = 1;
			process_terminated = 0;
			
			if(terminated_IDs[i] == 1){		//if a process is already terminated, it cannot run
				process_to_finish = 0;
			}
			
			for(int j = 0; j <= (num_resources-1); j++){	//if a process has a request that cannot be fulfilled, it cannot run
				if(requests[i][j] > unallocated[j]){
					process_to_finish = 0;
				}	
			}	
			
			if(process_to_finish == 1){	//if the process can run, mark it as terminated because it is now complete
				terminate(&requests, &allocations, &unallocated, i);
				printf("%d ", i+1);
				terminated_IDs[i] = 1;
				process_terminated = 1;
			}				
		}
		
	}while(process_terminated == 1 && terminated_processes < num_processes);	//if a process has sucessfully executed, continue looping through the processes to see if more processes can now execute
	
	//if no processes can execute but not all have finished, that means there is a deadlock
	if(terminated_processes < num_processes){

		check_deadlock(&requests, &allocations, &deadlock_IDs, &unallocated);	//check which processes are involved in the deadlock
		
		printf("\n");
		for(int i = 0; i <= (num_processes-1); i++){	//output all processes involved in the deadlock
			if(deadlock_IDs[i] == 1){
				printf("%d ", i+1);
			}
		}
		printf("\n"); 
		
		int total_allocations[num_deadlocked_processes];
		memset(total_allocations, 0, num_deadlocked_processes*sizeof(int));
		
		int index = 0;
		int max_process_index = 0;
		int max_allocations = 0;
		for(int i = 0; i <= (num_processes-1); i++){	//find the first deadlocked process with the largest total allocation
			if(deadlock_IDs[i] == 1){
				for(int j = 0; j <= (num_resources-1); j++){
					total_allocations[index] += allocations[i][j];	
				}
				
				if(total_allocations[index] > max_allocations){
					max_allocations = total_allocations[index];
					max_process_index = i;
				}
				
				index++;
			}
		}
		
		printf("%d ", max_process_index+1);
		terminate(&requests, &allocations, &unallocated, max_process_index);	//terminate the first process with the largest total allocation
		terminated_IDs[max_process_index] = 1;
		
		printf("\n");
		goto the_loop;		//return to checking for and executing any processes able to run now that a deadlock has been resolved
		
	}	
}
