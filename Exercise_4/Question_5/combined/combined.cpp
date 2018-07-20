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
#define CANNY_DEADLINE 20
#define HOUGH_DEADLINE 30
#define HOUGH_ELIPTICAL_DEADLINE 40

// Transform display window
char timg_window_name[] = "Edge Detector Transform";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame, cdst, timg_gray, timg_grad;

IplImage* frame;

pthread_t thread_canny, thread_hough, thread_hough_eliptical;

pthread_attr_t attribute_canny, attribute_hough, attribute_hough_eliptical;

struct sched_param parameter_canny, parameter_hough, parameter_hough_eliptical;

typedef struct
{
	uint8_t* title;
	struct timespec deadline;
	struct timespec start;
	struct timespec stop;
	struct timespec difference;
	struct timespec jitter;
}measured_time;

static measured_time canny_time_struct;
static measured_time hough_time_struct;
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
	
	delta_t(&(timeptr->difference), &(timeptr->deadline), &(timeptr->jitter));
	return;
}

void print_time_logs(measured_time * timeptr)
{
	printf("%s",timeptr->title);
	//printf("Resolution %dx%d"HRES,VRES);
	
	printf("Transform start seconds = %ld, nanoseconds = %ld\n", 
        timeptr->start.tv_sec, timeptr->start.tv_nsec);

	printf("Transform stop seconds = %ld, nanoseconds = %ld\n", 
        timeptr->stop.tv_sec, timeptr->stop.tv_nsec);

	printf("Transform time required seconds = %ld, nanoseconds = %ld\n", 
        timeptr->difference.tv_sec, timeptr->difference.tv_nsec);

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
    CvCapture* capture;
    int dev=0,rc=0;

    namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    while(1)
    {
	jitter_difference_start(&canny_time_struct);
        frame=cvQueryFrame(capture);
        if(!frame) break;
	jitter_difference_end(&canny_time_struct);
	print_time_logs(&canny_time_struct);

        CannyThreshold(0, 0);
	jitter_difference_end(&canny_time_struct);
        cvShowImage("Capture Example", frame);

        char c = cvWaitKey(10);
        if( c == 27 ) break;
    }
}

void *hough_func(void *threadp)
{
    CvCapture* capture;
    IplImage* frame;
    int dev=0;
    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    while(1)
    {
	
    CvCapture* capture;
    int dev=0,rc=0;

    namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

	jitter_difference_start(&hough_time_struct);
        frame=cvQueryFrame(capture);

        Mat mat_frame(frame);
        Canny(mat_frame, canny_frame, 50, 200, 3);

        //cvtColor(canny_frame, cdst, CV_GRAY2BGR);
        //cvtColor(mat_frame, gray, CV_BGR2GRAY);

        HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

        for( size_t i = 0; i < lines.size(); i++ )
        {
          Vec4i l = lines[i];
          line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }

     
        if(!frame) break;
	
        CannyThreshold(0, 0);

	jitter_difference_end(&hough_time_struct);
	print_time_logs(&hough_time_struct);

        cvShowImage("Capture Example", frame);

        char c = cvWaitKey(10);
        if( c == 27 ) break;
    }
}

void *hough_eliptical_func(void *threadp)
{
   cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(0);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(argv[1]);
    CvCapture* capture;
    IplImage* frame;
    int dev=0;
    Mat gray;
    vector<Vec3f> circles;

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

    while(1)
    {
	jitter_difference_start(&hough_eliptical_time_struct);
        frame=cvQueryFrame(capture);

        Mat mat_frame(frame);
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

   cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(0);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(argv[1]);
    CvCapture* capture;
    IplImage* frame;
    int dev=0,thread_no=0,rc=0;
	
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
    uint8_t hough_title[]="\nHough Interactive Transform\n";
    uint8_t hough_eliptical_title[]="\nHough Eliptical Interactive Transform\n";
    canny_time_struct.title=canny_title;
    hough_time_struct.title=hough_title;
    hough_eliptical_time_struct.title=hough_eliptical_title;
    canny_time_struct.deadline.tv_nsec=CANNY_DEADLINE*NSEC_PER_MSEC;
    hough_time_struct.deadline.tv_nsec=HOUGH_DEADLINE*NSEC_PER_MSEC;
    hough_eliptical_time_struct.deadline.tv_nsec=HOUGH_ELIPTICAL_DEADLINE*NSEC_PER_MSEC;

   	rc=pthread_attr_init(&attribute_canny);
  	rc=pthread_attr_setinheritsched(&attribute_canny, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_canny, SCHED_FIFO);
  	parameter_canny.sched_priority=MAX_PRIORITY-thread_no;
	thread_no++;
  	pthread_attr_setschedparam(&attribute_canny, &parameter_canny);

   	rc=pthread_attr_init(&attribute_hough);
  	rc=pthread_attr_setinheritsched(&attribute_hough, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_hough, SCHED_FIFO);
  	parameter_hough.sched_priority=MAX_PRIORITY-thread_no;
	thread_no++;
  	pthread_attr_setschedparam(&attribute_hough, &parameter_hough);

   	rc=pthread_attr_init(&attribute_hough_eliptical);
  	rc=pthread_attr_setinheritsched(&attribute_hough_eliptical, PTHREAD_EXPLICIT_SCHED);
  	rc=pthread_attr_setschedpolicy(&attribute_hough_eliptical, SCHED_FIFO);
  	parameter_hough_eliptical.sched_priority=MAX_PRIORITY-thread_no;
	thread_no++;
  	pthread_attr_setschedparam(&attribute_hough_eliptical, &parameter_hough_eliptical);

	if(pthread_create(&thread_canny, &attribute_canny, canny_func, NULL)==0)
		printf("\n\rcanny thread created\n\r");
  	else perror("\n\rcanny thread creation failed\n\r");

	if(pthread_create(&thread_hough, &attribute_hough, hough_func, NULL)==0)
		printf("\n\rhough thread created\n\r");
  	else perror("\n\rhough thread creation failed\n\r");

	if(pthread_create(&thread_hough_eliptical, &attribute_hough_eliptical, hough_eliptical_func, NULL)==0)
		printf("\n\rhough eliptical thread created\n\r");
  	else perror("\n\rhough eliptical thread creation failed\n\r");

  	pthread_join(thread_canny, NULL);
  	pthread_join(thread_hough, NULL);
  	pthread_join(thread_hough_eliptical, NULL);

    cvReleaseCapture(&capture);
    cvDestroyWindow("Capture Example");
    
};