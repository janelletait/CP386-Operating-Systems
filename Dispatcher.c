#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

int num_processes = 1;				//the number of processes (including process 0), determined at runtime based on user input
int num_finished_processes = 0;		
int num_blocked_processes = 0;	
int time = 0;						//the global time counter

typedef struct process{
	int pID;					//process ID
	int pStatus; 				//Process status: 0 = new, 1 = running, 2 = ready, 3 = blocked, 4 = done
	int processType; 			//0 = System Idle, 1 = I/O bound, 2 = CPU bound, 3 = Short
	int serviceTime;			//local counter which keeps track of the runtime of the process
	int HDD_requests[3];		//array of requests to the hard drive
	int requiredRunningTime;	//the total required amount of service time depending on the process type
	int arrivalTime;			//the time at which the process is admitted to the system
	int totalTimeReady;			//local counter to keep track of how long the process has been in the ready state
	int totalTimeBlocked;		//local counter to keep track of how long the process has been in the blocked state
} PROCESS;

typedef struct queue_node {		//CITATION: all queue_node and queue based structures and functions have been adapted from Hongbing Fan's Winter 2020 CP264 notes
	struct process *data;
	struct queue_node *next;
} QNODE;

typedef struct queue {
	QNODE *front;
	QNODE *rear;
} QUEUE;

typedef struct processor{
	int running_process_id;		//a pointer to the currently running process
} PROCESSOR;

typedef struct drive{
	int driver_time;			//a local timer which keeps track of how much longer a process has access to the hard drive for the current request
	int io_process_id;			//a pointer to the process currently accessing the hard drive
} DRIVE;

//the Process_new constructer creates a new process object given 3 parameters: the process ID, the process arrival time, and the process type
PROCESS *Process_new(int id, int arrivalTime, int type) { 
	PROCESS* p = malloc(sizeof(PROCESS));
	p->pID = id;
	p->pStatus = 0;
	p->serviceTime = 0;
	p->processType = type;
	
	if(type == 0){		
		p->HDD_requests[0] = -100;		//if the process is not the I/O type, it's array of hard drive requests will be set to -100 so that it will never trigger a hard drive request
		p->HDD_requests[1] = -100;
		p->HDD_requests[2] = -100;
		
		p->requiredRunningTime = -100;		//the required runtime of the idle process is set to -100 because it needs to be capable of running for as long as needed, so there is no limit to its service time
	}

	else if(type == 1){
		p->HDD_requests[0] = 50;
		p->HDD_requests[1] = 100;
		p->HDD_requests[2] = 150;
		
		p->requiredRunningTime = 200;
		
	}
	
	else if(type == 2){		
		p->HDD_requests[0] = -100;
		p->HDD_requests[1] = -100;
		p->HDD_requests[2] = -100;
		
		p->requiredRunningTime = 1000;		
	}
	
	else if(type == 3){	
		p->HDD_requests[0] = -100;
		p->HDD_requests[1] = -100;
		p->HDD_requests[2] = -100;
		
		p->requiredRunningTime = 200;		
	}
	
	p->arrivalTime = arrivalTime;
	p->totalTimeReady = 0;
	p->totalTimeBlocked = 0;
	
	return p;
}

QUEUE *createQueue(){
	QUEUE *q = (QUEUE*)malloc(sizeof(QUEUE));
	q->front = q->rear = NULL;
	return q;
}

QNODE *createQnode(PROCESS *data){
	QNODE *q;
	q = (QNODE*)malloc(sizeof(QNODE));
	q->data = data;
	q->next = NULL;
	return q;
}

//the enqueue function adds a new process np to the queue qp
//parameters: queue to be added to and the process to be added
void enqueue(QUEUE *qp, PROCESS *np) {
	
	QNODE *q = createQnode(np);
	
	if (qp->front == NULL){
		qp->front = q;
		qp->rear = q;
		qp->front->next = qp->rear->next = NULL;
	}
	
	else{
		qp->rear->next = q;
		qp->rear = q;
		qp->rear->next = NULL;
	}
}  

//the dequeue function removes a process from the front of queue qp
//parameters: queue to be removed from
PROCESS *dequeue(QUEUE *qp) {
	if(qp->front == NULL){
		printf("\nUNDERFLOW");
		return NULL;
	}
	
	else{
		QNODE *ptr = qp->front;
		PROCESS *temp = ptr->data;
		qp->front = ptr->next;
		free(ptr);
		return temp;
	}
}

