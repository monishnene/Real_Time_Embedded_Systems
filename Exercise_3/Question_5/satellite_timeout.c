#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define LOOPS 10
#define TIMEOUT 10
#define DELAY 1000 

pthread_mutex_t navigation_mutex;

struct timespec mutex_timeout;

typedef struct 
{
	double x;
	double y;
	double z;
	double yaw;
	double pitch;
	double roll;
	struct timespec time_stamp;
}typedef_navigation;

typedef_navigation latest_state =
{
	.x = 0,
	.y = 0,
	.z = 0,
	.yaw = 0,
	.pitch = 0,
	.roll = 0,
};

void* update_navigation(void* ptr)
{
	pthread_mutex_lock(&navigation_mutex);
	latest_state.x += 5;
	latest_state.y += 10;
	latest_state.z += 20;
	latest_state.yaw += 1;
	latest_state.pitch += 2;
	latest_state.roll += 3;
	clock_gettime(CLOCK_REALTIME,&(latest_state.time_stamp));
	usleep(DELAY);
	pthread_mutex_unlock(&navigation_mutex);
}

void* read_navigation(void* ptr)
{
	static uint8_t count = 1;
	uint8_t rc = 1;
	uint16_t wait = 0;
	while(rc != 0)
	{
		rc = pthread_mutex_timedlock(&navigation_mutex,&mutex_timeout);
		wait++;
	}
	printf("\n\rwait = %d",wait);
	printf("\n\rMutex timeout %d ended at Time seconds = %ld, nanoseconds = %ld\n\r",count++, latest_state.time_stamp.tv_sec, latest_state.time_stamp.tv_nsec);
	printf("\n\rTime seconds = %ld, nanoseconds = %ld\n\r", latest_state.time_stamp.tv_sec, latest_state.time_stamp.tv_nsec);
	printf("x-acceleration = %lf\n\r",latest_state.x);
	printf("y-acceleration = %lf\n\r",latest_state.y);
	printf("z-acceleration = %lf\n\r",latest_state.z);
	printf("yaw_rate = %lf\n\r",latest_state.yaw);
	printf("pitch_rate = %lf\n\r",latest_state.pitch);
	printf("roll_rate = %lf\n\r",latest_state.roll);
	pthread_mutex_unlock(&navigation_mutex);
}

void main()
{
	mutex_timeout.tv_sec = TIMEOUT;
	pthread_mutex_init(&navigation_mutex,NULL);
	pthread_t update_thread[LOOPS],read_thread[LOOPS];
	uint8_t i=0,ret_1=0,ret_2=0;
	for(i=0;i<LOOPS;i++)
	{

		ret_2 = pthread_create(&read_thread[i], NULL, read_navigation, NULL);
		ret_1 = pthread_create(&update_thread[i], NULL, update_navigation, NULL);

		pthread_join(read_thread[i],NULL);
		pthread_join(update_thread[i],NULL);
	}
}
