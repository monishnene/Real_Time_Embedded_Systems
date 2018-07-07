#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<pthread.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<mqueue.h>

#define SNDRCV_MQ	"/send_receive_mq"
#define ERROR	(-1)


pthread_t send_thread, receive_thread;

struct mq_attr mq_attr;

pthread_attr_t att[2];

struct sched_param parameter[2];


static char canned_msg[] = "This is a test, and only a test, in the event of real emergency, you would be instructed...."; // Message that is to be sent by sender for posix_mq

void *sender_function(void *arg){
mqd_t mymq;
int prio;
int nbytes;

mymq=mq_open(SNDRCV_MQ, O_RDWR, 0664, &mq_attr);	// read write permission using O_RDWR

if((nbytes = mq_send(mymq, canned_msg, sizeof(canned_msg),30))== ERROR){	// Error checking while sending
printf("mq_send");
}
else{	// Message is being sent successfully
printf("send: message successfully sent\n");
}
}


void *receiver_function(void *arg){
mqd_t mymq;
char buffer[128];
int prio;
int nbytes;
mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0664, &mq_attr); //mq_open function with its parameters 's output being returned to mymq

if((nbytes = mq_receive(mymq,buffer, 128 , &prio)) == ERROR)	// Error checking while receiving
{
printf("mq_receive");
}
else
{
// Message is being received successfully
buffer[nbytes]='\0';
printf("recive:msg %s received with priority =%d, length =%d\n",buffer, prio, nbytes);
}
}

void main(){
int i=0;
for(i=0;i<2;i++){
pthread_attr_init(&att[i]);	// Attribute initialisation for both threads
pthread_attr_setinheritsched(&att[i],PTHREAD_EXPLICIT_SCHED);//Setting the scheduling policy
pthread_attr_setschedpolicy(&att[i], SCHED_FIFO);//Scheduling policy is set to SCHED_FIFO
parameter[i].sched_priority=99-i;	//Priority assignment
pthread_attr_setschedparam(&att[i],&parameter[i]);	// Assigning the remaining parameters
}


mq_attr.mq_maxmsg=10;		//number of messages
mq_attr.mq_msgsize= 128;	//size of a message

mq_attr.mq_flags=0;

pthread_create(&receive_thread, &att[0], receiver_function, NULL);	// Creating  the receive thread

pthread_create(&send_thread, &att[1] ,sender_function, NULL);	//Creating the sender thread

pthread_join(receive_thread,NULL);
pthread_join(send_thread,NULL);
}
