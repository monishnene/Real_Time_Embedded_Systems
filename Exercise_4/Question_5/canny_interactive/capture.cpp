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
#define CANNY_DEADLINE 20

// Transform display window
char timg_window_name[] = "Canny Interactive Transform";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame, cdst, timg_gray, timg_grad;
IplImage* frame;
CvCapture* capture;
int horizontal = HRES, vertical = VRES;
double fps = 0;

pthread_t thread_canny;

pthread_attr_t attribute_canny;

struct sched_param parameter_canny;

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

static measured_time canny_time_struct;

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

void CannyThreshold(int, void*)
{
    Mat mat_frame(frame);

    cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size(3,3) );

    /// Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all(0);

    mat_frame.copyTo( timg_grad, canny_frame);

    imshow( timg_window_name, timg_grad );

}

void *canny_func(void *threadp)
{
    while(1)
    {
	jitter_difference_start(&canny_time_struct);
        frame=cvQueryFrame(capture);
        if(!frame) break;
	jitter_difference_end(&canny_time_struct);
	print_time_logs(&canny_time_struct);

        CannyThreshold(0, 0);
	jitter_difference_end(&canny_time_struct);

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
    uint8_t canny_title[]="\nCanny Interactive Transform\n";
    canny_time_struct.title=canny_title;
    canny_time_struct.deadline.tv_nsec=CANNY_DEADLINE*NSEC_PER_MSEC;

   	rc=pthread_attr_init(&attribute_canny);
  	rc=pthread_attr_setinheritsched(&attribute_canny, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_canny, SCHED_FIFO);
  	parameter_canny.sched_priority=MAX_PRIORITY-thread_no;
	thread_no++;
  	pthread_attr_setschedparam(&attribute_canny, &parameter_canny);

	if(pthread_create(&thread_canny, &attribute_canny, canny_func, NULL)==0)
		printf("\n\rcanny thread created\n\r");
  	else perror("\n\rcanny thread creation failed\n\r");

  	pthread_join(thread_canny, NULL);

    cvReleaseCapture(&capture);
    cvDestroyWindow("Capture Example");
    
};