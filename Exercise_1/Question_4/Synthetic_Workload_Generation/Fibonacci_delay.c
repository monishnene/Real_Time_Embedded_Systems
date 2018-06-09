#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <math.h>
#include <sys/param.h>

#define FIB_TIMER_CONFIG 
/* Used to configure the iterations of
FIB_TEST */

//#define SYSLOG /* For syslog messages */

pthread_t testThread10;
pthread_t testThread20;

pthread_attr_t rt10_sched_attr;
pthread_attr_t rt20_sched_attr;
pthread_attr_t main_sched_attr;
sem_t sem_t10,sem_t20;

double start_1 = 0;


int rt_max_prio, rt_min_prio, min, abortTest_10 = 0, abortTest_20 = 0;
struct sched_param  rt10_param;
struct sched_param  rt20_param;
struct sched_param  nrt_param;
struct sched_param  main_param ;
#define FIB_LIMIT_FOR_32_BIT 47
int seqIterations = FIB_LIMIT_FOR_32_BIT;


double readTOD(void);
double elapsedTOD(double stopTOD,double startTOD);
void printTOD(double stopTOD,double startTOD);

#define FIB_TEST(seqCnt, iterCnt)      \
for(idx=0; idx < iterCnt; idx++)    \
{                                   \
	while(jdx < seqCnt)	\
	{                              \
		if (jdx == 0)	\
		{	\
			fib = 1;	\
		}	\
		else	\
		{	\
			fib0 = fib1;	\
			fib1 = fib;	\
			fib = fib0 + fib1;	\
		}	\
		jdx++;	\
	}	\
}	\

double readTOD(void)
{
	struct timeval tv;
	double ft=0.0;
	if(gettimeofday(&tv,NULL)!=0)
	{
		perror("readTOD");
		return 0.0;
	}
	else
	{
		ft=((double)(((double)tv.tv_sec)+(((double)tv.tv_usec)/1000000.0)));
	}
	return ft;
}

void elapsedTODPrint(double stopTOD,double startTOD)
{
	double dt;
	if(stopTOD > startTOD)
	{	
		dt=(stopTOD - startTOD);
		printf("dt=%lf\n",dt);
	}
	else
{
		printf("WARNING: OVERFLOW\n");
		dt=0.0;
	}
}


double elapsedTOD(double stopTOD,double startTOD)
{
	double dt;
	if(stopTOD > startTOD)
	{
		dt=(stopTOD - startTOD);
	}
	else
	{
		printf("WARNING: OVERFLOW\n");
		dt=0.0;
	}
	return dt;
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

#ifdef FIB_TIMER_CONFIG
void *Thread10(void *threadid)
{
	struct sched_param param;
	int policy;
	double start =0;
	double stop =0;

	double duration;
	int idx=0,jdx=1;
	int fib=0,fib0=0,fib1=1;
	unsigned long mask =1;
	/* processor 0 */
	/* bind process to processor 0 */

	if(pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask)<0)
	{
		perror("pthread_setaffinity_np");
		// Estimate CPU clock rate
	}
	start = readTOD();
	//mdelay(10); 410000 for 10 msec 838000 for 20 msec
	FIB_TEST(seqIterations,1349000);
	stop= readTOD();

	pthread_getschedparam(testThread10,&policy,&param);
	#ifdef SYSLOG
	syslog(LOG_KERN|LOG_CRIT,"Thread10 priority = %d and timestamp %lf msec\n",param.sched_priority,(double)(stop-start)*1000);
	#else
	printf("Thread10 priority = %d and time stamp %lfmsec\n",param.sched_priority,(double)(stop-start)*1000);
	#endif
}

#else

void *Thread10(void *threadid)
{
	struct sched_param    param;
	int policy;
	double start =0;
	double stop =0;
	double duration;
	int idx =0, jdx =1;
	int fib =0, fib0 =0, fib1 =1;
	unsigned long mask =1;
	/* processor 0 */
	/* bind process to processor 0 */
	while(!abortTest_10)
	{
		sem_wait(&sem_t10);
		if(pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask)<0)
		{
			perror("pthread_setaffinity_np");
			// Estimate CPU clock rate
		}
		//start = readTOD();
		//mdelay(10);
		FIB_TEST(seqIterations,1314000);
		stop = readTOD();

		pthread_getschedparam(testThread10,&policy ,&param);
		#ifdef SYSLOG
		syslog(LOG_KERN|LOG_CRIT,"Thread10 priority = %d and time stamp%lf msec\n",param.sched_priority,(double)(stop-start_1)*1000);
		#else
		printf("Thread10 priority = %d and time stamp %lfmsec\n",param.sched_priority,(double)(stop-start_1)*1000);
		#endif
	}
}

