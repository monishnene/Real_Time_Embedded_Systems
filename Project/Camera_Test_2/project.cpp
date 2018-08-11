/*
 * project.cpp
 *
 *  Created on: August 4, 2018
 *      Author: monish
 * Main, initializations and primary functions
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <stdbool.h>

#include <sys/param.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define MAX_PRIORITY  sched_get_priority_max(SCHED_FIFO)

#define OK (0)
#define NSEC_PER_SEC 1000000000
#define NSEC_PER_MSEC 1000000
#define TOTAL_THREADS 7
#define TOTAL_CAPTURES 10
#define THREADS_POST_TIME (1*NSEC_PER_MSEC)
#define SCHEDULER_FREQ 30
#define True 1
#define False 0
#define ON 0
#define OFF 1
#define VRES 480
#define HRES 640

//opencv Declarations
//IplImage* frame;
//CvCapture* camera = cvCaptureFromCAM(CV_CAP_ANY);
uint8_t* image_pointer;

/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	clock_gettime(CLOCK_REALTIME,&code_start_time);
	//cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, HRES);
	//cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, VRES);
	cvReleaseCapture(&camera);
	clock_gettime(CLOCK_REALTIME,&code_end_time); 
	delta_t(&code_end_time, &code_start_time, &code_execution_time);
	cout<<"\n\nCode Execution Time = "<<code_execution_time.tv_sec<<" seconds "<<code_execution_time.tv_nsec<<" nano seconds.\n";
}
