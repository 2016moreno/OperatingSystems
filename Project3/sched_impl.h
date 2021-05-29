#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include "list.h"
#include "semaphore.h"

struct thread_info {
	/*...Fill this in...*/
	sched_queue_t* sched_q; 
	list_elem_t* wrkr_node;
	sem_t thread_lk; 
	pthread_t thread_id; 

};

struct sched_queue {
	/*...Fill this in...*/
	int size;
	list_t *wrkr_q; 
	list_elem_t* crntwrkr_node;
	sem_t cpu_lk, qsize_lk; 
	pthread_mutex_t	q_lk; 
};

#endif /* __SCHED_IMPL__H__ */
