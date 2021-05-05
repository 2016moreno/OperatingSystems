#include <stdlib.h>
#include <unistd.h>
#include "scheduler.h"
#include "sched_impl.h"
/* Fill in your scheduler implementation code below: */

static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->queue_elem = (list_elem_t*) malloc(sizeof(list_elem_t));
	info->queue = queue;

	info->sem_cpu = (sem_t*) malloc(sizeof(sem_t));
	info->sem_ready = (sem_t*) malloc(sizeof(sem_t));

	sem_init(info->sem_cpu, 0, 0);
	sem_init(info->sem_ready, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	free(info->queue_elem);
	info->queue = NULL;

	sem_destroy(info->sem_cpu);
	sem_destroy(info->sem_ready);

	free(info->sem_cpu);
	free(info->sem_ready);
}

/*...More functions go here...*/

static void enter_sched_queue(thread_info_t *info)
{
	list_elem_init(info->queue_elem, info);

	sem_wait(info->queue->sem_admit);

	pthread_mutex_lock(info->queue->access_mutex);
	list_insert_tail(info->queue->list, info->queue_elem);
	pthread_mutex_unlock(info->queue->access_mutex);

	sem_post(info->queue->sem_release);
}

static void leave_sched_queue(thread_info_t *info)
{
	sem_wait(info->queue->sem_release);

	pthread_mutex_lock(info->queue->access_mutex);
	list_remove_elem(info->queue->list, info->queue_elem);
	pthread_mutex_unlock(info->queue->access_mutex);

	sem_post(info->queue->sem_admit);
}

static void wait_for_cpu(thread_info_t *info)
{
	sem_wait(info->sem_ready);
}

static void release_cpu(thread_info_t *info)
{
	sem_post(info->sem_cpu);
}

static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->list = (list_t*) malloc(sizeof(list_t));
	list_init(queue->list);

	queue->sem_admit = (sem_t*) malloc(sizeof(sem_t));
	queue->sem_release = (sem_t*) malloc(sizeof(sem_t));

	queue->access_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

	sem_init(queue->sem_admit, 0, queue_size);
	sem_init(queue->sem_release, 0, 0);
	pthread_mutex_init(queue->access_mutex, NULL);
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

	sem_destroy(queue->sem_admit);
	sem_destroy(queue->sem_release);
	pthread_mutex_destroy(queue->access_mutex);

	free(queue->sem_admit);
	free(queue->sem_release);
	free(queue->access_mutex);
}

/*...More functions go here...*/

static void wake_up_worker(thread_info_t *info)
{
	sem_post(info->sem_ready);
}

void wait_for_worker_fifo (sched_queue_t *queue)
{
  	if(!(list_get_head(queue->list))) {
    	return;
	}

	pthread_mutex_lock(queue->access_mutex);
  	thread_info_t* queue_thread = (thread_info_t*) (list_get_head(queue->list))->datum;
	pthread_mutex_unlock(queue->access_mutex);

	sem_wait(queue_thread->sem_cpu);
}

void wait_for_worker_rr (sched_queue_t *queue)
{
	list_elem_t* sched_queue_head;

	pthread_mutex_lock(queue->access_mutex);

	if (!(sched_queue_head = list_get_head(queue->list)))
	{
		pthread_mutex_unlock(queue->access_mutex);
		return;
	}

	thread_info_t* queue_thread = (thread_info_t*) sched_queue_head->datum;
	list_remove_elem(queue->list, sched_queue_head);
	list_insert_tail(queue->list, sched_queue_head);
	pthread_mutex_unlock(queue->access_mutex);

	sem_wait(queue_thread->sem_cpu);
}

static thread_info_t* next_worker(sched_queue_t *queue)
{
	list_elem_t* sched_queue_head;
	thread_info_t* next_thread = NULL;

	if ((sched_queue_head = list_get_head(queue->list)))
	{
		pthread_mutex_lock(queue->access_mutex);
		next_thread = (thread_info_t*) sched_queue_head->datum;
		pthread_mutex_unlock(queue->access_mutex);
	}
	return next_thread;
}

static void wait_for_queue(sched_queue_t *queue)
{
	sem_wait(queue->sem_release);
	usleep(1000);
	sem_post(queue->sem_release);
}

static void wait_for_queue(sched_queue_t *queue)
{
	sem_wait(queue->sem_release);
	usleep(1000);
	sem_post(queue->sem_release);
}

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue /*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue /*, ...etc... */ } };
