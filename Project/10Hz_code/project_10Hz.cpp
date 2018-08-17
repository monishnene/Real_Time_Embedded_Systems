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
#include <syslog.h>

#include <sys/param.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <fstream>

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
#define TOTAL_THREADS 3
#define TOTAL_CAPTURES 1800
#define THREADS_POST_TIME (1*NSEC_PER_MSEC)
#define SCHEDULER_FREQ 3
#define FREQUENCY 10
#define True 1
#define False 0
#define ON 0
#define OFF 1
#define VRES 480
#define HRES 640
#define SYSLOG_PRIORITY (MAX_PRIORITY-TOTAL_THREADS)

static uint32_t seconds_since_start=0;
static uint8_t thread_count=0,error=0,loop_condition=True;
static uint8_t thread_frequency_array[TOTAL_THREADS]={0,1,1};

sem_t sem_capture;
sem_t sem_ppm;
sem_t sem_capture_done;
ostringstream file_name;

time_t raw_time;
struct tm* time_info;

vector<int> ppm_settings;
vector<int> jpg_settings;
Mat frame(VRES,HRES,CV_8UC3);
VideoCapture camera(0);

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

static struct timespec code_start_time,code_end_time,code_execution_time,time_check;
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
 * @brief number string()
 * This function is used to convert number to string
 * @param number to be printed
 * @param display_width number of digits to be displayed
 * @param str pointer to store output str
***********************************************************************/
void number_string(int32_t number,uint8_t display_width,uint8_t* str)
{
	uint8_t temp_data[10];
    	uint8_t* temp_str=temp_data;
	int8_t counter=0;
	uint32_t value_check=0;
    	for(counter=display_width;counter>1;counter--)
    	{
        	switch(counter)
        	{
            		case 4:
            		{
                		value_check = 999;
                		if(number<=value_check)
                		{
                    			*(str++) = '0';
                		}
                		break;
            		}

            		case 3:
            		{
                		value_check = 99;
                		if(number<=value_check)
                		{
                    			*(str++) = '0';
                		}
                		break;
            		}

            		case 2:
            		{
                		value_check = 9;
                		if(number<=value_check)
                		{
                   	 		*(str++) = '0';
                		}
                		break;
            		}
        	}
	}
	counter=0;
	do
	{
		*(++temp_str)='0'+ number%10;
		number/=10;
		counter++;
	}while(number>0);
	for(;counter>0;counter--)
	{
		*(str++) = *(temp_str--);
	}
	*(str)=0;
	return;
}


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
  * @brief function_end()
  * Code to be executed at the end of all sequencer functions
  * @param func_id array index of the function that called this function
  ***********************************************************************/
void function_end(uint8_t func_id)
{
	func_props[func_id].counter++;
	return;
}

/***********************************************************************
  * @brief function_beginning()
  * Code to be executed at the beginning of all sequencer functions
  * @param func_id array index of the function that called this function
  ***********************************************************************/
void function_beginning(uint8_t func_id)
{
	sem_wait(&(func_props[func_id].sem));		
	return;
}

/***********************************************************************
  * @brief loop_condition_check()
  * Check frames captured and exit loop if Total frames captured
  ***********************************************************************/
void loop_condition_check(void)
{
	static uint32_t iterations=0;
	if(iterations++==TOTAL_CAPTURES)
	{
		loop_condition=False;
	}
	return;
}	

/***********************************************************************
  * @brief thread_create()
  * create threads according to thread properties.
  * param struct_pointer for the thread properties for thread to be created.
  ***********************************************************************/
void thread_create(thread_properties* struct_pointer)
{	
	pthread_attr_init(&(struct_pointer->attribute));
	pthread_attr_setinheritsched(&(struct_pointer->attribute),PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&(struct_pointer->attribute),SCHED_FIFO);
	struct_pointer->parameter.sched_priority = struct_pointer->priority;
	sem_init(&(struct_pointer->sem),0,0);
	pthread_attr_setschedparam(&(struct_pointer->attribute), &(struct_pointer->parameter));
	if(pthread_create(&(struct_pointer->thread), &(struct_pointer->attribute), struct_pointer->function_pointer, NULL)==0)
	{
        	syslog(SYSLOG_PRIORITY,"thread %d created\n\r",struct_pointer->thread_id);
    	}
    	else
    	{
        	syslog(SYSLOG_PRIORITY,"thread %d creation failed\n\r",struct_pointer->thread_id);
    	}
} 

/***********************************************************************
  * @brief thread_join()
  * join threads according to thread properties.
  * @param struct_pointer for the thread properties for thread to be joined.
  ***********************************************************************/