void *Thread20(void*threadid)
{
	struct sched_param  param;
	int    policy;
	double start =0;
	double stop =0;
	int idx =0, jdx =1;
	int fib =0, fib0 =0, fib1 =1;
	double duration;

	while(!abortTest_20)
	{
		sem_wait(&sem_t20);
		/* bind process to processor 0 */
		unsigned long mask =1;
		/* processor 0 */
		if(pthread_setaffinity_np(pthread_self(),sizeof(mask),&mask)<0)
		{
			perror("pthread_setaffinity_np");
		}
		//start = readTOD();
		FIB_TEST(seqIterations,2628000);
		stop = readTOD();

		pthread_getschedparam(testThread20,&policy ,&param);
		#ifdef SYSLOG
		syslog(LOG_KERN|LOG_CRIT,"Thread20 priority = %d and time stamp%lf msec\n",param.sched_priority,(double)(stop-start_1)*1000);
		#else
		printf("Thread20 priority = %d and time stamp %lfmsec\n",param.sched_priority,(double)(stop-start_1)*1000);
		#endif
	}
}
#endif


int main(int argc,char *argv[])
{
	int rc,scope,i;
	useconds_t t_10,t_20;
	sem_init(&sem_t10,0,1);
	sem_init(&sem_t20,0,1);
	double stop_1=0;
	t_10=10000;
	t_20=20000;
	printf("Before adjustments to scheduling policy:\n");
	print_scheduler();
	pthread_attr_init(&rt10_sched_attr);
	pthread_attr_init(&rt20_sched_attr);
	pthread_attr_init(&main_sched_attr);
	pthread_attr_setinheritsched(&rt10_sched_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&rt10_sched_attr,SCHED_FIFO);
	pthread_attr_setinheritsched(&rt20_sched_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&rt20_sched_attr,SCHED_FIFO);
	pthread_attr_setinheritsched(&main_sched_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&main_sched_attr,SCHED_FIFO);
	rt_max_prio= sched_get_priority_max(SCHED_FIFO);
	rt_min_prio =sched_get_priority_min(SCHED_FIFO);

	rc = sched_getparam(getpid(),&nrt_param);
	main_param.sched_priority=rt_max_prio-2;
	rt10_param.sched_priority=rt_max_prio;
	rt20_param.sched_priority=rt_max_prio-1;

	rc=sched_setscheduler(getpid(),SCHED_FIFO,&main_param);

	if(rc)
	{
		printf("ERROR; sched_setscheduler rc is %d\n",rc);
		perror(NULL);
		exit(-1);
	}
	printf("After adjustments to scheduling policy:\n");
	print_scheduler();
	printf("min prio = %d, max prio = %d\n",rt_min_prio,rt_max_prio);
	pthread_attr_getscope(&rt10_sched_attr,&scope);
	// Check the scope of the POSIX scheduling mechanism
	//
	if(scope==PTHREAD_SCOPE_SYSTEM)
	printf("PTHREAD SCOPE SYSTEM\n");
	else if(scope==PTHREAD_SCOPE_PROCESS)
	printf("PTHREAD SCOPE PROCESS\n");
	else
	printf("PTHREAD SCOPE UNKNOWN\n");


	pthread_attr_setschedparam(&rt10_sched_attr,&rt10_param);
	pthread_attr_setschedparam(&rt20_sched_attr,&rt20_param);
	pthread_attr_setschedparam(&main_sched_attr,&main_param);
	#ifdef FIB_TIMER_CONFIG
	rc = pthread_create(&testThread10,&rt10_sched_attr,Thread10 ,(void*)0);
	if(rc)
	{
		printf("ERROR; pthread_create() rc is %d\n",rc);
		perror(NULL);
		exit(-1);
	}
	pthread_join(testThread10,NULL);
	if(pthread_attr_destroy(&rt10_sched_attr)!=0)
	perror("attr destroy");
	rc = sched_setscheduler(getpid(),SCHED_OTHER,&nrt_param);
	printf("TEST COMPLETE\n");
	#else
	start_1 = readTOD();
	rc = pthread_create(&testThread10,&rt10_sched_attr,Thread10 ,(void*)0);
	if(rc)
	{
		printf("ERROR; pthread_create() rc is %d\n", rc);
		perror(NULL);
		exit(-1);
	}
	rc = pthread_create(&testThread20,&rt20_sched_attr,Thread20 ,(void*)0);
	if(rc)
	{
		printf("ERROR; pthread_create() rc is %d\n", rc);
		perror(NULL);
		exit(-1);
	}
	/* Basic sequence of releases after CI */
	usleep(t_20);

	sem_post(&sem_t10);
	usleep(t_20);
	sem_post(&sem_t10);
	usleep(t_10);
	abortTest_20 =1;
	sem_post(&sem_t20);
	usleep(t_10);
	sem_post(&sem_t10);
	usleep(t_20);
	abortTest_10 =1;
	sem_post(&sem_t10);
	usleep(t_20);
	stop_1 = readTOD();
	printf("Test Conducted over  %lf msec\n",(double)(stop_1-start_1)*1000);
	pthread_join(testThread10,NULL);
	pthread_join(testThread20,NULL);
	if(pthread_attr_destroy(&rt10_sched_attr)!=0)
	perror("attr destroy");
	if(pthread_attr_destroy(&rt20_sched_attr)!=0)
	perror("attr destroy");
	sem_destroy(&sem_t10);
	sem_destroy(&sem_t20);
	rc=sched_setscheduler(getpid(), SCHED_OTHER,&nrt_param);
	printf("TEST COMPLETE\n");
	#endif
}