void main(){
	
	QUEUE *new_queue = createQueue();	
	QUEUE *ready_queue = createQueue();
	QUEUE *blocked_queue = createQueue();
	
	PROCESSOR *CPU = (PROCESSOR*)malloc(sizeof(PROCESSOR));
	CPU->running_process_id = -10;		//before the simulation begins, no process is currently running so the CPU will be pointing to an arbitrary invalid process ID -10
	
	DRIVE *HDD = (DRIVE*)malloc(sizeof(DRIVE));
	HDD->io_process_id = -10;			//before the simulation begins, no process is currently accessing the hard drive so the hard drive will be pointing to an arbitrary invalid process ID -10
	HDD->driver_time = 0;				
	
	char input_string[25];
	scanf("%25s", &input_string);
	
	
	for(int i = 0; i < 25 && input_string[i] != '\0'; i++){
		num_processes++;				//based on the length of the user input string, calculate and update the number of processes in the simulation
	}
	
	PROCESS *table[num_processes];		//table is an array of pointers to all processes in the simulation 
	
	for(int i = 1; i < num_processes; i++){
		int type;
		
		if(input_string[i-1] == 'I'){
			type = 1;
		}
		
		else if(input_string[i-1] == 'C'){
			type = 2;
		}
		
		else if(input_string[i-1] == 'S'){
			type = 3;
		}
		
		table[i] = Process_new(i, (i-1)*100, type);		//for each letter in the input string, create a new process 
	}
	
	PROCESS *system_idle = Process_new(0, 0, 0);		//create the system idle process with ID 0 and set it to the ready state for the beginning of the simulation
	system_idle->pStatus = 2;
	table[0] = system_idle;
	
	for(int i = 1; i < num_processes; i++){
		enqueue(new_queue, table[i]);					//enqueue all processes except for the system idle process into the new_queue
	}	
	
	
	while(num_finished_processes < (num_processes - 1)){		//the simulation will run until all processes are finished, excluding the system idle process
		
		if(HDD->io_process_id > 0){			
			if(HDD->driver_time == 0){		//if there is a process accessing the hard drive and the current request is complete, that process is transfered to the ready queue and the hard drive is now free
				enqueue(ready_queue, table[HDD->io_process_id]);	
				table[HDD->io_process_id]->pStatus = 2;
				HDD->io_process_id = -10;
			}
		}
		
		if(new_queue->front){
			if(new_queue->front->data->arrivalTime == time){	//for each process in the new_queue, when the global time is equal to its arrival time, the process is made ready and officially enters the simulation
				PROCESS *incoming_process = dequeue(new_queue);
				enqueue(ready_queue, incoming_process);
				incoming_process->pStatus = 2;	
			}		
		}		
	
		
		if(CPU->running_process_id > 0){
			if(table[CPU->running_process_id]->serviceTime == table[CPU->running_process_id]->requiredRunningTime){		//if the service time of the process currently in CPU is equal to the required running time, the process has completed its execution so it is terminated
				table[CPU->running_process_id]->pStatus = 4;
				num_finished_processes++;	
				
				if(num_finished_processes == (num_processes-1)){		//if all processes are now complete, the simulation ends
					break;
				}
				else{
					CPU->running_process_id = 0;		//if there are still other incomplete processes, the system idle process temporarily occupies the CPU until the ready queue is checked
					table[0]->pStatus = 1;
				}
			}
		}		
		
		if(CPU->running_process_id > 0){

			if(table[CPU->running_process_id]->processType == 1){		//if the currently running process is an I/O process, we need to check if it has an active hard drive request
				int flag = 0;
				for(int i = 0; i <= 2; i++){	//loop through the process' array of hard drive requests and check if the service time is equal to the timestamp for any of the hard drive requests
					if(table[CPU->running_process_id]->serviceTime == table[CPU->running_process_id]->HDD_requests[i]){
						flag = 1;
						break;
					}
				}
				
				if(flag == 1){		//if the process has a hard drive request for the current time, block the process and move the idle process to CPU
					enqueue(blocked_queue, table[CPU->running_process_id]);
					table[CPU->running_process_id]->pStatus = 3;
					table[0]->pStatus = 1;					
					CPU->running_process_id = 0;
					num_blocked_processes++;
				}
				
			}

		}
		
		if(CPU->running_process_id <= 0 && ready_queue->front){		//if there is no process in CPU because the simulation is just beginning, or the idle process is running, and there is a ready process, run the first process in the ready queue
			PROCESS *next_process = dequeue(ready_queue);
			CPU->running_process_id = next_process->pID;
			next_process->pStatus = 1;
			table[0]->pStatus = 2;
		}
		
		if(CPU->running_process_id >= 0){		//increase the service time of the currently running process by 50ms in each iteration
			table[CPU->running_process_id]->serviceTime += 50;
		}
		
		
		if(HDD->io_process_id == -10 && blocked_queue->front){		//if there is no process currently accessing the hard drive and there is a blocked process waiting to access the hard drive, allow the first process in the blocked queue to access the hard drive
			PROCESS *next_io = dequeue(blocked_queue);
			HDD->io_process_id = next_io->pID;
			HDD->driver_time = 1000;				
		}
		
		if(HDD->io_process_id > 0){		//if there is a process accessing the hard drive, increase the total time blocked counter of the process currently accessing the hard drive and decrement the driver time, both by 50ms in each iteration
			table[HDD->io_process_id]->totalTimeBlocked += 50;
			HDD->driver_time = HDD->driver_time - 50;
			
		}
		
		QNODE *current = ready_queue->front;
		while(current){		//iterate through each process in the ready queue and increment the total time ready counter by 50ms for each process
			current->data->totalTimeReady += 50;
			current = current->next;
		}		

		current = blocked_queue->front;
		while(current){		//iterate through each process in the blocked queue and increment the total time blocked counter by 50ms for each process
			current->data->totalTimeBlocked += 50;
			current = current->next;
		}
		
		if(CPU->running_process_id != 0){		//if the idle process is not currently running, incrememnt its total time ready counter by 50ms, since it is always either running or ready
			table[0]->totalTimeReady += 50;
		}
		
		time = time + 50;	
	}
	

	for(int i = 0; i < num_processes; i++){
		if(i == 0){
			printf("%2d %7d %7d\n", table[i]->pID, table[i]->serviceTime, table[i]->totalTimeReady);
		}
		
		else{
			printf("%2d %7d %7d %7d\n", table[i]->pID, table[i]->serviceTime, table[i]->totalTimeReady, table[i]->totalTimeBlocked);
		}
	}
	
	
}