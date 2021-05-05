#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__
#include <semaphore.h>
#include <pthread.h>
#include "list.h"

struct thread_info 
{
	/*...Fill this in...*/
	struct sched_queue *queue;
	list_elem_t	*q_e;
	sem_t *s_cpu; 
	sem_t *s_ready;
};

struct sched_queue 
{
	/*...Fill this in...*/
	list_t *list;
	sem_t *srelease;
	sem_t *sadmit;
	pthread_mutex_t *mutex_un;
};

#endif /* __SCHED_IMPL__H__ */
