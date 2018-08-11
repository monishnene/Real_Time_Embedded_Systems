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

Mat frame;

/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	VideoCapture camera(0);
	camera >> frame;	
	imwrite("0.ppm",frame);
	camera >> frame;	
	imwrite("1.ppm",frame);
	camera >> frame;	
	imwrite("2.ppm",frame);
	camera >> frame;	
	imwrite("3.ppm",frame);
	camera >> frame;	
	imwrite("4.ppm",frame);
	camera >> frame;	
	imwrite("5.ppm",frame);
	camera >> frame;	
	imwrite("6.ppm",frame);
	camera >> frame;	
	imwrite("7.ppm",frame);
	camera >> frame;	
	imwrite("8.ppm",frame);
	camera >> frame;	
	imwrite("9.ppm",frame);
	camera.release();
}
