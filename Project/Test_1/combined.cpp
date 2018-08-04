/*
 * thrad_operations.cpp
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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define OK (0)
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define HRES 80
#define VRES 60
#define CANNY_DEADLINE 20
#define HOUGH_DEADLINE 30
#define HOUGH_ELIPTICAL_DEADLINE 40

sem_t sem_canny, sem_hough, sem_hough_eliptical;

char timg_window_name[] = "Combined Transform";

int lowThreshold = 0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame,cdst, timg_gray, timg_grad;

IplImage* frame_canny;
IplImage* frame_hough;
IplImage* frame_hough_eliptical;

CvCapture* capture;
double fps = 0;

pthread_t thread_canny, thread_hough, thread_hough_eliptical;

pthread_attr_t attribute_main, attribute_canny, attribute_hough, attribute_hough_eliptical;

struct sched_param parameter_main, parameter_scheduler, parameter_canny, parameter_hough, parameter_hough_eliptical;

typedef struct
{
	uint8_t* title;
	float frames;
	struct timespec deadline;
	struct timespec start;
	struct timespec stop;
	struct timespec difference;
	struct timespec jitter;
}measured_time;

typedef struct
{
	uint8_t priority = Max_Priority - thread_count++;
	uint8_t thread_id = thread_count;
	pthread_t thread;
	pthread_attr_t attribute;
	struct sched_param parameter;
	measured_time time_struct;
	void*(*function_pointer)(void*);
}thread_properties;

static measured_time canny_time_struct;
static measured_time hough_time_struct;
static measured_time hough_eliptical_time_struct;

/***********************************************************************
  * @brief delta_t()
  * Find the difference between two timespec structures eg a - b =c 
  * @param struct timespec *stop (a)
  * @param struct timespec *start (b)
  * @param struct timespec *delta_t (c)
  * @return success or fail
  ***********************************************************************/
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


/***********************************************************************
  * @brief jitter_difference_start()
  * Measure start time of a thread 
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void jitter_difference_start(measured_time * timeptr)
{
  	clock_gettime(CLOCK_REALTIME, &(timeptr->start));
}


/***********************************************************************
  * @brief jitter_difference_end()
  * Measure stop time of a thread and calculate execution time
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void jitter_difference_end(measured_time * timeptr)
{
  	clock_gettime(CLOCK_REALTIME, &(timeptr->stop));

	delta_t(&(timeptr->stop),&(timeptr->start), &(timeptr->difference));

	fps = NSEC_PER_SEC/timeptr->difference.tv_nsec;

	delta_t(&(timeptr->difference), &(timeptr->deadline), &(timeptr->jitter));

	return;
}


/***********************************************************************
  * @brief print_time_logs()
  * print start time, stop time, execution time, jitter and fps logs for various threads 
  * @param measured_time * timeptr pointer to thread time structure
  ***********************************************************************/
void print_time_logs(measured_time * timeptr)
{
	printf("%s",timeptr->title);
	printf("Resolution 80 x 60\n");

	printf("Transform start seconds = %ld, nanoseconds = %ld\n",
        timeptr->start.tv_sec, timeptr->start.tv_nsec);

	printf("Transform stop seconds = %ld, nanoseconds = %ld\n",
        timeptr->stop.tv_sec, timeptr->stop.tv_nsec);

	printf("Transform time required seconds = %ld, nanoseconds = %ld\n",
        timeptr->difference.tv_sec, timeptr->difference.tv_nsec);

	printf("Frames per second = %f\n", fps);

	printf("Jitter seconds = %ld, nanoseconds = %ld\n",
        timeptr->jitter.tv_sec, timeptr->jitter.tv_nsec);
}


/***********************************************************************
  * @brief CannyThreshold()
  * Adjust canny threshold value
  * @param threshold to be set
  ***********************************************************************/
void CannyThreshold(int, void*)
{
    Mat mat_frame(frame_canny);

    cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

    blur( timg_gray, canny_frame, Size(3,3) );

    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    timg_grad = Scalar::all(0);

    mat_frame.copyTo( timg_grad, canny_frame);
}

/***********************************************************************
  * @brief canny_func()
  * capture frame and implement canny interactive transform
  ***********************************************************************/
void *canny_func(void *threadid)
{
  while(1){
    sem_wait(&sem_canny);
    jitter_difference_start(&canny_time_struct);
        frame_canny =cvQueryFrame(capture);
        if(!frame_canny ) break;
	jitter_difference_end(&canny_time_struct);
	print_time_logs(&canny_time_struct);

        CannyThreshold(0, 0);

	char c = cvWaitKey(10);
        if( c == 27 ) break;

    sem_post(&sem_hough);
  }
  pthread_exit(NULL);
}


/***********************************************************************
  * @brief hough_func()
  * capture frame and implement hough interactive transform
  ***********************************************************************/
void *hough_func(void *threadid)
{
  long val;
  while(1){

    sem_wait(&sem_hough);

    Mat gray, canny_frame_h, cdst;
    vector<Vec4i> lines;
    jitter_difference_start(&hough_time_struct);
        frame_hough =cvQueryFrame(capture);

        Mat mat_frame(frame_hough);
        Canny(mat_frame, canny_frame_h, 50, 200, 3);

        cvtColor(canny_frame_h, cdst, CV_GRAY2BGR);
        cvtColor(mat_frame, gray, CV_BGR2GRAY);

        HoughLinesP(canny_frame_h, lines, 1, CV_PI/180, 50, 50, 10);

        for( size_t i = 0; i < lines.size(); i++ )
        {
          Vec4i l = lines[i];
          line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }


        if(!frame_hough) break;

        CannyThreshold(0, 0);

	jitter_difference_end(&hough_time_struct);
	print_time_logs(&hough_time_struct);

        char c = cvWaitKey(10);
        if( c == 27 ) break;
    sem_post(&sem_hough_eliptical);
  }
    pthread_exit(NULL);
}


