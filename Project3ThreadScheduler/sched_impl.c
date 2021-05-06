#include <stdlib.h>
#include "scheduler.h"
#include "sched_impl.h"
#include "pthread.h"
#include "semaphore.h"
#include "list.h"

/* Fill in your scheduler implementation code below: */

static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->sched_q = queue;
	sem_init(&info->thread_lk, 0, 0);
}

static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->sched_q = NULL;
	info->wrkr_node = NULL;
}

static void enter_sched_queue(thread_info_t *info)
{
	sem_wait(&info->sched_q->qsize_lk);
	sched_queue_t* sched_q = info->sched_q;

	pthread_mutex_lock(&sched_q->q_lk);

	list_elem_t* elem = (list_elem_t*) malloc(sizeof(list_elem_t));
	elem->datum = (void*) info;
	list_insert_tail(sched_q->wrkr_q, elem);
	info->wrkr_node = list_get_tail(sched_q->wrkr_q);

	pthread_mutex_unlock(&sched_q->q_lk);
}
static void leave_sched_queue(thread_info_t *info)
{
	pthread_mutex_lock(&info->sched_q->q_lk);

	list_remove_elem(info->sched_q->wrkr_q, info->wrkr_node);
	info->sched_q->crntwrkr_node = NULL;
	sem_post(&info->sched_q->qsize_lk);

	pthread_mutex_unlock(&info->sched_q->q_lk);
}
static void wait_for_cpu(thread_info_t *info)
{
	sem_wait(&info->thread_lk);
}

static void release_cpu(thread_info_t *info)
{	
	sem_post(&info->sched_q->cpu_lk);
}

/*...More functions go here...*/
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->wrkr_q = (list_t*) malloc(sizeof(list_t));
	list_init(queue->wrkr_q);
	queue->size = queue_size;
	queue->crntwrkr_node = NULL;
	sem_init(&queue->cpu_lk, 0, 0);
	sem_init(&queue->qsize_lk, 0, queue_size);
	pthread_mutex_init(&queue->q_lk, NULL);
}

static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	list_elem_t *curr;
	while((curr = list_get_head(queue->wrkr_q)) != NULL)
	{
		list_remove_elem(queue->wrkr_q, curr);
		free(curr);
	}
}

static void wake_up_worker(thread_info_t *info)
{
	sem_post(&info->thread_lk);
}

static void wait_for_worker(sched_queue_t *queue)
{
	sem_wait(&queue->cpu_lk);
}

static thread_info_t * next_worker_fifo(sched_queue_t *queue)
{
	pthread_mutex_lock(&queue->q_lk);

	list_elem_t* head = list_get_head(queue->wrkr_q);
	if(head == NULL)
	{
		pthread_mutex_unlock(&queue->q_lk);
		return NULL;
	}
	thread_info_t * next_worker_fifo = (thread_info_t *)(head->datum);
	pthread_mutex_unlock(&queue->q_lk);
	
	return next_worker_fifo;
}

static thread_info_t * next_worker_rr(sched_queue_t *queue)
{

	pthread_mutex_lock(&queue->q_lk);

	if(queue->crntwrkr_node != NULL)
	{
		list_remove_elem(queue->wrkr_q, queue->crntwrkr_node);
		list_insert_tail(queue->wrkr_q, queue->crntwrkr_node);
	}

	queue->crntwrkr_node = list_get_head(queue->wrkr_q);

	if(queue->crntwrkr_node == NULL)
	{
		pthread_mutex_unlock(&queue->q_lk);
		return NULL;
	}

	thread_info_t * next_worker_fifo = (thread_info_t *)(queue->crntwrkr_node->datum);

	pthread_mutex_unlock(&queue->q_lk);
	return next_worker_fifo;
}

static void wait_for_queue(sched_queue_t *queue)
{
	while(!list_size(queue->wrkr_q))
	{
		sched_yield();
	}
}

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu/*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_fifo, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu, /*...etc...*/  }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
