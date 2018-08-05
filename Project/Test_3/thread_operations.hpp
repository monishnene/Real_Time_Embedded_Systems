/*
 * thrad_operations.hpp
 *
 *  Created on: August 4, 2018
 *      Author: monish
 * Create, join thread
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
