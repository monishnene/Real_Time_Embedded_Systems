/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define OK (0)
#define NSEC_PER_SEC (1000000000)
#define HRES 1280
#define VRES 960


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


void jitter_difference_start(void)
{
	printf("\nHough interactive transform\n");
	printf("Resolution 1280x960 \n");
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
    cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(0);
    //CvCapture* capture = (CvCapture *)cvCreateCameraCapture(argv[1]);
    CvCapture* capture;
    IplImage* frame;
    int dev=0;
    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

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

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);
    min_diff.tv_sec = 1600000000;

    while(1)
    {
	jitter_difference_start();
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
	jitter_difference_end();

        cvShowImage("Capture Example", frame);

        char c = cvWaitKey(10);
        if( c == 27 ) break;
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow("Capture Example");
    
};