void thread_join(thread_properties* struct_pointer)
{
	pthread_join(struct_pointer->thread,NULL);
}

/***********************************************************************
  * @brief image_capture()
  * Capture a Mat frame from the camera.
  ***********************************************************************/
void image_capture(void)
{
	sem_wait(&sem_capture);
	camera >> frame;
	sem_post(&sem_capture_done);
	sem_post(&sem_capture);
}

/***********************************************************************
  * @brief save_ppm()
  * Save a frame in ppm format.
  * @param count of the frame to be saved.
  ***********************************************************************/
void  save_ppm(uint32_t count)
{
	uint8_t str[10];
	number_string(count,4,str);
	sem_wait(&sem_ppm);
	sem_wait(&sem_capture_done);
	file_name.str("");	
	file_name<<"frame_"<<str<<".ppm";
	imwrite(file_name.str(),frame,ppm_settings);
	sem_post(&sem_ppm);
}

/***********************************************************************
  * @brief func_1()
  * Sequencer thread. sequences other threads according to requirement.
  * @param ptr pointer to be used to update data modified in the thread.
  ***********************************************************************/
void* func_1(void* ptr)
{
	uint8_t func_id=0,i=0,j=0;
	uint8_t prev_sec=11,new_sec=1;
	while(loop_condition)
	{	
		clock_gettime(CLOCK_REALTIME,&time_check);
		new_sec=time_check.tv_nsec/(NSEC_PER_SEC/FREQUENCY);
		if(new_sec != prev_sec)
		{
			loop_condition_check();
			prev_sec=new_sec;
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
						if(i%(SCHEDULER_FREQ/func_props[j].thread_frequency)==0)
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
	}
	for(j=1;j<TOTAL_THREADS;j++)
	{
		func_props[j].exit_condition=True;
		sem_post(&(func_props[j].sem));
	}
	pthread_exit(NULL);
}

/***********************************************************************
  * @brief func_2()
  * Thread function to be used according to requirement.
  * @param ptr pointer to be used to update data modified in the thread.
  ***********************************************************************/
void* func_2(void* ptr)
{
	uint8_t func_id=1;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{
			image_capture();
			function_end(func_id);		
		}
	}
	pthread_exit(NULL);
}

/***********************************************************************
  * @brief func_3()
  * Thread function to be used according to requirement.
  * @param ptr pointer to be used to update data modified in the thread.
  ***********************************************************************/
void* func_3(void* ptr)
{
	uint8_t func_id=2;
	while((loop_condition)||(func_props[func_id].thread_live))
	{	
		function_beginning(func_id);
		if(!func_props[func_id].exit_condition)
		{	
			save_ppm(func_props[func_id].counter);
			function_end(func_id);
		}
	}
	pthread_exit(NULL);
}

/***********************************************************************
  * @brief struct_settings()
  * Set up structure properties required by the code.
  ***********************************************************************/
void struct_settings(void)
{
	func_props[0].function_pointer = func_1; 
	func_props[1].function_pointer = func_2; 
	func_props[2].function_pointer = func_3; 
}

/***********************************************************************
  * @brief sem_settings()
  * Set up semaphores required in the code.
  ***********************************************************************/
void sem_settings(void)
{
	sem_init(&sem_capture,0,1);
	sem_init(&sem_capture_done,0,0);
	sem_init(&sem_ppm,0,1);
}

/***********************************************************************
  * @brief camera_test()
  * Test if camera is acquired properly.
  ***********************************************************************/
void camera_test(void)
{
	ppm_settings.push_back(CV_IMWRITE_PXM_BINARY);
	ppm_settings.push_back(1);
	jpg_settings.push_back(CV_IMWRITE_JPEG_QUALITY);
	ppm_settings.push_back(1);
	camera.open(False);
	camera >> frame;	
	imwrite("test.ppm",frame);
}

/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	clock_gettime(CLOCK_REALTIME,&code_start_time);
	system("uname -a  > system_version.out");
	struct_settings();
	func_props[0].function_pointer = func_1; 
	func_props[1].function_pointer = func_2; 
	func_props[2].function_pointer = func_3; 
	camera_test();
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_create(&func_props[i]);
	}
	sem_settings();
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_join(&func_props[i]);
	}
	camera.release();
	clock_gettime(CLOCK_REALTIME,&code_end_time); 
	delta_t(&code_end_time, &code_start_time, &code_execution_time);
	syslog(SYSLOG_PRIORITY,"\nCode Execution Time = %ld seconds %ld nanoseconds\n",code_execution_time.tv_sec,code_execution_time.tv_nsec);
}
