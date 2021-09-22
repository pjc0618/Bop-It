#ifndef __SHARED_STRUCTS_H__
#define __SHARED_STRUCTS_H__


/** Implement your structs here */
//0 if all deadlines so far met; 1 if any deadlines missed
typedef struct {
	unsigned int sec;
	unsigned int msec;
} realtime_t;

// The current time relative to process_start
extern realtime_t current_time;

// The number of processes that have terminated before or after their deadline, respectively.
extern int process_deadline_met;
extern int process_deadline_miss;

/* Create a new realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_create(void (*f)(void), int n, realtime_t* start, realtime_t* deadline);

/* Create a new periodic realtime process out of the function f with the given parameters.
 * Returns -1 if unable to malloc a new process_t, 0 otherwise.
 */
int process_rt_periodic(void (*f)(void), int n, realtime_t *start, realtime_t *deadline, realtime_t *period);
/**
 * This structure holds the process structure information
 */
struct process_state {
	/** stack pointer at last checkpoint */
	unsigned int *sp;
	/** needed for freeing properly */
	unsigned int *sp_old;
	int n;
	/** pointer to next process in queue */
	struct process_state* next;
	/** the arrival time of a realtime process */
	realtime_t* arrival_time;
	/** the amount of time a realtime process has to complete */
	realtime_t* deadline;
	/** whether a process is a realtime process or not; 
	0 if it isn't, 1 if it is*/
	int is_rt;
};
//gets the time in milliseconds
int getabstime(realtime_t* rt);
#endif
