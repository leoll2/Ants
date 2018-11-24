#include <stdio.h>
#include <string.h>

#include "rt_thread.h"


task_par rt_threads[MAX_THREADS];
unsigned int active_rt_threads = 0;
pthread_mutex_t rt_threads_mtx = PTHREAD_MUTEX_INITIALIZER;


void time_copy(struct timespec *dst, struct timespec src) {
	
	dst->tv_sec  = src.tv_sec;
	dst->tv_nsec = src.tv_nsec;
}


void time_add_ms(struct timespec *t, int ms) {

	t->tv_sec += ms/1000;
	t->tv_nsec += (ms%1000)*1000000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}


int time_cmp(struct timespec t1, struct timespec t2) {
	
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}


/* Initialize thread pool data structures*/
void init_rt_thread_manager(void) {

	pthread_mutex_lock(&rt_threads_mtx);

	active_rt_threads = 0;

	for (int i = 0; i < MAX_THREADS; ++i) {
		rt_threads[i].in_use = false;
	}

	pthread_mutex_unlock(&rt_threads_mtx);
}



/* Allocates a previously unused task_par structure and returns its index.
*  If none is free, return MAX_THREADS.
*/
unsigned int allocate_task_id(void) {

	unsigned int i = 0;

	pthread_mutex_lock(&rt_threads_mtx);

	if (active_rt_threads == MAX_THREADS) {
		pthread_mutex_unlock(&rt_threads_mtx);
		return MAX_THREADS;
	}

	while (rt_threads[i].in_use)
		++i;

	rt_threads[i].in_use = true;
	pthread_mutex_init(&rt_threads[i].mtx, NULL);
	++active_rt_threads;

	pthread_mutex_unlock(&rt_threads_mtx);
	return i;
}


/* Deallocates the task_par of a finished (by assumption) thread. */
void deallocate_task_id(unsigned int id) {

	pthread_mutex_lock(&rt_threads_mtx);
	--active_rt_threads;
	rt_threads[id].in_use = false;
	pthread_mutex_destroy(&rt_threads[id].mtx);
	pthread_mutex_unlock(&rt_threads_mtx);
}

/* Initialize pthread_attr_t structure */
int init_sched_attr(pthread_attr_t *attr, int policy, int prio) {

	struct sched_param sp;
	int err;

	err = pthread_attr_init(attr);
	if (err)
		return 1;

	err = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (err)
		return 2;

	err = pthread_attr_setschedpolicy(attr, policy);
	if (err)
		return 3;

	sp.sched_priority = prio;
	err = pthread_attr_setschedparam(attr, &sp);
	if (err)
		return 4;

	return 0;
}


void set_activation(task_par *tp) {

	struct timespec t;

	pthread_mutex_lock(&tp->mtx);
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->deadline);
	pthread_mutex_unlock(&tp->mtx);
}


bool missed_deadline(task_par *tp) {

	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(now, tp->dl) > 0) {
		++tp->dl_missed;
		return true;
	}
	return false;
}


void wait_next_activation(task_par *tp) {

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp->at), NULL);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->period);
}


void *rt_thr_body(void *arg) {

	struct timespec t;
	task_par *tp = (task_par *)arg;
	int tid = tp - rt_threads;

	set_activation(tp);

	while (true) {
		pthread_mutex_lock(&tp->mtx);

		// Stop the thread if requested
		if (tp->stopped) {
			pthread_mutex_unlock(&tp->mtx);
			break;
		}
		// Execute the instance-specific code
		tp->behaviour(tp->data);

		// Check for deadline miss
		if (missed_deadline(tp))
			printf("Deadline missed. Total for this thread: %d\n", tp->dl_missed);

		pthread_mutex_unlock(&tp->mtx);

		// Sleep until next activation
		wait_next_activation(tp);
	}
	printf("Shutting down thread with id #%d\n", tid);
	return NULL;
}



/* Starts a new real-time thread. 
*  Returns a unique index identifying the thread, or -1 in case of error.
*/
int start_thread(
		void *(*func)(void *),
		void *args,
		int policy,
		long wcet,
		int prd,
		int dl,
		int prio)
{
	pthread_attr_t attr;
	task_par *tp;
	unsigned int id;
	int ret;

	id = allocate_task_id();
	if (id == MAX_THREADS)
		return -1;

	tp = &rt_threads[id];
	pthread_mutex_lock(&tp->mtx);

	tp->behaviour = func;
	tp->data = args;
	tp->wcet = wcet;
	tp->period = prd;
	tp->deadline = dl;
	tp->priority = prio;
	tp->stopped = false;
	tp->dl_missed = 0;

	ret = init_sched_attr(&attr, policy, prio);
	if (ret) {
		printf("Init of sched attributes failed (error: %d)\n", ret);
		goto error;
	}

	ret = pthread_create(&tp->tid, &attr, rt_thr_body, (void*)tp);
	if (ret) {
		printf("Thread creation failed (error: %s)\n", strerror(ret));
		goto error;
	}

	ret = pthread_attr_destroy(&attr);
	if (ret) {		// no corrective actions are actually necessary
		printf("Destruction of sched attributes failed (error: %d)\n", ret);
	}

	pthread_mutex_unlock(&tp->mtx);
	return id;

error:
	pthread_mutex_unlock(&tp->mtx);
	deallocate_task_id(id);
	return -1;
}


int stop_thread(unsigned int id) {

	if (id >= MAX_THREADS)
		return -1;

	if (!rt_threads[id].in_use) {
		return -1;
	}

	pthread_mutex_lock(&rt_threads[id].mtx);
	rt_threads[id].stopped = true;
	pthread_mutex_unlock(&rt_threads[id].mtx);
	pthread_join(rt_threads[id].tid, NULL);

	// mutex on rt_threads[i] not needed anymore, since the thread finished

	deallocate_task_id(id);

	return 0;
}