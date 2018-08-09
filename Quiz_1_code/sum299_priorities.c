#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>


pthread_t thread_1,thread_2,thread_3;

pthread_attr_t attribute_1, attribute_2, attribute_3 ;

struct sched_param parameter_1, parameter_2, parameter_3;

void* add_100_numbers (void* ptr)
{
uint16_t* ptr_2 = (uint16_t*) ptr;
uint8_t i = 0;
uint32_t result=0;
for(i=0;i<100;i++)
{
result += *(ptr_2+i);
} 
*(ptr_2+100) = result;
return (void *)ptr_2+200;
}

void main()
{

uint16_t array_1[101],array_2[101],array_3[101],counter=0;
uint8_t i=0,ret_1=0,ret_2=0,ret_3=0,rc=0;
uint16_t result=0;
void *sum_1,*sum_2,*sum_3;
for(i=0;i<100;i++)
{
array_1[i]=counter;
counter++;
}
for(i=0;i<100;i++)
{
array_2[i]=counter;
counter++;
}
for(i=0;i<100;i++)
{
array_3[i]=counter;
counter++;
}
rc=pthread_attr_init(&attribute_1);
rc=pthread_attr_setinheritsched(&attribute_1, PTHREAD_EXPLICIT_SCHED);
rc=pthread_attr_setschedpolicy(&attribute_1, SCHED_FIFO);
parameter_1.sched_priority=120;
pthread_attr_setschedparam(&attribute_1, &parameter_1);

rc=pthread_attr_init(&attribute_2);
rc=pthread_attr_setinheritsched(&attribute_2, PTHREAD_EXPLICIT_SCHED);
rc=pthread_attr_setschedpolicy(&attribute_2, SCHED_FIFO);
parameter_2.sched_priority=110;
pthread_attr_setschedparam(&attribute_2, &parameter_2);

rc=pthread_attr_init(&attribute_3);
rc=pthread_attr_setinheritsched(&attribute_3, PTHREAD_EXPLICIT_SCHED);
rc=pthread_attr_setschedpolicy(&attribute_3, SCHED_FIFO);
parameter_3.sched_priority=100;
pthread_attr_setschedparam(&attribute_3, &parameter_3);

ret_1 = pthread_create(&thread_1, (void*)&attribute_1, add_100_numbers, (void*)array_1);
ret_2 = pthread_create(&thread_2, (void*)&attribute_2, add_100_numbers, (void*)array_2);
ret_3 = pthread_create(&thread_3, (void*)&attribute_3, add_100_numbers, (void*)array_3);

pthread_join(thread_1,&sum_1);
pthread_join(thread_2,&sum_2);
pthread_join(thread_3,&sum_3);

result = *((uint16_t*)sum_1)+*((uint16_t*)sum_2)+*((uint16_t*)sum_3);
printf("The sum of numbers from 0 to 299 is %d\n\r", result);
}
