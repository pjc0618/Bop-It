#include "3140_concur.h"
#include "realtime.h"
#include <stdlib.h>
struct process_state {
	unsigned int* sp;
	unsigned int* sp_old;
	struct process_state* next;
	realtime_t* arrival_time;
	realtime_t* deadline;
	int is_rt;
	int n;
};

process_t* process_queue = NULL;
process_t* current_process = NULL;
realtime_t current_time = {0,0};
process_t* ready_queue = NULL;
process_t* wait_queue = NULL;
int process_deadline_miss = 0;
int process_deadline_met = 0;

int getabstime(realtime_t* rt) {
	int t = 0;
	t += (rt->sec) * 1000;
	t += rt->msec;
	return t;
}

// append process (w/ state) to process_queue
void enqueue(process_t* *queue, process_t* process) {
	if (*queue == NULL) {
		*queue = process;
		process->next = NULL;
	} else if (*queue == process_queue) {
		process_t* tmp = *queue;
		while(tmp->next != NULL) {
			tmp = tmp->next;
			process->next = NULL;
		}
		tmp->next = process;
	} else if (*queue == ready_queue) {
		process_t* tmp = *queue;
		if(getabstime(tmp->deadline) >= getabstime(process->deadline)){
			process->next = tmp;
			*queue = process;
		}else{
			while((getabstime(tmp->deadline) < getabstime(process->deadline)) && (tmp->next != NULL)){
				tmp = tmp->next;
			}
			process->next = tmp->next;
			tmp->next = process;
		}
	} else {
		process_t* tmp = *queue;
		if(getabstime(tmp->arrival_time) > getabstime(process->arrival_time)){
			process->next = tmp;
			*queue = process;
		}else{
			while((getabstime(tmp->arrival_time) <= getabstime(process->arrival_time)) && (tmp->next != NULL)){
				tmp = tmp->next;
			}
			process->next = tmp->next;
			tmp->next = process;
		}
	}
	//process->next = NULL;
}

// remove first head of queue and return it
process_t* dequeue(process_t* *queue) {
	if (*queue == NULL) return NULL;
	
	process_t* element = *queue;
	*queue = (*queue)->next;
	element->next = NULL;
	return element;
}

int process_create (void (*f)(void), int n) {
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		return -1;
	}
	proc->sp_old = process_stack_init(f,n);
	proc->sp = proc->sp_old;
	proc->n = n;
	proc->is_rt = 0;
	
	if(!proc->sp){
		return -1;
	}
	
	proc->next = NULL;
	enqueue(&process_queue, proc);
	return 0;
}

void checkunready() {
	if (wait_queue != NULL) {
		process_t* tmp = wait_queue;
		while(tmp != NULL){
			if(getabstime(tmp->arrival_time) <= getabstime(&current_time)){
				process_t* tmp2 = dequeue(&wait_queue);
				enqueue(&ready_queue, tmp2);
				tmp = wait_queue;
			}else tmp=NULL;
		}
	}
}

int process_rt_create(void (*f)(void), int n, realtime_t *start, realtime_t *deadline) {
	process_t *proc2 = (process_t*) malloc(sizeof(process_t));
	if (!proc2) {
		return -1;
	}
	proc2->sp_old = process_stack_init(f,n);
	proc2->sp = proc2->sp_old;
	proc2->n = n;
	proc2->arrival_time = start;
	//proc2->deadline = &{deadline->sec + start->sec, deadline->msec + start->msec};
	proc2->deadline = deadline;
	proc2->deadline->msec = deadline->msec + start->msec;
	proc2->deadline->sec = deadline->sec + start->sec;
	proc2->is_rt = 1;
	if (proc2->deadline->msec >= 1000) {
		proc2->deadline->sec += 1;
		proc2->deadline->msec -= 1000;
	}
	
	if(!proc2->sp){
		return -1;
	}
	
	proc2->next = NULL;
	enqueue(&wait_queue, proc2);
	checkunready();
	return 0;
}

void process_start(void) {
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_EnableIRQ(PIT1_IRQn);
	NVIC_SetPriority(PIT0_IRQn, 2);
	NVIC_SetPriority(PIT1_IRQn, 0);
	NVIC_SetPriority(SVCall_IRQn, 1);
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0;
	PIT->CHANNEL[0].LDVAL = 26000;
	PIT->CHANNEL[0].TFLG = 1;
	PIT->CHANNEL[0].TCTRL = 0b10;
	
	PIT->CHANNEL[1].LDVAL = SystemCoreClock / 1000;
	PIT->CHANNEL[1].TFLG = 1;
	PIT->CHANNEL[1].TCTRL = 0b11;
	
	current_time.msec = 0;
	current_time.sec = 0;
	
	process_begin();
}

void PIT1_IRQHandler(void) {
	//__disable_irq();
	//PIT->CHANNEL[1].TFLG = 0x1;
	//PIT->CHANNEL[1].TCTRL &= ~(1<<0);
	current_time.msec = current_time.msec+1;
	
	if (current_time.msec >= 1000) {
		current_time.sec = current_time.sec+1;
		current_time.msec -= 1000;
	}
	PIT->CHANNEL[1].TFLG = 1;
	//__enable_irq();
}


void waiter(){
	__enable_irq();
	while(!process_queue && !ready_queue) {
		checkunready();
	}
	__disable_irq();
}

unsigned int *process_select(unsigned int *cursp)
{
	
  /* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
	 cursp will be NULL when first starting the scheduler, and when a process terminates
	 Return the stack pointer for the new process to run, or NULL to exit the scheduler.
*/
	__disable_irq();
	
	checkunready();
		
	if(!cursp){
		if(!current_process && !process_queue && !ready_queue && !wait_queue){
			return NULL;
		} else if (current_process != NULL) {
			if(current_process->is_rt){
				if(getabstime(current_process->deadline) >= getabstime(&current_time)) {
					process_deadline_met += 1;
				} else {
					process_deadline_miss += 1;
				}
			}if(ready_queue || process_queue){
				process_stack_free(current_process->sp_old, current_process->n);
				free(current_process);
			}
		}
	}	else {
		current_process->sp = cursp;
		if(current_process->is_rt) {
			enqueue(&ready_queue, current_process);
		} else {
			enqueue(&process_queue, current_process);
		}
	}
	if (wait_queue != NULL){
			waiter();
	}
	process_t *next_one;
	if (!ready_queue) {
		next_one = dequeue(&process_queue);
	} else {
		next_one = dequeue(&ready_queue);
	}
	if(!next_one){
		return NULL;
	} else {
		current_process = next_one;
		return current_process->sp;
	}
}

