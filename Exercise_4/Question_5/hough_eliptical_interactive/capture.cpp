/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define OK (0)
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define HRES 1280
#define VRES 960
#define MAX_PRIORITY 100
#define HOUGH_ELIPTICAL_DEADLINE 40

// Transform display window
char timg_window_name[] = "Hough Eliptical Interactive Transform";

IplImage* frame;
CvCapture* capture;
int horizontal = HRES, vertical = VRES;
double fps = 0;

pthread_t thread_hough_eliptical;

pthread_attr_t attribute_hough_eliptical;

struct sched_param parameter_hough_eliptical;

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

static measured_time hough_eliptical_time_struct;

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


void jitter_difference_start(measured_time * timeptr)
{
  	clock_gettime(CLOCK_REALTIME, &(timeptr->start));
}

void jitter_difference_end(measured_time * timeptr)
{
  	clock_gettime(CLOCK_REALTIME, &(timeptr->stop));
	
	delta_t(&(timeptr->stop),&(timeptr->start), &(timeptr->difference));
	
	fps = 1/(timeptr->difference.tv_sec+double(timeptr->difference.tv_nsec/NSEC_PER_SEC));

	delta_t(&(timeptr->difference), &(timeptr->deadline), &(timeptr->jitter));	

	return;
}

void print_time_logs(measured_time * timeptr)
{
	printf("%s",timeptr->title);
	//printf("Resolution %d x %d"&horizontal,&vertical);
	
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

void *hough_eliptical_func(void *threadp)
{
    int dev=0;
    Mat gray;
    vector<Vec3f> circles;
    while(1)
    {
	jitter_difference_start(&hough_eliptical_time_struct);
        frame=cvQueryFrame(capture);

        /*Mat mat_frame(frame);
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
        }*/
     
        if(!frame) break;
	jitter_difference_end(&hough_eliptical_time_struct);
	print_time_logs(&hough_eliptical_time_struct);

        cvShowImage("Capture Example", frame);

        char c = cvWaitKey(10);
        if( c == 27 ) break;
    }
}

int main( int argc, char** argv )
{
    int thread_no = 0, dev = 0, rc = 0;
     if(argc > 1)
    {
        sscanf(argv[1], "%d", &dev);
        printf("using %s\n", argv[1]);
    }
    else if(argc == 1)
        printf("using default\n");

    else
    {
        printf("usage: capture [dev]\n");
        exit(-1);
    }
    cvNamedWindow("Combined", CV_WINDOW_AUTOSIZE);
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold ); 
    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    if(argc > 1)
    {
        sscanf(argv[1], "%d", &dev);
        printf("using %s\n", argv[1]);
    }
    else if(argc == 1)
        printf("using default\n");

    else
    {
        printf("usage: capture [dev]\n");
        exit(-1);
    }
    uint8_t hough_eliptical_title[]="\nHough Eliptical Interactive Transform\n";
    hough_eliptical_time_struct.title=hough_eliptical_title;
    hough_eliptical_time_struct.deadline.tv_nsec=HOUGH_ELIPTICAL_DEADLINE*NSEC_PER_MSEC;

   	rc=pthread_attr_init(&attribute_hough_eliptical);
  	rc=pthread_attr_setinheritsched(&attribute_hough_eliptical, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_hough_eliptical, SCHED_FIFO);
  	parameter_hough_eliptical.sched_priority=MAX_PRIORITY-thread_no;
	thread_no++;
  	pthread_attr_setschedparam(&attribute_hough_eliptical, &parameter_hough_eliptical);


	if(pthread_create(&thread_hough_eliptical, &attribute_hough_eliptical, hough_eliptical_func, NULL)==0)
		printf("\n\rhough eliptical thread created\n\r");
  	else perror("\n\rhough eliptical thread creation failed\n\r");

  	pthread_join(thread_hough_eliptical, NULL);

    cvReleaseCapture(&capture);
    cvDestroyWindow("Capture Example");
    
};