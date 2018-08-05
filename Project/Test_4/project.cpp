/*
 * project.cpp
 *
 *  Created on: August 4, 2018
 *      Author: monish
 * Main, initializations and primary functions
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

#define MAX_PRIORITY  sched_get_priority_max(SCHED_FIFO)

#define OK (0)
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define TOTAL_THREADS 7
#define TOTAL_CAPTURES 2000
#define True 1
#define False 0

static uint8_t thread_count=0,error=0,loop_condition=True;

typedef struct
{
	uint8_t priority = MAX_PRIORITY - thread_count++;
	uint8_t thread_id = thread_count;
	pthread_t thread;
	sem_t sem;
	pthread_attr_t attribute;
	struct sched_param parameter;
	void*(*function_pointer)(void*);
	uint8_t* title;
	uint32_t counter=0;
	double start_ms=0;
	double stop_ms=0;
	double difference_ms=0;
	double accumulated_jitter_ms=0;
	double average_jitter_ms=0;
}thread_properties;

static struct timespec code_start_time;

static thread_properties func_props[TOTAL_THREADS];

void delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t);
void jitter_difference_start(thread_properties * timeptr);
void jitter_difference_end(thread_properties * timeptr);
void print_time_logs(thread_properties * timeptr);
void function_end(uint8_t func_id);
void function_beginning(uint8_t func_id);
void loop_condition_check(void);
void thread_create(thread_properties* struct_pointer);
void thread_join(thread_properties* struct_pointer);


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



void* func_1(void* ptr)
{
	uint8_t func_id=0;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rPikachu\n\r");
		loop_condition_check();
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_2(void* ptr)
{
	uint8_t func_id=1;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rLapras\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_3(void* ptr)
{
	uint8_t func_id=2;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rBulbasaur\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_4(void* ptr)
{
	uint8_t func_id=3;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rCharmander\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_5(void* ptr)
{
	uint8_t func_id=4;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rSquirtle\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_6(void* ptr)
{
	uint8_t func_id=5;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rPrimeape\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_7(void* ptr)
{
	uint8_t func_id=6;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rSnorlax\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}


/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	clock_gettime(CLOCK_REALTIME,&code_start_time); 
	func_props[0].function_pointer = func_1; 
	func_props[1].function_pointer = func_2; 
	func_props[2].function_pointer = func_3; 
	func_props[3].function_pointer = func_4;
	func_props[4].function_pointer = func_5;
	func_props[5].function_pointer = func_6;
	func_props[6].function_pointer = func_7;
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_create(&func_props[i]);
	}
	sem_post(&(func_props[0].sem));
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_join(&func_props[i]);
	}
}
