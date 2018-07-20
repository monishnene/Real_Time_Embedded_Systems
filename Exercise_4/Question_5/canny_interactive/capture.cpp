/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


#define NSEC_PER_SEC (1000000000)
#define HRES 640
#define VRES 480
#define OK (0)

// Transform display window
char timg_window_name[] = "Edge Detector Transform";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame, cdst, timg_gray, timg_grad;

IplImage* frame;

static struct timespec start_time;
static struct timespec stop_time;
static struct timespec difference;
static struct timespec max_diff;
static struct timespec min_diff;
static struct timespec jitter;
static struct timespec temp;
 

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

void jitter_difference_start(void)
{
	printf("\nCanny interactive transform\n");
	printf("Resolution 640x480 \n");
  	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("Transform start seconds = %ld, nanoseconds = %ld\n", 
         start_time.tv_sec, start_time.tv_nsec);
}

void jitter_difference_end(void)
{
	
  	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("Transform stop seconds = %ld, nanoseconds = %ld\n", 
         stop_time.tv_sec, stop_time.tv_nsec);

	delta_t(&stop_time, &start_time, &difference);
	printf("Transform time required seconds = %ld, nanoseconds = %ld\n", 
        difference.tv_sec, difference.tv_nsec);

	delta_t(&difference, &min_diff, &temp);
	if(temp.tv_sec < 0)
	{
		min_diff.tv_sec = difference.tv_sec;
		min_diff.tv_nsec = difference.tv_nsec;
	}
	
	delta_t(&max_diff, &difference, &temp);
	if(temp.tv_sec < 0)
	{
		max_diff.tv_sec = difference.tv_sec;
		max_diff.tv_nsec = difference.tv_nsec;
	}

	delta_t(&max_diff, &min_diff, &jitter);
	printf("Jitter seconds = %ld, nanoseconds = %ld\n", 
        jitter.tv_sec, jitter.tv_nsec);

	return;
}


int main( int argc, char** argv )
{
    CvCapture* capture;
    int dev=0,rc=0;
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

    namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );
    // Create a Trackbar for user to enter threshold
    createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    min_diff.tv_sec = 1600000000;
    while(1)
    {
	jitter_difference_start();
        frame=cvQueryFrame(capture);
        if(!frame) break;
	jitter_difference_end();

        CannyThreshold(0, 0);

        char q = cvWaitKey(33);
        if( q == 'q' )
        {
            printf("got quit\n"); 
            break;
        }
    }

    cvReleaseCapture(&capture);
    
};