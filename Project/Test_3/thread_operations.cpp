/*
 * thrad_operations.cpp
 *
 *  Created on: August 4, 2018
 *      Author: monish
 * Create, join thread
 */

#include "thread_operations.hpp"

/***********************************************************************
  * @brief delta_t()
  * Find the difference between two timespec structures eg a - b =c 
  * @param struct timespec *stop (a)
  * @param struct timespec *start (b)
  * @param struct timespec *delta_t (c)
  * @return success or fail
  ***********************************************************************/
void delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  return;
}

/***********************************************************************
  * @brief jitter_difference_start()
  * Measure start time of a thread 
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void jitter_difference_start(thread_properties * timeptr)
{
	struct timespec fetched_time,delta_time;
	clock_gettime(CLOCK_REALTIME, &fetched_time);
	delta_t(&fetched_time, &code_start_time, &delta_time);
  	timeptr->start_ms = double(delta_time.tv_sec*NSEC_PER_SEC + delta_time.tv_nsec);
	return;
}


/***********************************************************************
  * @brief jitter_difference_end()
  * Measure stop time of a thread and calculate execution time
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void jitter_difference_end(thread_properties * timeptr)
{
  	struct timespec fetched_time,delta_time;
	double prev_difference = 0;
	clock_gettime(CLOCK_REALTIME, &fetched_time);
	delta_t(&fetched_time, &code_start_time, &delta_time);
  	timeptr->stop_ms = double(delta_time.tv_sec*NSEC_PER_SEC + delta_time.tv_nsec);
	prev_difference = timeptr->difference_ms;
	timeptr->difference_ms = double(timeptr->stop_ms - timeptr->start_ms);
	timeptr->counter++;
	if(timeptr->counter > 1)
	{
		timeptr->accumulated_jitter_ms += double(timeptr->difference_ms - prev_difference);
		timeptr->average_jitter_ms = double(timeptr->accumulated_jitter_ms/ timeptr->counter);
	}	
	return; 	
}

/**********************************************************************
  * @brief print_time_logs()
  * print start time, stop time, execution time, jitter and fps logs for various threads 
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void print_time_logs(thread_properties * timeptr)
{
	printf("Thread %d starts at %f ns\n",timeptr->thread_id,timeptr->start_ms);
	printf("Thread %d stops  at %f ns\n",timeptr->thread_id,timeptr->stop_ms);
	printf("Thread %d Duration %f ns\n",timeptr->thread_id,timeptr->difference_ms);
	printf("Thread %d accumulated jitter %f ns\n",timeptr->thread_id,timeptr->accumulated_jitter_ms);
	printf("Thread %d average jitter %f ns\n",timeptr->thread_id,timeptr->average_jitter_ms);
}

void function_end(uint8_t func_id)
{
	jitter_difference_end(&func_props[func_id]);
	print_time_logs(&func_props[func_id]);
	if(func_id == TOTAL_THREADS-1)
	{
		func_id = 0;
	}
	else
	{
		func_id++;
	}
	sem_post(&(func_props[func_id].sem));
	return;
}

void function_beginning(uint8_t func_id)
{
	sem_wait(&(func_props[func_id].sem));		
	jitter_difference_start(&func_props[func_id]);
	return;
}

void loop_condition_check(void)
{
	static uint32_t iterations=0;
	if(++iterations==TOTAL_CAPTURES)
	{
		loop_condition=False;
	}
	return;
}	

void thread_create(thread_properties* struct_pointer)
{	
	pthread_attr_init(&(struct_pointer->attribute));
	pthread_attr_setinheritsched(&(struct_pointer->attribute),PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&(struct_pointer->attribute),SCHED_FIFO);
	struct_pointer->parameter.sched_priority = struct_pointer->priority;
	sem_init(&(struct_pointer->sem),0,0);
	pthread_attr_setschedparam(&(struct_pointer->attribute), &(struct_pointer->parameter));
	if(pthread_create(&(struct_pointer->thread), &(struct_pointer->attribute), struct_pointer->function_pointer, NULL)==0)
		printf("thread %d created\n\r",struct_pointer->thread_id);
  	else printf("thread %d creation failed\n\r",struct_pointer->thread_id);
} 

void thread_join(thread_properties* struct_pointer)
{
	pthread_join(struct_pointer->thread,NULL);
}

