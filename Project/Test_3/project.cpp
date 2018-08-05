/*
 * project.cpp
 *
 *  Created on: August 4, 2018
 *      Author: monish
 * Main, initializations and primary functions
 */
#include "thread_operations.hpp"

void* func_1(void* ptr)
{
	uint8_t func_id=0;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rPikachu\n\r");
		loop_condition_check();
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_2(void* ptr)
{
	uint8_t func_id=1;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rLapras\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_3(void* ptr)
{
	uint8_t func_id=2;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rBulbasaur\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_4(void* ptr)
{
	uint8_t func_id=3;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rCharmander\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_5(void* ptr)
{
	uint8_t func_id=4;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rSquirtle\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_6(void* ptr)
{
	uint8_t func_id=5;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rPrimeape\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}

void* func_7(void* ptr)
{
	uint8_t func_id=6;
	while(loop_condition)
	{	
		function_beginning(func_id);
		printf("\n\rSnorlax\n\r");
		function_end(func_id);
	}
	pthread_exit(NULL);
}


/***********************************************************************
  * @brief main()
  * initialize variables and threads
  ***********************************************************************/
int main(int argc, char** argv)
{
	uint8_t i=0;
	clock_gettime(CLOCK_REALTIME,&code_start_time); 
	func_props[0].function_pointer = func_1; 
	func_props[1].function_pointer = func_2; 
	func_props[2].function_pointer = func_3; 
	func_props[3].function_pointer = func_4;
	func_props[4].function_pointer = func_5;
	func_props[5].function_pointer = func_6;
	func_props[6].function_pointer = func_7;
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_create(&func_props[i]);
	}
	sem_post(&(func_props[1].sem));
	for(i=0;i<TOTAL_THREADS;i++)
	{
		thread_join(&func_props[i]);
	}
}
