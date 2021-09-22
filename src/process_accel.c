#include "3140_concur.h"
#include "shared_structs.h"

/* the currently running process. current_process must be NULL if no process is running,
    otherwise it must point to the process_t of the currently running process
*/
process_t * current_process = NULL; 
process_t * process_queue = NULL;
process_t * process_tail = NULL;
realtime_t current_time = {0,0};
int process_deadline_met=0;
int process_deadline_miss=0;

static process_t * pop_front_process() {
	if (!process_queue) return NULL;
	process_t *proc = process_queue;
	process_queue = proc->next;
	if (process_tail == proc) {
		process_tail = NULL;
	}
	proc->next = NULL;
	return proc;
}

static void push_tail_process(process_t *proc) {
	if (!process_queue) {
		process_queue = proc;
	}
	if (process_tail) {
		process_tail->next = proc;
	}
	process_tail = proc;
	proc->next = NULL;
}

static void process_free(process_t *proc) {
	process_stack_free(proc->sp_old, proc->n);
	free(proc);
}


void waiter(void) {
	while(process_queue==NULL);
}
/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/
unsigned int * process_select (unsigned int * cursp) {
	if (cursp) {
		// Suspending a process which has not yet finished, save state and make it the tail
		current_process->sp = cursp;
		push_tail_process(current_process);
	} else {
		// Check if a process was running, free its resources if one just finished
		if (current_process) {
			process_free(current_process);
		}
	}
	
	// Select the new current process from the front of the queue
	current_process = pop_front_process();
	
	if (current_process) {
		// Launch the process which was just popped off the queue
		return current_process->sp;
	} else {
		// No process was selected, exit the scheduler
		waiter();
	}
}

/* Starts up the concurrent execution */
void process_start (void) {
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0;
	PIT->CHANNEL[0].LDVAL = SystemCoreClock/10;
	PIT->CHANNEL[0].TFLG = 1;
	PIT->CHANNEL[0].TCTRL = 0b10;

	PIT->CHANNEL[1].LDVAL = SystemCoreClock / 1000;
	PIT->CHANNEL[1].TFLG = 1;
	PIT->CHANNEL[1].TCTRL = 0b11;
	
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_EnableIRQ(PIT1_IRQn);
	NVIC_SetPriority(PIT0_IRQn, 2);
	NVIC_SetPriority(PIT1_IRQn, 0);
	NVIC_SetPriority(SVCall_IRQn, 1);
		
	
	// Don't enable the timer yet. The scheduler will do so itself
	
	// Bail out fast if no processes were ever created
	if (!process_queue) return;
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
	if(current_process->is_rt) {
		if (current_time.sec*1000 + current_time.msec > current_process->deadline->msec + current_process->deadline->sec){
			process_deadline_miss = 1;
		}
	}
	PIT->CHANNEL[1].TFLG = 1;
	//__enable_irq();
}

/* Create a new process */
int process_create (void (*f)(void), int n) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc->sp = proc->sp_old = sp;
	proc->n = n;
	proc->is_rt = 0;
	
	push_tail_process(proc);
	return 0;
}

int process_rt_create(void (*f)(void), int n, realtime_t* start, realtime_t* deadline){
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc->sp = proc->sp_old = sp;
	proc->n = n;
	proc->is_rt = 1;
	proc->arrival_time->sec= start->sec;
	proc->arrival_time->msec = start->msec;
	proc->deadline->sec= start->sec + deadline->msec;
	proc->deadline->msec = start->msec + deadline->msec;
	
	push_tail_process(proc);
	return 0;
}