/***********************************************************************
  * @brief hough_eliptical_func()
  * capture frame and implement hough eliptical interactive transform
  ***********************************************************************/
void *hough_eliptical_func(void *threadid)
{
  long val;
  while(1){
    sem_wait(&sem_hough_eliptical);

    Mat gray;
    vector<Vec3f> circles;

    jitter_difference_start(&hough_eliptical_time_struct);
        frame_hough_eliptical=cvQueryFrame(capture);

        Mat mat_frame(frame_hough_eliptical);
        cvtColor(mat_frame, gray, CV_BGR2GRAY);
        GaussianBlur(gray, gray, Size(9,9), 2, 2);

        HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);

        printf("circles.size = %d\n", circles.size());

        for( size_t i = 0; i < circles.size(); i++ )
        {
          Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
          int radius = cvRound(circles[i][2]);
          // circle center
          circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
          // circle outline
          circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }

        if(!frame_hough_eliptical) break;
	jitter_difference_end(&hough_eliptical_time_struct);
	print_time_logs(&hough_eliptical_time_struct);

        char c = cvWaitKey(10);
        if( c == 27 ) break;

    sem_post(&sem_canny);
  }
  pthread_exit(NULL);
}


/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{

	int dev=0, Max_Priority, thread_no = 0;
	int rc, scope;
	uint8_t canny_title[]="\nCanny Interactive Transform\n";
    uint8_t hough_title[]="\nHough Interactive Transform\n";
    uint8_t hough_eliptical_title[]="\nHough Eliptical Interactive Transform\n";
    canny_time_struct.title=canny_title;
    hough_time_struct.title=hough_title;
    hough_eliptical_time_struct.title=hough_eliptical_title;
    canny_time_struct.deadline.tv_nsec=CANNY_DEADLINE*NSEC_PER_MSEC;
    hough_time_struct.deadline.tv_nsec=HOUGH_DEADLINE*NSEC_PER_MSEC;
    hough_eliptical_time_struct.deadline.tv_nsec=HOUGH_ELIPTICAL_DEADLINE*NSEC_PER_MSEC;
	capture = (CvCapture *)cvCreateCameraCapture(dev);

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

	Max_Priority = sched_get_priority_max(SCHED_FIFO);

	pthread_attr_init(&attribute_main);
	pthread_attr_setinheritsched(&attribute_main,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attribute_main,SCHED_FIFO);
	parameter_main.sched_priority = Max_Priority - thread_no;
	thread_no++;

	pthread_attr_init(&attribute_canny);
	pthread_attr_setinheritsched(&attribute_canny,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attribute_canny,SCHED_FIFO);
	parameter_canny.sched_priority = Max_Priority - thread_no;
	thread_no++;

	pthread_attr_init(&attribute_hough_eliptical);
	pthread_attr_setinheritsched(&attribute_hough,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attribute_hough,SCHED_FIFO);
	parameter_hough.sched_priority = Max_Priority - thread_no;
	thread_no++;

	pthread_attr_init(&attribute_hough);
	pthread_attr_setinheritsched(&attribute_hough_eliptical,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attribute_hough_eliptical,SCHED_FIFO);
	parameter_hough_eliptical.sched_priority = Max_Priority - thread_no;
	thread_no++;

	rc=sched_getparam(getpid(), &parameter_scheduler);
	if (rc)
	{
		printf("ERROR; sched_setscheduler rc is %d\n", rc);
		perror(NULL);
		exit(-1);
	}

	rc=sched_setscheduler(getpid(),SCHED_FIFO,&parameter_main);
	if(rc)
	{
		printf("ERROR; main sched_setscheduler rc is %d\n",rc);
		perror(NULL);
		exit(-1);
	}

	if (sem_init (&sem_canny, 0, 1))
	{
	printf ("Failed to initialize sem_canny semaphore\n");
	exit (-1);
	}

	if (sem_init (&sem_hough, 0, 0))
	{
	printf ("Failed to initialize sem_hough semaphore\n");
	exit (-1);
	}

	if (sem_init (&sem_hough_eliptical, 0, 0))
	{
	printf ("Failed to initialize sem_hough_eliptical semaphore\n");
	exit (-1);
	}

	pthread_attr_setschedparam(&attribute_canny, &parameter_canny);
	pthread_attr_setschedparam(&attribute_hough, &parameter_hough);
	pthread_attr_setschedparam(&attribute_hough_eliptical, &parameter_hough_eliptical);
	pthread_attr_setschedparam(&attribute_main, &parameter_main);

	if(pthread_create(&thread_canny, &attribute_canny, canny_func, NULL)==0)
		printf("\n\rcanny thread created\n\r");
  	else perror("\n\rcanny thread creation failed\n\r");

	if(pthread_create(&thread_hough, &attribute_hough, hough_func, NULL)==0)
		printf("\n\rhough thread created\n\r");
  	else perror("\n\rhough thread creation failed\n\r");

	if(pthread_create(&thread_hough_eliptical, &attribute_hough_eliptical, hough_eliptical_func, NULL)==0)
		printf("\n\rhough eliptical thread created\n\r");
  	else perror("\n\rhough eliptical thread creation failed\n\r");

  pthread_join(thread_canny,NULL);
  pthread_join(thread_hough_eliptical,NULL);
  pthread_join(thread_hough,NULL);

  cvReleaseCapture(&capture);
  cvDestroyWindow("Combined Transform");

  rc=sched_setscheduler(getpid(), SCHED_OTHER, &parameter_scheduler);

}