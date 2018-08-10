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
#include <stdbool.h>

#include <sys/param.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define MAX_PRIORITY  sched_get_priority_max(SCHED_FIFO)

#define OK (0)
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define TOTAL_THREADS 7
#define TOTAL_CAPTURES 10
#define THREADS_POST_TIME (1*NSEC_PER_MSEC)
#define SCHEDULER_FREQ 30
#define True 1
#define False 0
#define ON 0
#define OFF 1
#define VRES 480
#define HRES 640

static uint32_t seconds_since_start=0;
static uint8_t thread_count=0,error=0,loop_condition=True;
static uint8_t thread_frequency_array[TOTAL_THREADS]={0,1,1,1,0,0,0};
sem_t sem_print_time_logs;

//opencv Declarations
//IplImage* frame;
//CvCapture* camera = cvCaptureFromCAM(CV_CAP_ANY);
uint8_t* image_pointer;

typedef struct
{	
	uint8_t thread_frequency = thread_frequency_array[thread_count];
	uint8_t priority = MAX_PRIORITY - thread_count++;
	uint8_t thread_id = thread_count;
	uint8_t times_exe_per_sec=0;
	uint8_t thread_live = False;
	uint8_t exit_condition = False;
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
	double WCET_ms=0;
	double average_difference_ms=0;
	double accumulated_jitter_ms=0;
	double average_jitter_ms=0;
	struct timespec delta_time={0,0};
}thread_properties;

uint8_t title_1[]="\nSequencer\n";
uint8_t title_2[]="\nCapture image.\n";
uint8_t title_3[]="\nSave PPM image.\n";
uint8_t title_4[]="\nJPEG compression of PPM image\n";
uint8_t title_5[]="\nSquirtle\n";
uint8_t title_6[]="\nPrimeape\n";
uint8_t title_7[]="\nSnorlax\n";

static struct timespec code_start_time,code_end_time,code_execution_time;
static thread_properties func_props[TOTAL_THREADS];

//Function Prototypes
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
	struct timespec fetched_time;
	clock_gettime(CLOCK_REALTIME, &fetched_time);
	delta_t(&fetched_time, &code_start_time, &(timeptr->delta_time));
  	timeptr->start_ms = double(timeptr->delta_time.tv_sec*NSEC_PER_SEC + timeptr->delta_time.tv_nsec);
	return;
}

