#include "scheduler.h"
#include "sched_impl.h"
#include <stdlib.h>
#include <unistd.h>

/* Fill in your scheduler implementation code below: */

static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->q_e = (list_elem_t*) malloc(sizeof(list_elem_t));
	info->queue = queue;
	info->s_cpu = (sem_t*) malloc(sizeof(sem_t));
	info->s_ready = (sem_t*) malloc(sizeof(sem_t));
	sem_init(info->s_cpu, 0, 0);
	sem_init(info->s_ready, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	free(info->q_e);
	info->queue = NULL;
	sem_destroy(info->s_cpu);
	sem_destroy(info->s_ready);
	free(info->s_cpu);
	free(info->s_ready);
}

/*...More functions go here...*/

static void enter_sched_queue(thread_info_t *info)
{
    list_elem_init(info->q_e, info);
	sem_wait(info->queue->sadmit);
	pthread_mutex_lock(info->queue->mutex_un);
	list_insert_tail(info->queue->list, info->q_e);
	pthread_mutex_unlock(info->queue->mutex_un);
	sem_post(info->queue->srelease);
}

static void leave_sched_queue(thread_info_t *info)
{
	sem_wait(info->queue->srelease);
	pthread_mutex_lock(info->queue->mutex_un);
	list_remove_elem(info->queue->list, info->q_e);
	pthread_mutex_unlock(info->queue->mutex_un);
	sem_post(info->queue->sadmit);
}

static void wait_for_cpu(thread_info_t *info)
{
    sem_wait(info->s_ready);
}

static void release_cpu(thread_info_t *info)
{
    sem_post(info->s_cpu);
}

static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->list = (list_t*) malloc(sizeof(list_t));
	list_init(queue->list);

	queue->sadmit = (sem_t*) malloc(sizeof(sem_t));
	queue->srelease = (sem_t*) malloc(sizeof(sem_t));
	queue->mutex_un = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

	sem_init(queue->sadmit, 0, queue_size);
	sem_init(queue->srelease, 0, 0);
	pthread_mutex_init(queue->mutex_un, NULL);
}

static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	list_elem_t* elem;
	while((elem = list_get_head(queue->list)))
	{
		list_remove_elem(queue->list, elem);
		destroy_thread_info((thread_info_t*) elem->datum);
	}

	free(queue->list);
	sem_destroy(queue->sadmit);
	sem_destroy(queue->srelease);
	pthread_mutex_destroy(queue->mutex_un);

	free(queue->sadmit);
	free(queue->srelease);
	free(queue->mutex_un);
}

/*...More functions go here...*/

static void wake_up_worker(thread_info_t *info)
{
    sem_post(info->s_ready);
}

static void wait_for_worker(sched_queue_t *queue)
{
    if(!(list_get_head(queue->list))) 
	{
    	return;
	}

	pthread_mutex_lock(queue->mutex_un);
  	thread_info_t* qt = (thread_info_t*) (list_get_head(queue->list))->datum;
	pthread_mutex_unlock(queue->mutex_un);
	sem_wait(qt->s_cpu);
}

void wait_for_worker_rr(sched_queue_t *queue)
{
	list_elem_t* sched_qh;

	pthread_mutex_lock(queue->mutex_un);

	if (!(sched_qh = list_get_head(queue->list)))
	{
		pthread_mutex_unlock(queue->mutex_un);
		return;
	}

	thread_info_t* queue_thread = (thread_info_t*) sched_qh->datum;
	list_remove_elem(queue->list, sched_qh);
	list_insert_tail(queue->list, sched_qh);
	pthread_mutex_unlock(queue->mutex_un);
	sem_wait(queue_thread->s_cpu);
}

static thread_info_t* next_worker(sched_queue_t *queue)
{
	list_elem_t* sched_qh;
	thread_info_t* next_t = NULL;

	if ((sched_qh = list_get_head(queue->list)))
	{
		pthread_mutex_lock(queue->mutex_un);
		next_t = (thread_info_t*) sched_qh->datum;
		pthread_mutex_unlock(queue->mutex_un);
	}

	return next_t;
}

/* Block until at least one worker thread is in the scheduler queue. */
static void wait_for_queue(sched_queue_t *queue)
{
	sem_wait(queue->srelease);
	usleep(1000);
	sem_post(queue->srelease);
}

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue /*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue /*, ...etc... */ } };
