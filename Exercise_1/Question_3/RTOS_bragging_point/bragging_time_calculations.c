#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#define NSEC_PER_SEC (1000000000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)

#define NUM_THREADS 12
void end_delay_test(void);

static struct timespec rtclk_interrupt_stop_time = {0, 0};
static struct timespec rtclk_interrupt_start_time = {0, 0};
static struct timespec rtclk_interrupt_delta_time = {0, 0};
static struct timespec context_switching_time[NUM_THREADS] = {0, 0};
static struct timespec context_switching_delta_time = {0, 0};

pthread_t main_thread;
pthread_attr_t main_sched_attr;
int rt_max_prio, rt_min_prio, min,first_interrupt=1;
struct sched_param main_param;

typedef struct
{
    int threadIdx;
} threadParams_t;


int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
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
  return(OK);
}

void end_delay_test(void)
{
  int i=0;
  long seconds=0,nano_seconds=0,average_seconds=0,average_nano_seconds=0;
  delta_t(&rtclk_interrupt_stop_time,&rtclk_interrupt_start_time,&rtclk_interrupt_delta_time);
  printf("\n");
  printf("Interrupt Handler start seconds = %ld, nanoseconds = %ld\n", 
         rtclk_interrupt_start_time.tv_sec, rtclk_interrupt_start_time.tv_nsec);

  printf("Interrupt Handler stop seconds = %ld, nanoseconds = %ld\n", 
         rtclk_interrupt_stop_time.tv_sec, rtclk_interrupt_stop_time.tv_nsec);

  printf("Interrupt Handler Latency seconds = %ld, nanoseconds = %ld\n", 
         rtclk_interrupt_delta_time.tv_sec, rtclk_interrupt_delta_time.tv_nsec);
  for(i=0;i<NUM_THREADS-1;i++)
  {
	delta_t(context_switching_time+i+1,context_switching_time+i,&context_switching_delta_time);
	/*printf("Context switch time %d seconds = %ld, nanoseconds = %ld\n", 
         i+1,context_switching_delta_time.tv_sec, context_switching_delta_time.tv_nsec);*/
	seconds+=context_switching_delta_time.tv_sec;
	nano_seconds+=context_switching_delta_time.tv_nsec;
  }
  average_seconds = seconds/(NUM_THREADS-1);
  average_nano_seconds = nano_seconds/(NUM_THREADS-1);	
  printf("Average Context Switching seconds = %ld, nanoseconds = %ld\n", 
         average_seconds, average_nano_seconds);
  exit(0);
}


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

void *counterThread(void *threadp)
{
    int sum=0, i;
    static int counter = 0;
    if(first_interrupt==1)
    {
	clock_gettime(CLOCK_REALTIME, &rtclk_interrupt_stop_time);   
	first_interrupt=0;
    }
	clock_gettime(CLOCK_REALTIME, context_switching_time + counter);
	counter++; 
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=1; i < (threadParams->threadIdx)+1; i++)
        sum=sum+i;
 
    printf("Thread idx=%d, sum[0...%d]=%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum);
}

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}

int main (int argc, char *argv[])
{
   int rc;
   int i;
   printf("Before adjustments to scheduling policy:\n");
   print_scheduler();
   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );
   }

   for(i=0;i<NUM_THREADS;i++)
   {
	if(first_interrupt==1)
    	{
		clock_gettime(CLOCK_REALTIME, &rtclk_interrupt_start_time);  
	}
	pthread_join(threads[i], NULL);	 
   }
   end_delay_test();
   printf("TEST COMPLETE\n");
}
