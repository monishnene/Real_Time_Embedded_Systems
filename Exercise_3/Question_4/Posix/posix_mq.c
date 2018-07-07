#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>

#define SNDRCV_MQ "/send_receive_mq"

struct mq_attr mq_attr;				

static mqd_t mymq;

pthread_t thread_receive, thread_send;

pthread_attr_t attribute_receive, attribute_send;

struct sched_param parameter_receive, parameter_send;		

void *receiver(void *threadp)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr; 
  int prio;
  int nbytes;
  int count = 0;
  int id;
 
  while(1) {

    /* read oldest, highest priority msg from the message queue */

    printf("\nReading %ld bytes\n", sizeof(void *));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == -1)
    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("\nReceive: ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);

      printf("\nContents of ptr = \n%s\n", (char *)buffptr);

      free(buffptr);

      printf("\nHeap space memory freed\n");

    }
    
  }

}


static char imagebuff[4096];

void *sender(void *threadp)
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;


  while(1) {

    /* send malloc'd message with priority=30 */

    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("\nMessage to send = %s\n", (char *)buffptr);

    printf("\nSending %ld bytes\n", sizeof(buffptr));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == -1)
    {
      perror("mq_send");
    }
    else
    {
      printf("\nSend: message ptr 0x%p successfully sent\n", buffptr);
    }

    usleep(3000000);

  }
  
}


static int sid, rid;

void main()
{
  int i, j;
  char pixel = 'A';
  int rc;

  for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);
  //printf("%ld", sizeof(imagebuff));

  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;

  /* note that VxWorks does not deal with permissions? */
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0664, &mq_attr);


  rc=pthread_attr_init(&attribute_receive);
  rc=pthread_attr_setinheritsched(&attribute_receive, PTHREAD_EXPLICIT_SCHED);
  rc=pthread_attr_setschedpolicy(&attribute_receive, SCHED_FIFO);
  parameter_receive.sched_priority=50;
  pthread_attr_setschedparam(&attribute_receive, &parameter_receive);

  rc=pthread_attr_init(&attribute_send);
  rc=pthread_attr_setinheritsched(&attribute_send, PTHREAD_EXPLICIT_SCHED);
  rc=pthread_attr_setschedpolicy(&attribute_send, SCHED_FIFO);
  parameter_send.sched_priority=49;
  pthread_attr_setschedparam(&attribute_send, &parameter_send);

if(pthread_create(&thread_receive, (void*)&attribute_receive, receiver, NULL)==0)
	printf("\n\rReceiver thread created\n\r");
  else perror("\n\rReceiver thread creation failed\n\r");

if(pthread_create(&thread_send, (void*)&attribute_send, sender, NULL)==0)
	printf("\n\rSender thread created\n\r");
  else perror("\n\rSender thread creation failed\n\r");

  pthread_join(thread_receive, NULL);
  pthread_join(thread_send, NULL);

}

