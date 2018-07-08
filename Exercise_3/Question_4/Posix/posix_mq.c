#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define SNDRCV_MQ "/send_receive_mq"
#define BUFFER_SIZE 100
#define ERROR (-1)

struct mq_attr mq_attr;

pthread_t thread_receive, thread_send;

pthread_attr_t attribute_receive, attribute_send;

struct sched_param parameter_receive, parameter_send;	


static char canned_msg[] = "This is a test, and only a test, in the event of real emergency, you would be instructed...."; // Message to be sent

void *receiver(void *arg)
{
	mqd_t mymq;
	char buffer[BUFFER_SIZE];		
	int prio;
	int nbytes;
	mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0664, &mq_attr); 
	if((nbytes = mq_receive(mymq,buffer, BUFFER_SIZE, &prio)) == ERROR)	
	{
		printf("mq_receive");
	}
	else
	{	// Message received successfully
		buffer[nbytes]='\0';
		printf("recive:msg %s received with priority =%d, length =%d\n",buffer, prio, nbytes);
	}
}


void *sender(void *arg)
{
	mqd_t mymq;
	int prio;
	int nbytes;

	mymq=mq_open(SNDRCV_MQ, O_RDWR, 0664, &mq_attr);	// Create a message queue with name SNDRCV_MQ with read write permission

	if((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg),30))== ERROR)
	{	
		printf("mq_send");
	}
	else
	{	
		printf("send: message successfully sent\n");
	}
}

void main()
{
	int i=0,rc=0;
	mq_attr.mq_maxmsg=10;		//Message numbers
	mq_attr.mq_msgsize= BUFFER_SIZE;	//Message size

	mq_attr.mq_flags=0;

	rc=pthread_attr_init(&attribute_receive);
  	rc=pthread_attr_setinheritsched(&attribute_receive, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_receive, SCHED_FIFO);
  	parameter_receive.sched_priority=10;
  	pthread_attr_setschedparam(&attribute_receive, &parameter_receive);

  	rc=pthread_attr_init(&attribute_send);
  	rc=pthread_attr_setinheritsched(&attribute_send, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_send, SCHED_FIFO);
  	parameter_send.sched_priority=9;
  	pthread_attr_setschedparam(&attribute_send, &parameter_send);

	if(pthread_create(&thread_receive, &attribute_receive, receiver, NULL)==0)
		printf("\n\rReceiver thread created\n\r");
  	else perror("\n\rReceiver thread creation failed\n\r");

	if(pthread_create(&thread_send, &attribute_send, sender, NULL)==0)
		printf("\n\rSender thread created\n\r");
  	else perror("\n\rSender thread creation failed\n\r");

  	pthread_join(thread_receive, NULL);
  	pthread_join(thread_send, NULL);
}