/***********************************************************************
  * @brief jitter_difference_end()
  * Measure stop time of a thread and calculate execution time
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void jitter_difference_end(thread_properties * timeptr)
{
  	struct timespec fetched_time,delta_2_time;
	double prev_average_difference = 0;
	clock_gettime(CLOCK_REALTIME, &fetched_time);
	delta_t(&fetched_time, &code_start_time, &delta_2_time);
  	timeptr->stop_ms = double(delta_2_time.tv_sec*NSEC_PER_SEC + delta_2_time.tv_nsec);
	timeptr->difference_ms = double(timeptr->stop_ms - timeptr->start_ms);
	if(timeptr->WCET_ms < timeptr->difference_ms)
	{
		timeptr->WCET_ms = timeptr->difference_ms;
	}
	timeptr->counter++;
	if(timeptr->counter > 1)
	{
		timeptr->accumulated_jitter_ms += double(timeptr->difference_ms - timeptr->average_difference_ms);
		timeptr->average_jitter_ms = double(timeptr->accumulated_jitter_ms/ timeptr->counter);
		timeptr->average_difference_ms = double((timeptr->average_difference_ms*(timeptr->counter-1)+timeptr->difference_ms)/timeptr->counter);
	}
	else
	{
		timeptr->average_difference_ms = timeptr->difference_ms;
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
	sem_wait(&sem_print_time_logs);
	cout<<timeptr->title;
	printf("Time since start of code = %ld seconds %ld nanoseconds\n",timeptr->delta_time.tv_sec,timeptr->delta_time.tv_nsec);
	printf("Thread %d Frequency = %d Hz\n",timeptr->thread_id,timeptr->times_exe_per_sec);
	printf("Thread %d starts at %f ns\n",timeptr->thread_id,timeptr->start_ms);
	printf("Thread %d stops  at %f ns\n",timeptr->thread_id,timeptr->stop_ms);
	printf("Thread %d average execution time %f ns\n",timeptr->thread_id,timeptr->average_difference_ms);
	printf("Thread %d worst case execution time %f ns\n",timeptr->thread_id,timeptr->WCET_ms);
	printf("Thread %d accumulated jitter %f ns\n",timeptr->thread_id,timeptr->accumulated_jitter_ms);
	printf("Thread %d average jitter %f ns\n",timeptr->thread_id,timeptr->average_jitter_ms);
	sem_post(&sem_print_time_logs);
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
	if(iterations++==TOTAL_CAPTURES)
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
	uint8_t func_id=0,i=0,j=0;
	uint32_t time_difference=0,value=0,prev_sec=1;
	struct timespec time_check;
	while(loop_condition)
	{	
		clock_gettime(CLOCK_REALTIME,&code_end_time);
		if((code_end_time.tv_nsec > THREADS_POST_TIME)&&(code_end_time.tv_nsec < 2*THREADS_POST_TIME)&&(code_end_time.tv_sec != prev_sec))
		{
			loop_condition_check();
			seconds_since_start++;
			prev_sec=code_end_time.tv_sec;
			for(j=1;j<TOTAL_THREADS;j++)
			{
				func_props[j].times_exe_per_sec=0;
			}
			for(i=0;i<SCHEDULER_FREQ;i++)
			{
				for(j=1;j<TOTAL_THREADS;j++)
				{
					if(func_props[j].thread_frequency > 0)
					{
						if(func_props[j].thread_frequency > SCHEDULER_FREQ)
						{
							func_props[j].thread_frequency=SCHEDULER_FREQ;
						}
						if((i%(SCHEDULER_FREQ/func_props[j].thread_frequency)==0)&&(func_props[j].times_exe_per_sec < func_props[j].thread_frequency))
						{
							func_props[j].times_exe_per_sec++;
							func_props[j].thread_live=True;
							sem_post(&(func_props[j].sem));
							func_props[j].thread_live=False;
						}
					}
				}
			}
		}		
		sem_post(&(func_props[0].sem));		
	}
	for(j=1;j<TOTAL_THREADS;j++)
	{
		func_props[j].exit_condition=True;
		sem_post(&(func_props[j].sem));
	}
	pthread_exit(NULL);
}

void* func_2(void* ptr)
{
	uint8_t func_id=1;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			//camera = (CvCapture *)cvCreateCameraCapture(ON);
			frame = cvQueryFrame(camera); 
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

void* func_3(void* ptr)
{
	//Mat frame_2 ;
	uint8_t func_id=2;
	ostringstream file_name;
    	vector<int> ppm_settings;
    	ppm_settings.push_back(CV_IMWRITE_PXM_BINARY);
    	ppm_settings.push_back(1);
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			file_name.str("");	
			file_name<<func_props[func_id].counter<<".ppm";
			//imwrite(file_name.str(),frame_2,ppm_settings);
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

void* func_4(void* ptr)
{
	uint8_t func_id=3;
	ostringstream file_name;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			file_name.str("");	
			file_name<<func_props[func_id].counter<<".jpg";
			//cvSaveImage(file_name.str(),frame);
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

void* func_5(void* ptr)
{
	uint8_t func_id=4;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			function_end(func_id);
		}	
	}
	pthread_exit(NULL);
}

void* func_6(void* ptr)
{
	uint8_t func_id=5;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

void* func_7(void* ptr)
{
	uint8_t func_id=6;
	while(loop_condition|func_props[func_id].thread_live)
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

void struct_settings(void)
{
	func_props[0].title=title_1;
	func_props[1].title=title_2;
	func_props[2].title=title_3;
	func_props[3].title=title_4;
	func_props[4].title=title_5;
	func_props[5].title=title_6;
	func_props[6].title=title_7;
	func_props[0].function_pointer = func_1; 
	func_props[1].function_pointer = func_2; 
	func_props[2].function_pointer = func_3; 
	func_props[3].function_pointer = func_4;
	func_props[4].function_pointer = func_5;
	func_props[5].function_pointer = func_6;
	func_props[6].function_pointer = func_7;
}

void sem_settings(void)
{
	sem_post(&(func_props[0].sem));
	sem_init(&sem_print_time_logs,0,1);
}

/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	clock_gettime(CLOCK_REALTIME,&code_start_time);
	//cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, HRES);
	//cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, VRES);
	struct_settings();
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
	sem_settings();
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_join(&func_props[i]);
	}
	cvReleaseCapture(&camera);
	clock_gettime(CLOCK_REALTIME,&code_end_time); 
	delta_t(&code_end_time, &code_start_time, &code_execution_time);
	cout<<"\n\nCode Execution Time = "<<code_execution_time.tv_sec<<" seconds "<<code_execution_time.tv_nsec<<" nano seconds.\n";
}
