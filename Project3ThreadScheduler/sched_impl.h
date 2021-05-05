#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__

#include <semaphore.h>
#include <pthread.h>
#include "list.h"

struct thread_info 
{
	/*...Fill this in...*/
	list_elem_t	*queue_elem;
	struct sched_queue *queue;
	sem_t *sem_cpu, *sem_ready;
};

struct sched_queue 
{
	/*...Fill this in...*/
	list_t *list;
	sem_t *sem_release, *sem_admit;
	pthread_mutex_t *access_mutex;
};

#endif /* __SCHED_IMPL__H__ */
